#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_measurement_mode6_series05.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u16 selected_threshold;
static u8 validator_result;
static u8 validated_index;
static u8 solver_calls;
static u8 selector_calls;

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

u32 MultiplyU32Low(u32 left, u32 right)
{
    return left * right;
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

static void WriteFloat(fw_addr_t address, float value)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.value = value;
    WriteDword(address, converted.bits);
}

u16 SelectMode6MeasurementThresholdFromProfile(void)
{
    return selected_threshold;
}

u8 ValidateControllerMeasurementSeriesOrderingAtIndex(u8 series_index)
{
    validated_index = series_index;
    return validator_result;
}

void SolveControllerMeasurementInterpolationAtSelectedInterval(void)
{
    ++solver_calls;
    if(solver_calls == 1u)
    {
        assert(ReadDword(0x0012e6ul) == 818ul);
        assert(ReadDword(0x0012f2ul) == 8180ul);
        assert(ReadDword(0x0012eaul) == 90ul);
        assert(ReadDword(0x0012eeul) == 900ul);
        assert(memory[0x12f6ul] == 12u);
        WriteFloat(0x0012e2ul, 0.0f);
        WriteFloat(0x0012deul, 4.0f);
    }
    else
    {
        assert(ReadDword(0x0012e6ul) == 3272ul);
        assert(ReadDword(0x0012f2ul) == 16360ul);
        assert(ReadDword(0x0012eaul) == 180ul);
        assert(ReadDword(0x0012eeul) == 900ul);
        assert(memory[0x12f6ul] == 12u);
        WriteFloat(0x0012e2ul, 0.0f);
        WriteFloat(0x0012deul, 0.125f);
    }
}

void SelectControllerMeasurementSeries00AverageOffset(void)
{
    ++selector_calls;
}

static void Reset(void)
{
    u8 index;

    memset(memory, 0, sizeof(memory));
    selected_threshold = 600u;
    validator_result = 1u;
    validated_index = 0xffu;
    solver_calls = 0u;
    selector_calls = 0u;
    for(index = 0u; index < 15u; ++index)
    {
        WriteWord(0x00ef89ul + (fw_addr_t)((u16)index * 2u), index);
        WriteWord(0x00efa7ul + (fw_addr_t)((u16)index * 2u), (u16)((u16)index * 2u));
        WriteWord(0x00f01ful + (fw_addr_t)((u16)index * 2u), (u16)((u16)index * 10u));
    }
}

static void CheckOrderingFailure(void)
{
    Reset();
    selected_threshold = 55u;
    validator_result = 0u;
    DeriveMode6ControllerMeasurementStateFromSeries05();
    assert(validated_index == 13u);
    assert(memory[0xef80ul] == 2u);
    assert(solver_calls == 0u && selector_calls == 0u);
}

static void CheckFirstRegressionFailure(void)
{
    Reset();
    memory[0xef80ul] = 2u;
    DeriveMode6ControllerMeasurementStateFromSeries05();
    assert(validated_index == 0u);
    assert(solver_calls == 1u && selector_calls == 0u);
}

static void CheckTwoRegressionsAndCaps(void)
{
    Reset();
    WriteWord(0x00f074ul, 0u);
    DeriveMode6ControllerMeasurementStateFromSeries05();
    assert(validated_index == 0u);
    assert(solver_calls == 2u && selector_calls == 1u);
    assert(ReadWord(0x00ef85ul) == 126u);
    assert(ReadWord(0x00ef87ul) == 0x1100u);
    assert(memory[0xef80ul] == 1u);
    assert(memory[0xf05dul] == 13u && memory[0xf05eul] == 2u);
}

int main(void)
{
    CheckOrderingFailure();
    CheckFirstRegressionFailure();
    CheckTwoRegressionsAndCaps();
    puts("controller_measurement_mode6_series05_tests: ok");
    return 0;
}
