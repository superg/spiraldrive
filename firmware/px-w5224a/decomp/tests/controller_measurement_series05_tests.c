#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_measurement_series05.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u16 duration_value;
static u8 validator_result;
static u8 validated_index;
static u16 selected_scale;
static u16 inherited_scale_word;
static float bounded_measurement;
static u16 bounded_reference;
static u8 bounded_calls;

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

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadDword(fw_addr_t address)
{
    u32 value = (u32)memory[address] << 24;

    value |= (u32)memory[address + 1u] << 16;
    value |= (u32)memory[address + 2u] << 8;
    value |= memory[address + 3u];
    return value;
}

static float ReadFloat(fw_addr_t address)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.bits = ReadDword(address);
    return converted.value;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static float AbsoluteFloat(float value)
{
    return value < 0.0f ? -value : value;
}

u16 SelectMeasurementDurationFromOperatingCode(void)
{
    return duration_value;
}

u8 ValidateControllerMeasurementSeriesOrderingAtIndex(u8 series_index)
{
    validated_index = series_index;
    return validator_result;
}

u16 SelectControllerMeasurementScalePercent(u16 inherited_word)
{
    inherited_scale_word = inherited_word;
    return selected_scale;
}

u16 ComputeBoundedControllerAdjustmentCode(float measurement_value, u16 reference_word)
{
    bounded_measurement = measurement_value;
    bounded_reference = reference_word;
    ++bounded_calls;
    return 77u;
}

double ObserveControllerMeasurementCeil(double value)
{
    s32 integer = (s32)value;

    if((double)integer < value)
        ++integer;
    return (double)integer;
}

static void Reset(void)
{
    u8 index;

    memset(memory, 0, sizeof(memory));
    duration_value = 1000u;
    validator_result = 1u;
    validated_index = 0xffu;
    selected_scale = 100u;
    inherited_scale_word = 0u;
    bounded_measurement = 0.0f;
    bounded_reference = 0u;
    bounded_calls = 0u;
    WriteDword(0x00f1bbul, 0x00002000ul);
    WriteWord(0x00002004ul, 1000u);
    WriteWord(0x00f074ul, 100u);
    WriteWord(0x00f072ul, 2000u);
    for(index = 0u; index < 15u; ++index)
        WriteWord(0x00f01ful + (fw_addr_t)((u16)index * 2u), 800u);
    WriteWord(0x00f01ful + 10u, 1200u);
    WriteWord(0x00f01ful + 12u, 800u);
    WriteWord(0x00ef89ul + 10u, 100u);
    WriteWord(0x00ef89ul + 12u, 90u);
    WriteWord(0x00efa7ul + 10u, 300u);
    WriteWord(0x00efa7ul + 12u, 200u);
}

static void CheckOrderingFailure(void)
{
    Reset();
    validator_result = 0u;
    DeriveControllerMeasurementStateFromSeries05();
    assert(validated_index == 5u);
    assert(memory[0xef80ul] == 2u && bounded_calls == 0u);
}

static void CheckOrdinaryInterpolation(void)
{
    float position;

    Reset();
    DeriveControllerMeasurementStateFromSeries05();
    position = ReadFloat(0x00f06eul);
    assert(validated_index == 5u);
    assert(AbsoluteFloat(ReadFloat(0x0012deul) - 159.42604f) < 0.001f);
    assert(AbsoluteFloat(ReadFloat(0x0012e2ul) - 517.6565f) < 0.001f);
    assert(AbsoluteFloat(position - 3.0255f) < 0.0001f);
    assert(inherited_scale_word == 0x4041u);
    assert(ReadWord(0x00f060ul) == 302u);
    assert(ReadWord(0x00f062ul) == 302u);
    assert(ReadWord(0x00f064ul) == 302u);
    assert(ReadWord(0x00ef87ul) == 1888u);
    assert(AbsoluteFloat(bounded_measurement - position) < 0.0001f);
    assert(bounded_reference == 1888u && bounded_calls == 1u);
    assert(ReadWord(0x00ef85ul) == 77u && memory[0xef80ul] == 1u);
}

static void CheckOptionalAverageAndCap(void)
{
    Reset();
    memory[0xfb88ul] = 2u;
    memory[0xf05ful] = 3u;
    selected_scale = 120u;
    WriteWord(0x00f83070ul + 6u, 100u);
    WriteWord(0x00002004ul, 30000u);
    DeriveControllerMeasurementStateFromSeries05();
    assert(AbsoluteFloat(ReadFloat(0x00f06eul) - 7.8153f) < 0.001f);
    assert(ReadWord(0x00ef87ul) == 0x1164u);
    assert(bounded_reference == 0x1164u);
}

static void CheckModeZeroScale105Retry(void)
{
    Reset();
    WriteWord(0x00f072ul, 1800u);
    DeriveControllerMeasurementStateFromSeries05();
    assert(AbsoluteFloat(ReadFloat(0x00f06eul) - 3.176775f) < 0.0001f);
    assert(ReadWord(0x00ef87ul) == 1923u);
    assert(AbsoluteFloat(bounded_measurement - 3.176775f) < 0.0001f);
    assert(bounded_reference == 1923u);
}

int main(void)
{
    CheckOrderingFailure();
    CheckOrdinaryInterpolation();
    CheckOptionalAverageAndCap();
    CheckModeZeroScale105Retry();
    puts("controller_measurement_series05_tests: ok");
    return 0;
}
