#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_position_sample_evaluation.h"
#include "platform/firmware_memory.h"

static u8 memory_bytes[0x20000ul];
static u8 low_speed_table[512];
static u8 high_speed_table[512];
static u8 firmware_mode;
static s16 forced_nearest_index;
static u8 spacing_result;
static s16 observed_spacing_reference;
static u8 indexed_state_result;
static unsigned update_flags_count;

static u8 *ResolveByte(fw_addr_t address)
{
    if(address < sizeof(memory_bytes))
        return &memory_bytes[address];
    if(address >= 0xff92ebul && address < 0xff92ebul + sizeof(low_speed_table))
        return &low_speed_table[address - 0xff92ebul];
    if(address >= 0xff9337ul && address < 0xff9337ul + sizeof(high_speed_table))
        return &high_speed_table[address - 0xff9337ul];
    assert(address == 0xf7c01ful);
    return &firmware_mode;
}

u8 FirmwareRead8(fw_addr_t address)
{
    return *ResolveByte(address);
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    *ResolveByte(address) = value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)FirmwareRead8(address) << 8) | FirmwareRead8(address + 1u));
}

static void WriteWord(fw_addr_t address, u16 value)
{
    FirmwareWrite8(address, (u8)(value >> 8));
    FirmwareWrite8(address + 1u, (u8)value);
}

void ObservePositionSampleSelectNearest(void)
{
    s16 target;
    s16 best;
    u8 best_index;
    u8 index;

    if(forced_nearest_index >= 0)
    {
        memory_bytes[0xff0u] = (u8)forced_nearest_index;
        return;
    }
    target = (s16)ReadWord(0xfb5fu);
    best = (s16)ReadWord(0xfdbu);
    best_index = 0u;
    for(index = 1u; index < 5u; ++index)
    {
        s16 candidate = (s16)ReadWord((fw_addr_t)(0xfdbu + index * 2u));
        u16 best_distance = (u16)(best < target ? (s32)target - (s32)best : (s32)best - (s32)target);
        u16 candidate_distance = (u16)(candidate < target ? (s32)target - (s32)candidate : (s32)candidate - (s32)target);

        if(candidate_distance < best_distance || (candidate_distance == best_distance && candidate <= target))
        {
            best = candidate;
            best_index = index;
        }
    }
    memory_bytes[0xff0u] = best_index;
}

u8 ObservePositionSampleInteriorSpacing(s16 reference_value)
{
    observed_spacing_reference = reference_value;
    return spacing_result;
}

void ObservePositionSampleUpdateHighSpeedFlags(void)
{
    ++update_flags_count;
}

u8 ObservePositionSampleIndexedState(u8 state_code)
{
    assert(state_code == 4u);
    return indexed_state_result;
}

static void ResetFixture(void)
{
    u8 index;

    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(low_speed_table, 0, sizeof(low_speed_table));
    memset(high_speed_table, 0, sizeof(high_speed_table));
    firmware_mode = 0u;
    forced_nearest_index = -1;
    spacing_result = 1u;
    observed_spacing_reference = 0;
    indexed_state_result = 0u;
    update_flags_count = 0u;
    memory_bytes[0xe47u] = 4u;
    WriteWord(0xff5u, 0x0050u);
    for(index = 0u; index < 5u; ++index)
        WriteWord((fw_addr_t)(0xfdbu + index * 2u), (u16)(100u + (u16)index * 100u));
    WriteWord(0xfb5fu, 250u);
    WriteWord(0xff92ebul + 2u, 1000u);
    WriteWord(0xff92ebul + 4u, 2000u);
}

static void TestEndpointRejections(void)
{
    ResetFixture();
    WriteWord(0xfaacu, 2u);
    forced_nearest_index = 1;
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 2u);

    ResetFixture();
    WriteWord(0xfb5fu, 50u);
    forced_nearest_index = 0;
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 2u);

    ResetFixture();
    WriteWord(0xfb5fu, 600u);
    forced_nearest_index = 4;
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 3u);

    ResetFixture();
    memory_bytes[0xe47u] = 16u;
    forced_nearest_index = 1;
    WriteWord(0xfb5fu, 150u);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 2u);
}

static void TestInvalidSpacingAndOrderingClearWindow(void)
{
    ResetFixture();
    spacing_result = 0u;
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0x1234) == 0u);
    assert(observed_spacing_reference == 0x1234);
    assert((ReadWord(0xff5u) & 0x10u) == 0u);

    ResetFixture();
    forced_nearest_index = 1;
    WriteWord(0xfdfu, 200u);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 0u);
    assert((ReadWord(0xff5u) & 0x10u) == 0u);

    ResetFixture();
    forced_nearest_index = 1;
    WriteWord(0xfddu, 10u);
    WriteWord(0xfdfu, 0xffffu);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 3u);
}

static void TestFirstPassInterpolationAndSnapshot(void)
{
    ResetFixture();
    WriteWord(0xfd3u, 100u);
    WriteWord(0x100cu, 0x4567u);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0x2345) == 0u);
    assert(ReadWord(0xfd3u) == 1600u);
    assert(ReadWord(0xfd7u) == 0x4567u);
    assert(memory_bytes[0xfae1u] == 1u);
    assert(observed_spacing_reference == 0x2345);
    assert(update_flags_count == 0u);
}

static void TestSecondPassPublicationAndStatuses(void)
{
    ResetFixture();
    memory_bytes[0xfae1u] = 1u;
    WriteWord(0xfd3u, 100u);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 1u);
    assert(ReadWord(0xfd3u) == 1600u);
    assert(ReadWord(0xfacfu) == 800u);
    assert(update_flags_count == 1u);

    ResetFixture();
    memory_bytes[0xfae1u] = 1u;
    indexed_state_result = 1u;
    firmware_mode = 0x11u;
    memory_bytes[0xfad4u] = 0x34u;
    WriteWord(0xfad7u, 0x0020u);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 5u);
}

static void TestSpeedFourSpecialAccumulation(void)
{
    ResetFixture();
    memory_bytes[0xfae1u] = 1u;
    memory_bytes[0x158eu] = 2u;
    WriteWord(0xfd3u, 100u);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 0u);
    assert((memory_bytes[0x158eu] & 8u) != 0u);
    assert(ReadWord(0xfacfu) == 800u);
    assert(ReadWord(0xfd3u) == 0u);
    assert((ReadWord(0xff5u) & 0x10u) == 0u);
    assert(update_flags_count == 0u);

    ResetFixture();
    memory_bytes[0xfae1u] = 1u;
    memory_bytes[0x158eu] = 10u;
    WriteWord(0xfd3u, 100u);
    WriteWord(0xfacfu, 200u);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 1u);
    assert(ReadWord(0xfacfu) == 500u);
}

static void TestHighSpeedTableAndIndexOffset(void)
{
    ResetFixture();
    memory_bytes[0xe47u] = 16u;
    memory_bytes[0xf962u] = 3u;
    memory_bytes[0xfae1u] = 1u;
    forced_nearest_index = 2;
    WriteWord(0xfb5fu, 350u);
    WriteWord(0xff9337ul + 10u, 2000u);
    WriteWord(0xff9337ul + 12u, 3000u);
    assert(EvaluateControllerPositionSamplesAndUpdateCalibration(0) == 1u);
    assert(ReadWord(0xfd3u) == 2500u);
    assert(ReadWord(0xfacfu) == 1250u);
}

int main(void)
{
    TestEndpointRejections();
    TestInvalidSpacingAndOrderingClearWindow();
    TestFirstPassInterpolationAndSnapshot();
    TestSecondPassPublicationAndStatuses();
    TestSpeedFourSpecialAccumulation();
    TestHighSpeedTableAndIndexOffset();
    puts("controller position sample evaluation tests passed");
    return 0;
}
