#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_operating_code_interpolation_tables.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static float native_residue;
static float measurement_arguments[8];
static float adjustment_arguments[8];
static u16 adjustment_references[8];
static unsigned measurement_count;
static unsigned adjustment_count;

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

float ObserveOperatingInterpolationNativeStackResidue(void)
{
    return native_residue;
}

u16 ObserveOperatingInterpolationMeasurementBound(float value)
{
    assert(measurement_count < 8u);
    measurement_arguments[measurement_count] = value;
    return (u16)(0x2100u + measurement_count++);
}

u16 ObserveOperatingInterpolationAdjustment(float value, u16 reference)
{
    assert(adjustment_count < 8u);
    adjustment_arguments[adjustment_count] = value;
    adjustment_references[adjustment_count] = reference;
    return (u16)(0x0040u + adjustment_count++);
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(measurement_arguments, 0, sizeof(measurement_arguments));
    memset(adjustment_arguments, 0, sizeof(adjustment_arguments));
    memset(adjustment_references, 0, sizeof(adjustment_references));
    native_residue = 0.0f;
    measurement_count = adjustment_count = 0u;
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

static void AssertFloatNear(float actual, float expected)
{
    assert(fabs((double)actual - (double)expected) < 0.0002);
}

static float MeasurementBase(s16 delta)
{
    double value = (double)delta * 0.0043;

    return (float)(value - 4.6592);
}

static float AdjustmentBase(s16 delta, s16 reference_offset)
{
    double value = (double)delta * 0.0089;

    value = value + 14.3;
    value = value * 0.01;
    value = value * (double)reference_offset;
    value = value + (double)delta * 0.001;
    value = value - 12.0;
    return (float)value;
}

static float Interpolate(float low, float high, u8 low_code, u8 high_code, u8 code)
{
    float slope = (high - low) / (float)(high_code - low_code);
    float intercept = low - slope * (float)low_code;
    float value = slope * (float)code;

    return value + intercept;
}

static void TestRange4To10PreservesNativeResidueAndCrossTableReuse(void)
{
    float first_high;
    float first_last;
    float second_high;
    unsigned index;

    ResetFixture();
    memory[0x1058u] = 4u;
    memory[0x1059u] = 10u;
    native_residue = -3.25f;
    WriteU16(0xef87u, 1500u);
    WriteU16(0xf074u, 1000u);
    WriteU16(0xef85u, 910u);
    memory[0xf06au] = 10u;

    BuildControllerOperatingCodeInterpolationTables();

    first_high = MeasurementBase(500);
    first_last = Interpolate(native_residue, first_high, 4u, 10u, 10u);
    second_high = AdjustmentBase(500, 900);
    assert(measurement_count == 7u && adjustment_count == 7u);
    for(index = 0u; index < 7u; ++index)
    {
        AssertFloatNear(measurement_arguments[index], Interpolate(native_residue, first_high, 4u, 10u, (u8)(4u + index)));
        AssertFloatNear(adjustment_arguments[index], Interpolate(first_last, second_high, 4u, 10u, (u8)(4u + index)));
        assert(adjustment_references[index] == (u16)(0x2100u + index));
        assert(ReadU16(0xf187u + (fw_addr_t)((index + 4u) * 2u)) == (u16)(0x2100u + index));
        assert(ReadU16(0xf157u + (fw_addr_t)((index + 4u) * 2u)) == (u16)(0x0040u + index));
    }
}

static void TestRange20To24UsesBothProfileScalePairs(void)
{
    float first_low;
    float first_high;
    float second_low;
    float second_high;
    unsigned index;

    ResetFixture();
    memory[0x1058u] = 20u;
    memory[0x1059u] = 24u;
    memory[0xf05fu] = 1u;
    WriteU16(0xef87u, 1800u);
    WriteU16(0xf074u, 1000u);
    WriteU16(0xef85u, 850u);
    memory[0xf06au] = 10u;
    WriteU16(0xf83913u, 80u);
    WriteU16(0xf8393bu, 120u);
    WriteU16(0xf83911u, 75u);
    WriteU16(0xf83939u, 125u);

    BuildControllerOperatingCodeInterpolationTables();

    first_low = MeasurementBase(800) * 80.0f / 100.0f;
    first_high = MeasurementBase(800) * 120.0f / 100.0f;
    second_low = AdjustmentBase(800, 840) * 75.0f / 100.0f;
    second_high = AdjustmentBase(800, 840) * 125.0f / 100.0f;
    assert(measurement_count == 5u && adjustment_count == 5u);
    for(index = 0u; index < 5u; ++index)
    {
        AssertFloatNear(measurement_arguments[index], Interpolate(first_low, first_high, 20u, 24u, (u8)(20u + index)));
        AssertFloatNear(adjustment_arguments[index], Interpolate(second_low, second_high, 20u, 24u, (u8)(20u + index)));
        assert(adjustment_references[index] == (u16)(0x2100u + index));
    }
}

int main(void)
{
    TestRange4To10PreservesNativeResidueAndCrossTableReuse();
    TestRange20To24UsesBothProfileScalePairs();
    puts("controller operating-code interpolation table tests passed");
    return 0;
}
