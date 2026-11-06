#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/format_plan_q.h"

static u8 memory[1u << 24];
static u8 last_track;
static u8 last_track_calls;
static u8 bcd_calls;
static u8 conversion_calls;
static s32 conversion_frames[2];
static fw_addr_t conversion_destinations[2];
static u8 pma_count;
static u8 pma_calls;
static fw_addr_t pma_destination;
static u8 unpack_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveFormatPlanPackedBcd(u8 value)
{
    bcd_calls++;
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

void ObserveFormatPlanFrameToMsf(s32 frame_position, fw_addr_t output_address)
{
    u32 position = (u32)frame_position;
    u32 total_seconds;
    assert(conversion_calls < 2u);
    conversion_frames[conversion_calls] = frame_position;
    conversion_destinations[conversion_calls] = output_address;
    conversion_calls++;
    if(frame_position < 0)
        position = (u32)(frame_position + 450000l);
    total_seconds = position / 75u;
    memory[output_address] = (u8)(total_seconds / 60u);
    memory[output_address + 1u] = (u8)(total_seconds % 60u);
    memory[output_address + 2u] = (u8)(position % 75u);
}

u8 ObserveFormatPlanLastTrack(void)
{
    last_track_calls++;
    return last_track;
}

u8 ObserveFormatPlanBuildPma(fw_addr_t destination)
{
    u8 index;
    pma_calls++;
    pma_destination = destination;
    for(index = 0u; index < pma_count; ++index)
        memory[destination + (fw_addr_t)index * 10ul] = (u8)(0xb1u + index);
    return pma_count;
}

u8 ObserveFormatPlanPackedBcdToBinary(u8 value)
{
    unpack_calls++;
    return (u8)(((value >> 4) * 10u) + (value & 0x0fu));
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u8 Packed(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(&memory[0x2bc11bu], 0xa5, 40u);
    last_track = 9u;
    last_track_calls = 0u;
    bcd_calls = 0u;
    conversion_calls = 0u;
    pma_count = 0u;
    pma_calls = 0u;
    pma_destination = 0u;
    unpack_calls = 0u;
    memset(conversion_frames, 0, sizeof(conversion_frames));
    memset(conversion_destinations, 0, sizeof(conversion_destinations));
    memory[0x000c47u] = 5u;
    memory[0x000c4au] = 0x12u;
    memory[0x0012c0u] = 6u;
    WriteU32(0x00e61au, 12345u);
    WriteU32(0x00e616u, 20000u);
    WriteU32(0x00dd3du, 30000u);
}

static void AssertRecordCommon(fw_addr_t record, u8 point)
{
    assert(memory[record] == 0x51u);
    assert(memory[record + 1u] == 0u);
    assert(memory[record + 2u] == point);
    assert(memory[record + 3u] == 0u);
    assert(memory[record + 4u] == 0u);
    assert(memory[record + 5u] == 0u);
    assert(memory[record + 6u] == 0u);
}

static void AssertPackedMsf(fw_addr_t tuple, u32 frame_position)
{
    u32 seconds = frame_position / 75u;
    assert(memory[tuple] == Packed((u8)(seconds / 60u)));
    assert(memory[tuple + 1u] == Packed((u8)(seconds % 60u)));
    assert(memory[tuple + 2u] == Packed((u8)(frame_position % 75u)));
}

static void TestNewDiscRecords(void)
{
    fw_addr_t base = 0x2bc11bul;
    ResetState();

    BuildFormatPlanPrimaryStagedRawQRecordSequence();

    AssertRecordCommon(base, 0xa0u);
    assert(memory[base + 7u] == 1u);
    assert(memory[base + 8u] == 0x12u);
    assert(memory[base + 9u] == 0u);
    AssertRecordCommon(base + 10u, 0xa1u);
    assert(memory[base + 17u] == 1u);
    assert(memory[base + 18u] == 0u && memory[base + 19u] == 0u);
    AssertRecordCommon(base + 20u, 0xa2u);
    AssertPackedMsf(base + 27u, 12345u);
    AssertRecordCommon(base + 30u, 1u);
    AssertPackedMsf(base + 37u, 20151u);
    assert(memory[0x2bc11au] == 4u);
    assert(last_track_calls == 1u);
    assert(bcd_calls == 9u);
    assert(conversion_calls == 2u);
    assert(conversion_frames[0] == 12345l && conversion_destinations[0] == base + 27u);
    assert(conversion_frames[1] == 20151l && conversion_destinations[1] == base + 37u);
}

static void TestExistingSessionTrackSelection(void)
{
    fw_addr_t base = 0x2bc11bul;
    ResetState();
    memory[0x00e5fau] = 1u;

    BuildFormatPlanPrimaryStagedRawQRecordSequence();

    assert(memory[base + 7u] == 7u);
    assert(memory[base + 17u] == 7u);
    assert(memory[base + 32u] == 7u);
    assert(conversion_frames[1] == 20151l);
}

static void TestAppendTrackAndPosition(void)
{
    fw_addr_t base = 0x2bc11bul;
    ResetState();
    memory[0x00e5fbu] = 1u;
    last_track = 12u;

    BuildFormatPlanPrimaryStagedRawQRecordSequence();

    assert(memory[base + 7u] == 0x12u);
    assert(memory[base + 17u] == 0x12u);
    assert(memory[base + 32u] == 0x12u);
    assert(conversion_frames[1] == 30000l);
    AssertPackedMsf(base + 37u, 30000u);
}

static void PrepareSecondaryFields(void)
{
    memory[0x00113bu] = 79u;
    memory[0x00113cu] = 59u;
    memory[0x00113du] = 74u;
    memory[0x00113fu] = 97u;
    memory[0x001140u] = 58u;
    memory[0x001141u] = 73u;
    memory[0x00fb7au] = 1u;
    memory[0x00fb7bu] = 2u;
    memory[0x00fb7cu] = 3u;
    memory[0x00fb7du] = 4u;
    memory[0x00fb7eu] = 8u;
    memory[0x00fb7fu] = 1u;
    memory[0x00fb80u] = 4u;
    memory[0x00fb81u] = 2u;
    memory[0x00fb82u] = 8u;
    memory[0x00fb83u] = 7u;
    WriteU32(0x00e61eu, 10000u);
    WriteU32(0x00dd69u, 0xfffffffful);
}

static void TestSecondaryC0WithC0AndC1(void)
{
    fw_addr_t base = 0x2bc519ul;
    ResetState();
    PrepareSecondaryFields();
    memset(&memory[base], 0xa5, 30u);
    memory[0x000c45u] = 0xc0u;
    memory[0x001008u] = 2u;

    BuildFormatPlanSecondaryStagedRawQRecordSequence();

    assert(memory[base] == 0x55u && memory[base + 2u] == 0xb0u);
    AssertPackedMsf(base + 3u, 14500u);
    assert(memory[base + 6u] == 3u);
    assert(memory[base + 7u] == 0x79u);
    assert(memory[base + 8u] == 0x59u);
    assert(memory[base + 9u] == 0x74u);
    assert(memory[base + 10u] == 0x55u && memory[base + 12u] == 0xc0u);
    assert(memory[base + 13u] == 6u);
    assert(memory[base + 14u] == 6u);
    assert(memory[base + 15u] == 24u);
    assert(memory[base + 17u] == 0x97u);
    assert(memory[base + 18u] == 0x58u);
    assert(memory[base + 19u] == 0x73u);
    assert(memory[base + 20u] == 0x55u && memory[base + 22u] == 0xc1u);
    assert(memory[base + 23u] == 10u);
    assert(memory[base + 24u] == 20u);
    assert(memory[base + 25u] == 14u);
    assert(memory[base + 21u] == 0u && memory[base + 26u] == 0u);
    assert(memory[base + 27u] == 0u && memory[base + 28u] == 0u && memory[base + 29u] == 0u);
    assert(memory[0x2bc517u] == 0u && memory[0x2bc518u] == 3u);
    assert(pma_calls == 1u && pma_destination == base + 10u);
    assert(unpack_calls == 1u);
}

static void TestSecondaryOperation40WithPmaSuppressesC(void)
{
    fw_addr_t base = 0x2bc519ul;
    ResetState();
    PrepareSecondaryFields();
    memset(&memory[base], 0xa5, 40u);
    memory[0x000c45u] = 0x40u;
    memory[0x000c47u] = 0u;
    pma_count = 2u;

    BuildFormatPlanSecondaryStagedRawQRecordSequence();

    assert(memory[base] == 5u && memory[base + 2u] == 0xb0u);
    assert(memory[base + 3u] == 0xffu && memory[base + 4u] == 0xffu && memory[base + 5u] == 0xffu);
    assert(memory[base + 6u] == 3u);
    assert(memory[base + 10u] == 0xb1u);
    assert(memory[base + 20u] == 0xb2u);
    assert(memory[base + 30u] == 0xa5u);
    assert(memory[0x2bc518u] == 3u);
}

static void TestSecondaryOperationZeroWithoutB0(void)
{
    fw_addr_t base = 0x2bc519ul;
    ResetState();
    memset(&memory[base], 0xa5, 10u);
    memory[0x000c45u] = 0u;

    BuildFormatPlanSecondaryStagedRawQRecordSequence();

    assert(memory[base] == 0x55u && memory[base + 2u] == 0xb0u);
    assert(memory[base + 6u] == 0xa5u);
    assert(memory[0x2bc517u] == 0u && memory[0x2bc518u] == 0u);
    assert(pma_calls == 1u && pma_destination == base);
    assert(unpack_calls == 0u);
}

int main(void)
{
    TestNewDiscRecords();
    TestExistingSessionTrackSelection();
    TestAppendTrackAndPosition();
    TestSecondaryC0WithC0AndC1();
    TestSecondaryOperation40WithPmaSuppressesC();
    TestSecondaryOperationZeroWithoutB0();
    return 0;
}
