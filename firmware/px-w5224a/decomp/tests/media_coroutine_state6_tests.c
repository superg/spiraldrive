#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/media_coroutine_state6.h"

static u8 memory_bytes[0x20000ul];
static u8 far_f7c01f;
static u8 completion_bit;
static u8 p3_bit0;
static u8 p3_bit1;
static u8 reduction_result;
static u8 set_completion_on_reduction;
static u8 calibration_result;
static u8 acquisition_result;
static unsigned reduction_count;
static u8 reduction_argument;
static unsigned measurement_retry_count;
static unsigned calibration_count;
static unsigned clear_flags_count;
static unsigned acquisition_count;
static unsigned apply_offset_count;
static unsigned yield_count;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x00f7c01ful)
        return far_f7c01f;
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_bytes[address] = (u8)(value >> 8);
    memory_bytes[address + 1u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

u8 ObserveMediaCoroutineState6ReadCompletionBit(void)
{
    return completion_bit;
}

void ObserveMediaCoroutineState6WriteCompletionBit(u8 value)
{
    completion_bit = (u8)(value != 0u);
}

void ObserveMediaCoroutineState6WriteP3Bit0(u8 value)
{
    p3_bit0 = (u8)(value != 0u);
}

void ObserveMediaCoroutineState6WriteP3Bit1(u8 value)
{
    p3_bit1 = (u8)(value != 0u);
}

u8 ObserveMediaCoroutineState6ReduceSpeed(u8 enabled)
{
    ++reduction_count;
    reduction_argument = enabled;
    if(set_completion_on_reduction != 0u)
        completion_bit = 1u;
    return reduction_result;
}

void ObserveMediaCoroutineState6MeasurementRetry(void)
{
    ++measurement_retry_count;
}

u8 ObserveMediaCoroutineState6Calibration(void)
{
    ++calibration_count;
    return calibration_result;
}

void ObserveMediaCoroutineState6ClearFlags(void)
{
    ++clear_flags_count;
}

u8 ObserveMediaCoroutineState6AcquirePosition(void)
{
    ++acquisition_count;
    return acquisition_result;
}

void ObserveMediaCoroutineState6ApplyOffset(void)
{
    ++apply_offset_count;
}

void ObserveMediaCoroutineState6Yield(void)
{
    ++yield_count;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    far_f7c01f = completion_bit = p3_bit0 = p3_bit1 = 0u;
    reduction_result = set_completion_on_reduction = 0u;
    calibration_result = acquisition_result = 1u;
    reduction_count = measurement_retry_count = calibration_count = 0u;
    clear_flags_count = acquisition_count = apply_offset_count = 0u;
    yield_count = 0u;
    memory_bytes[0xf971u] = 0xffu;
    memory_bytes[0xe43u] = 0x5au;
    memory_bytes[0xe47u] = 16u;
    memory_bytes[0xc8au] = 8u;
    memory_bytes[0x1302u] = 0xffu;
    memory_bytes[0x158eu] = 0xffu;
    memory_bytes[0xe3fu] = 1u;
    WriteDword(0x114fu, 0x00123456ul);
    WriteDword(0xfb8cu, 10000ul);
    WriteDword(0xfb90u, 20000ul);
    WriteWord(0xfacfu, 0x2468u);
}

static void TestSuccessfulLeadInPath(void)
{
    ResetFixture();
    memory_bytes[0x1304u] = 0x20u;
    WriteWord(0xfaacu, 2u);
    ProcessMediaCoroutineState6();
    assert(completion_bit == 0u);
    assert(memory_bytes[0xf971u] == 0xfdu);
    assert(memory_bytes[0xfaabu] == 0x12u);
    assert(memory_bytes[0x15b2u] == 2u);
    assert(memory_bytes[0xe43u] == 0x5au);
    assert(ReadDword(0xfe9u) == 0x00123456ul);
    assert((memory_bytes[0x1302u] & 0x10u) == 0u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);
    assert(clear_flags_count == 1u && acquisition_count == 1u);
    assert(ReadWord(0xfad1u) == 0x2468u);
    assert((ReadWord(0xff5u) & 0x40u) != 0u);
    assert(apply_offset_count == 1u && yield_count == 1u);
    assert(memory_bytes[0xe3fu] == 0u);
}

static void TestTargetAndDirectionVariants(void)
{
    ResetFixture();
    memory_bytes[0xc8au] = 0u;
    WriteWord(0xff5u, 0x1000u);
    ProcessMediaCoroutineState6();
    assert(ReadDword(0xfe9u) == 10000ul + 20000ul + 0x0a82ul);
    assert((memory_bytes[0x1302u] & 0x10u) == 0u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);

    ResetFixture();
    WriteWord(0xff5u, 0x1000u);
    far_f7c01f = 1u;
    ProcessMediaCoroutineState6();
    assert(ReadDword(0xfe9u) == 0x00034bc0ul);
    assert((memory_bytes[0x1302u] & 0x10u) == 0u);
}

static void TestMediaTypeTwoRetryPath(void)
{
    ResetFixture();
    memory_bytes[0x1008u] = 2u;
    ProcessMediaCoroutineState6();
    assert(reduction_count == 1u && reduction_argument == 1u);
    assert(measurement_retry_count == 1u);
    assert(calibration_count == 0u && yield_count == 0u);
}

static void TestCompletionBitShortCircuit(void)
{
    ResetFixture();
    WriteWord(0xfad7u, 0x1000u);
    set_completion_on_reduction = 1u;
    ProcessMediaCoroutineState6();
    assert(reduction_count == 1u && reduction_argument == 1u);
    assert(calibration_count == 0u);
    assert(clear_flags_count == 1u && yield_count == 1u);
    assert(memory_bytes[0xfaabu] == 0x12u);
    assert(ReadDword(0xe3bu) == 0x96ul);
    assert(memory_bytes[0xfafu] == 3u);
}

static void TestCalibrationFailureClasses(void)
{
    ResetFixture();
    calibration_result = 0u;
    memory_bytes[0xff2u] = 2u;
    ProcessMediaCoroutineState6();
    assert(clear_flags_count == 0u && yield_count == 0u);

    ResetFixture();
    calibration_result = 0u;
    memory_bytes[0xff2u] = 5u;
    ProcessMediaCoroutineState6();
    assert(clear_flags_count == 1u && yield_count == 1u);
    assert((memory_bytes[0x158eu] & 8u) == 0u);
    assert(memory_bytes[0xff1u] == 0u && memory_bytes[0xe44u] == 1u);
    assert((memory_bytes[0x1302u] & 0x10u) == 0u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);
}

static void TestSuccessfulCalibrationReacquisitionOutcomes(void)
{
    ResetFixture();
    acquisition_result = 0u;
    ProcessMediaCoroutineState6();
    assert(acquisition_count == 1u && yield_count == 0u);
    assert(p3_bit0 == 1u && p3_bit1 == 0u);

    ResetFixture();
    memory_bytes[0xe3fu] = 0u;
    ProcessMediaCoroutineState6();
    assert(yield_count == 1u);
    assert(ReadDword(0xe3bu) == 0x96ul);
    assert(memory_bytes[0xfafu] == 3u);
}

int main(void)
{
    TestSuccessfulLeadInPath();
    TestTargetAndDirectionVariants();
    TestMediaTypeTwoRetryPath();
    TestCompletionBitShortCircuit();
    TestCalibrationFailureClasses();
    TestSuccessfulCalibrationReacquisitionOutcomes();
    puts("media coroutine state-6 tests passed");
    return 0;
}
