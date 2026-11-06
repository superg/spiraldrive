#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_position_sample_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory[0x20000ul];
static u8 parse_result;
static u8 evaluate_result;
static unsigned arm_count;
static unsigned parse_count;
static unsigned yield_count;
static unsigned evaluate_count;
static s16 evaluate_reference;
static u8 zero_status_after_yield;
static u8 set_status_on_first_arm;
static u8 first_arm_status;

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

void ObserveControllerPositionSampleArm(void)
{
    ++arm_count;
    if(arm_count == 1u && set_status_on_first_arm != 0u)
        memory[0x1203u] = first_arm_status;
}

u8 ObserveControllerPositionSampleParse(void)
{
    ++parse_count;
    return parse_result;
}

void ObserveControllerPositionSampleYield(void)
{
    ++yield_count;
    if(zero_status_after_yield != 0u)
        memory[0x1203u] = 0u;
}

u8 ObserveControllerPositionSampleEvaluate(s16 interior_reference)
{
    ++evaluate_count;
    evaluate_reference = interior_reference;
    return evaluate_result;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
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

static s32 PrepareFixture(void)
{
    s32 target = 8804l;
    u8 index;

    memset(memory, 0, sizeof(memory));
    parse_result = 1u;
    evaluate_result = 7u;
    arm_count = parse_count = yield_count = evaluate_count = 0u;
    evaluate_reference = 0;
    zero_status_after_yield = 0u;
    set_status_on_first_arm = 0u;
    first_arm_status = 0u;
    memory[0x1301u] = 0xa0u;
    memory[0xe47u] = 4u;
    WriteU16(0xfb63u, 0x1234u);
    WriteU16(0xfb5fu, 0xa55au);
    WriteU32(0xfe9u, 10000ul);
    memory[0x0feeu] = 2u;
    memory[0x0fefu] = 4u;
    WriteU16(0xff5u, 0x4000u);
    for(index = 0u; index != 10u; ++index)
        memory[0xfdbu + index] = 0xa5u;
    return target;
}

static void AssertInitialization(void)
{
    u8 index;

    assert(ReadU32(0xfe5u) == 0x10000000ul);
    assert((memory[0x1301u] & 1u) != 0u);
    assert(memory[0xff0u] == 0u);
    for(index = 0u; index != 10u; ++index)
        assert(memory[0xfdbu + index] == 0u);
    assert(ReadU16(0xfb5fu) == 0x1234u);
    assert((ReadU16(0xff5u) & 0x10u) != 0u);
    assert(arm_count >= 1u);
}

static void TestZeroAndNegativeCaptureStatus(void)
{
    PrepareFixture();
    zero_status_after_yield = 1u;
    parse_result = 0xfeu;
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 0u);
    AssertInitialization();
    assert(parse_count == 1u && yield_count == 1u);
    assert((memory[0x1301u] & 1u) != 0u);

    PrepareFixture();
    set_status_on_first_arm = 1u;
    first_arm_status = 0xffu;
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 0u);
    assert((memory[0x1301u] & 1u) == 0u && parse_count == 0u);
}

static void TestExactTargetSetsWindowAndContinues(void)
{
    s32 target = PrepareFixture();

    WriteU32(0x1235u, (u32)target);
    zero_status_after_yield = 1u;
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 0u);
    assert(memory[0xfedu] == 0u);
    assert((ReadU16(0xff5u) & 8u) != 0u);
    assert(arm_count == 2u && yield_count == 1u);
}

static void TestTargetPlusOneRequiresEstablishedWindow(void)
{
    s32 target = PrepareFixture();

    WriteU32(0x1235u, (u32)target + 1ul);
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 0u);
    assert((memory[0x1301u] & 1u) == 0u);
    assert(yield_count == 0u);
}

static void TestTargetPlusFourResultClasses(void)
{
    s32 target = PrepareFixture();

    WriteU16(0xff5u, 0x4008u);
    WriteU32(0x1235u, (u32)target + 4ul);
    memory[0xf9a4u] = 0x10u;
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 11u);
    assert((ReadU16(0xff5u) & 8u) == 0u);
    assert((memory[0x1301u] & 1u) == 0u);

    target = PrepareFixture();
    WriteU16(0xff5u, 0x4008u);
    WriteU32(0x1235u, (u32)target + 4ul);
    memory[0xf9a4u] = 0x10u;
    memory[0xff1u] = 1u;
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 3u);
}

static void TestPastWindowDelegatesToEvaluator(void)
{
    s32 target = PrepareFixture();

    WriteU16(0xff5u, 0x4008u);
    WriteU32(0x1235u, (u32)target + 5ul);
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 7u);
    assert(evaluate_count == 1u);
    assert(evaluate_reference == 0);
    assert((ReadU16(0xff5u) & 8u) == 0u);
    assert((memory[0x1301u] & 1u) == 0u);

    PrepareFixture();
    target = 0x12345l;
    WriteU32(0xfe9u, (u32)target + 1196ul);
    WriteU16(0xff5u, 0x4008u);
    WriteU32(0x1235u, (u32)target + 5ul);
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 7u);
    assert(evaluate_reference == 1);
}

static void TestSpeedTargetSelection(void)
{
    static const u8 speeds[] = { 1u, 8u, 16u, 20u, 24u, 32u, 40u, 52u };
    static const u16 values[] = { 0x1111u, 0x2222u, 0x3333u, 0x3333u, 0x3333u, 0x3333u, 0x3333u, 0x3333u };
    u8 index;

    for(index = 0u; index != sizeof(speeds); ++index)
    {
        PrepareFixture();
        memory[0xe47u] = speeds[index];
        WriteU16(0xfb61u, 0x1111u);
        WriteU16(0xfb65u, 0x2222u);
        WriteU16(0xfb67u, 0x3333u);
        set_status_on_first_arm = 1u;
        first_arm_status = 0u;
        assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 0u);
        assert(ReadU16(0xfb5fu) == values[index]);
    }

    PrepareFixture();
    memory[0xe47u] = 7u;
    set_status_on_first_arm = 1u;
    first_arm_status = 0u;
    assert(AcquireControllerPositionSamplesAndUpdateCalibration() == 0u);
    assert(ReadU16(0xfb5fu) == 0xa55au);
}

int main(void)
{
    TestZeroAndNegativeCaptureStatus();
    TestExactTargetSetsWindowAndContinues();
    TestTargetPlusOneRequiresEstablishedWindow();
    TestTargetPlusFourResultClasses();
    TestPastWindowDelegatesToEvaluator();
    TestSpeedTargetSelection();
    puts("controller position sample acquisition tests passed");
    return 0;
}
