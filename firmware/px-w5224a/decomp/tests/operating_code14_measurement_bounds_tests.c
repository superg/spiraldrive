#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/operating_code14_measurement_bounds.h"

static u8 firmware_memory[0x1000000ul];
static float update_argument;
static float adjustment_argument;
static u16 adjustment_reference;
static u16 update_result;
static u16 adjustment_result;
static unsigned update_count;
static unsigned adjustment_count;
static u8 mutate_on_offset_read;

static void WriteU16(fw_addr_t address, u16 value);

u8 FirmwareRead8(fw_addr_t address)
{
    u8 value;

    assert(address < sizeof(firmware_memory));
    value = firmware_memory[address];
    if(address == 0xf06aul && mutate_on_offset_read != 0u)
    {
        WriteU16(0xef87u, 2000u);
        WriteU16(0xf074u, 1000u);
        firmware_memory[0xf05fu] = 3u;
        mutate_on_offset_read = 0u;
    }
    return value;
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware_memory));
    firmware_memory[address] = value;
}

u16 ObserveUpdateOperatingCode14MeasurementBound(float value)
{
    update_argument = value;
    ++update_count;
    return update_result;
}

u16 ObserveComputeOperatingCode14BoundedAdjustment(float value, u16 reference_word)
{
    adjustment_argument = value;
    adjustment_reference = reference_word;
    ++adjustment_count;
    return adjustment_result;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    firmware_memory[address] = (u8)(value >> 8);
    firmware_memory[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)firmware_memory[address] << 8) | firmware_memory[address + 1u]);
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0, sizeof(firmware_memory));
    update_argument = 0.0f;
    adjustment_argument = 0.0f;
    adjustment_reference = 0u;
    update_result = 0x2345u;
    adjustment_result = 0x0067u;
    update_count = 0u;
    adjustment_count = 0u;
    mutate_on_offset_read = 0u;
}

static void AssertFloatNear(float actual, float expected)
{
    assert(fabs((double)actual - (double)expected) < 0.00002);
}

static void TestPositiveDeltaAndSelectorZero(void)
{
    s16 delta = 500;
    s16 reference_offset = 900;
    double first;
    double denominator;
    double transformed;
    float expected_update;
    float expected_adjustment;

    ResetFixture();
    WriteU16(0xef87u, 1500u);
    WriteU16(0xf074u, 1000u);
    WriteU16(0xef85u, 910u);
    firmware_memory[0xf06au] = 10u;
    WriteU16(0xf8390fu, 104u);
    WriteU16(0xf8390du, 98u);

    ComputeOperatingCode14ControllerMeasurementBounds();

    first = (double)delta * 0.0043;
    first = first - 4.6592;
    expected_update = (float)first;
    expected_update = expected_update * 104.0f;
    expected_update = expected_update / 100.0f;
    denominator = (double)delta * 0.0089;
    denominator = denominator + 14.3;
    denominator = denominator * 0.01;
    transformed = denominator * (double)reference_offset;
    transformed = transformed + (double)delta * 0.001;
    transformed = transformed - 12.0;
    expected_adjustment = (float)transformed;
    expected_adjustment = expected_adjustment * 98.0f;
    expected_adjustment = expected_adjustment / 100.0f;

    assert(update_count == 1u && adjustment_count == 1u);
    AssertFloatNear(update_argument, expected_update);
    AssertFloatNear(adjustment_argument, expected_adjustment);
    assert(adjustment_reference == update_result);
    assert(ReadU16(0xef83u) == update_result);
    assert(ReadU16(0xef81u) == adjustment_result);
}

static void TestWrappedNegativeValuesAndIndexedScales(void)
{
    s16 denominator_delta;
    s16 addition_delta;
    s16 reference_offset;
    double first;
    double denominator;
    double transformed;
    float expected_update;
    float expected_adjustment;

    ResetFixture();
    firmware_memory[0xf05fu] = 2u;
    WriteU16(0xef87u, 0x0010u);
    WriteU16(0xf074u, 0xfff0u);
    WriteU16(0xef85u, 3u);
    firmware_memory[0xf06au] = 9u;
    WriteU16(0xf83917u, (u16)-120);
    WriteU16(0xf83919u, 75u);
    update_result = 0xabcd;
    adjustment_result = 0x7e00u;
    mutate_on_offset_read = 1u;
    denominator_delta = (s16)(0x0010u - 0xfff0u);
    addition_delta = 1000;
    reference_offset = (s16)(3u - 9u);

    ComputeOperatingCode14ControllerMeasurementBounds();

    first = (double)denominator_delta * 0.0043;
    first = first - 4.6592;
    expected_update = (float)first;
    expected_update = expected_update * -120.0f;
    expected_update = expected_update / 100.0f;
    denominator = (double)denominator_delta * 0.0089;
    denominator = denominator + 14.3;
    denominator = denominator * 0.01;
    transformed = denominator * (double)reference_offset;
    transformed = transformed + (double)addition_delta * 0.001;
    transformed = transformed - 12.0;
    expected_adjustment = (float)transformed;
    expected_adjustment = expected_adjustment * 75.0f;
    expected_adjustment = expected_adjustment / 100.0f;

    AssertFloatNear(update_argument, expected_update);
    AssertFloatNear(adjustment_argument, expected_adjustment);
    assert(adjustment_reference == 0xabcdu);
    assert(ReadU16(0xef83u) == 0xabcdu);
    assert(ReadU16(0xef81u) == 0x7e00u);
}

int main(void)
{
    TestPositiveDeltaAndSelectorZero();
    TestWrappedNegativeValuesAndIndexedScales();
    puts("operating-code-14 measurement-bound tests passed");
    return 0;
}
