#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_sector_sequence.h"
#include "platform/firmware_memory.h"

static u8 memory[0x10000ul];
static u8 selected_track;
static fw_addr_t resumed_cursor;
static u8 capacity_results[8];
static u8 capture_results[8];
static unsigned capacity_count;
static unsigned capture_count;
static fw_addr_t capture_addresses[8];
static u8 capture_preserve[8];
static u8 capture_clear[8];
static unsigned select_count;
static unsigned resume_count;
static unsigned convert_count;
static s32 converted_position;
static unsigned pack_count;
static unsigned copy_count;
static u16 copied_destination;
static unsigned enqueue_count;
static unsigned advance_count;
static unsigned increment_count;
static u16 increment_addresses[8];
static unsigned failure_count;
static u8 failure_code;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

u8 ObserveRawSectorSequenceSelectTrack(void)
{
    ++select_count;
    return selected_track;
}

fw_addr_t ObserveRawSectorSequenceResumeRing(void)
{
    ++resume_count;
    return resumed_cursor;
}

void ObserveRawSectorSequenceConvertPosition(s32 frame_position, fw_addr_t tuple_address)
{
    assert(tuple_address == 0x7000ul);
    ++convert_count;
    converted_position = frame_position;
    memory[tuple_address] = 1u;
    memory[tuple_address + 1u] = 2u;
    memory[tuple_address + 2u] = 3u;
}

void ObserveRawSectorSequencePackTuple(u16 tuple_address)
{
    assert(tuple_address == 0x7000u);
    ++pack_count;
    memory[tuple_address] = 0x11u;
    memory[tuple_address + 1u] = 0x22u;
    memory[tuple_address + 2u] = 0x33u;
}

void ObserveRawSectorSequenceCopyTuple(u16 destination, u16 source)
{
    unsigned index;

    assert(source == 0x7000u);
    ++copy_count;
    copied_destination = destination;
    for(index = 0u; index != 3u; ++index)
        memory[destination + index] = memory[source + index];
}

u8 ObserveRawSectorSequenceWaitCapacity(void)
{
    u8 result = capacity_results[capacity_count];

    ++capacity_count;
    return result;
}

u8 ObserveRawSectorSequenceCapture(fw_addr_t main_buffer_address, u8 preserve_main_buffer, u8 clear_subchannel_slot)
{
    u8 result = capture_results[capture_count];

    assert(capture_count < 8u);
    capture_addresses[capture_count] = main_buffer_address;
    capture_preserve[capture_count] = preserve_main_buffer;
    capture_clear[capture_count] = clear_subchannel_slot;
    ++capture_count;
    return result;
}

void ObserveRawSectorSequenceEnqueue(void)
{
    ++enqueue_count;
}

fw_addr_t ObserveRawSectorSequenceAdvanceRing(fw_addr_t cursor)
{
    ++advance_count;
    return cursor + 0x930ul;
}

void ObserveRawSectorSequenceIncrementMsf(u16 tuple_address)
{
    assert(increment_count < 8u);
    increment_addresses[increment_count++] = tuple_address;
}

void ObserveRawSectorSequenceReportFailure(u8 code)
{
    ++failure_count;
    failure_code = code;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static fw_addr_t SummaryAddress(u8 track)
{
    return 0xf2b7ul + (fw_addr_t)((u16)track * 6u);
}

static void ResetFixture(void)
{
    unsigned index;

    memset(memory, 0, sizeof(memory));
    selected_track = 7u;
    resumed_cursor = 0x2000ul;
    memset(capacity_results, 1, sizeof(capacity_results));
    memset(capture_results, 1, sizeof(capture_results));
    capacity_count = 0u;
    capture_count = 0u;
    select_count = 0u;
    resume_count = 0u;
    convert_count = 0u;
    converted_position = 0;
    pack_count = 0u;
    copy_count = 0u;
    copied_destination = 0u;
    enqueue_count = 0u;
    advance_count = 0u;
    increment_count = 0u;
    failure_count = 0u;
    failure_code = 0u;
    for(index = 0u; index != 10u; ++index)
        memory[0x1272u + index] = 0xa5u;
}

static void TestLeadInPolicyAndModeZeroLoop(void)
{
    ResetFixture();
    memory[0xe600u] = 0u;
    memory[0x12bfu] = 4u;
    memory[SummaryAddress(4u)] = 0x40u;
    WriteU32(0x1598u, 100u);

    assert(AcquireRawSectorSequence(1ul, 0u, 0u, 1u) == 1u);
    assert(memory[0x1285u] == 8u);
    assert(resume_count == 1u && select_count == 0u);
    assert(capacity_count == 2u && capture_count == 2u);
    assert(capture_addresses[0] == 0x2000ul);
    assert(capture_addresses[1] == 0x2930ul);
    assert(capture_preserve[0] == 1u && capture_preserve[1] == 1u);
    assert(capture_clear[0] == 1u && capture_clear[1] == 1u);
    assert(enqueue_count == 2u && advance_count == 2u);
    assert(ReadU32(0x126eu) == 0x2930ul);
    assert(ReadU32(0x1598u) == 102u);
    assert(memory[0x1278u] == 2u);
    assert(increment_count == 0u && failure_count == 0u);
    assert(memory[0x1272u] == 0u && memory[0x127bu] == 0u);
}

static void TestWorkingTrackAndCallerPolicies(void)
{
    ResetFixture();
    memory[0xe600u] = 3u;
    memory[0xdd36u] = 0u;
    assert(AcquireRawSectorSequence(0ul, 3u, 1u, 1u) == 1u);
    assert(memory[0x1285u] == 2u);
    assert(capture_preserve[0] == 0u && capture_clear[0] == 1u);

    ResetFixture();
    memory[0xe600u] = 9u;
    assert(AcquireRawSectorSequence(0ul, 3u, 2u, 7u) == 1u);
    assert(memory[0x1285u] == 2u);
    assert(capture_preserve[0] == 2u && capture_clear[0] == 7u);
}

static void TestOperationFiveTrackSelection(void)
{
    ResetFixture();
    memory[0xe600u] = 5u;
    memory[0x14ccu] = 0xbbu;
    memory[SummaryAddress(selected_track)] = 0x40u;

    assert(AcquireRawSectorSequence(0ul, 3u, 0u, 1u) == 1u);
    assert(select_count == 1u);
    assert(capture_preserve[0] == 1u);
    assert(capture_clear[0] == 0u);

    ResetFixture();
    memory[0xe600u] = 6u;
    memory[0x14ccu] = 3u;
    assert(AcquireRawSectorSequence(0ul, 3u, 1u, 1u) == 1u);
    assert(select_count == 0u);
    assert(capture_preserve[0] == 0u && capture_clear[0] == 0u);
}

static void TestModeOneTupleAndAdvancement(void)
{
    ResetFixture();
    memory[0xe600u] = 9u;
    WriteU32(0x1598u, 0x12345678ul);

    assert(AcquireRawSectorSequence(1ul, 1u, 1u, 0u) == 1u);
    assert(convert_count == 1u && converted_position == 0x12345678l);
    assert(pack_count == 1u && copy_count == 1u);
    assert(copied_destination == 0x1275u);
    assert(memory[0x1275u] == 0x11u);
    assert(memory[0x1276u] == 0x22u);
    assert(memory[0x1277u] == 0x33u);
    assert(increment_count == 2u);
    assert(increment_addresses[0] == 0x1275u);
    assert(increment_addresses[1] == 0x1275u);
}

static void TestModeTwoThresholdCrossing(void)
{
    ResetFixture();
    memory[0xe600u] = 9u;
    WriteU32(0x1598u, 149u);

    assert(AcquireRawSectorSequence(1ul, 2u, 1u, 0u) == 1u);
    assert(memory[0x1272u] == 0x60u);
    assert(memory[0x1273u] == 0xffu && memory[0x1274u] == 0xffu);
    assert(copied_destination == 0x1279u);
    assert(increment_count == 2u);
    assert(increment_addresses[0] == 0x1279u);
    assert(increment_addresses[1] == 0x1279u);
    assert(ReadU32(0x1598u) == 151u);
    assert(memory[0x127cu] == 0u);

    ResetFixture();
    memory[0xe600u] = 9u;
    WriteU32(0x1598u, (u32)-1l);
    assert(AcquireRawSectorSequence(0ul, 2u, 0u, 0u) == 1u);
    assert(memory[0x1272u] == 0x20u);
    assert(memory[0x127cu] == 1u);
}

static void TestFailureAndWrappedCountRoutes(void)
{
    ResetFixture();
    memory[0xe600u] = 9u;
    capacity_results[0] = 0u;
    assert(AcquireRawSectorSequence(2ul, 0u, 1u, 0u) == 0u);
    assert(capacity_count == 1u && capture_count == 0u);
    assert(failure_count == 1u && failure_code == 7u);

    ResetFixture();
    memory[0xe600u] = 9u;
    capture_results[0] = 2u;
    capture_results[1] = 0u;
    capacity_results[0] = 2u;
    capacity_results[1] = 2u;
    assert(AcquireRawSectorSequence(2ul, 0u, 1u, 0u) == 0u);
    assert(capacity_count == 2u && capture_count == 2u);
    assert(enqueue_count == 1u && advance_count == 1u);
    assert(failure_count == 1u && failure_code == 7u);

    ResetFixture();
    memory[0xe600u] = 9u;
    assert(AcquireRawSectorSequence(0xfffffffful, 0u, 1u, 0u) == 1u);
    assert(capacity_count == 0u && capture_count == 0u);
    assert(enqueue_count == 0u && failure_count == 0u);
}

int main(void)
{
    TestLeadInPolicyAndModeZeroLoop();
    TestWorkingTrackAndCallerPolicies();
    TestOperationFiveTrackSelection();
    TestModeOneTupleAndAdvancement();
    TestModeTwoThresholdCrossing();
    TestFailureAndWrappedCountRoutes();
    puts("raw sector sequence tests passed");
    return 0;
}
