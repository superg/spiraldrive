#include <assert.h>
#include <math.h>
#include <string.h>
#include "platform/controller_measurement_mode6_adjustment.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];

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

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
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
        float value;
        u32 bits;
    } converted;

    converted.value = value;
    WriteDword(address, converted.bits);
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    WriteWord(0x00efabul, 1000u);
    WriteWord(0x00efbful, 2000u);
    WriteWord(0x00f074ul, 1000u);
    WriteWord(0x00f089ul, 100u);
    WriteDword(0x00f1bbul, 0x002000ul);
    WriteWord(0x002004ul, (u16)(s16)80);
    WriteFloat(0x00f06eul, 2.5f);
    memory[0x00f06aul] = 4u;
    memory[0x00ef73ul] = 6u;
    memory[0x00ef72ul] = 0x41u;
}

static void ExpectedOutputs(float adjusted_value, float output_measurement, u16 *reference_word, u16 *output_code)
{
    float narrowed = (float)((double)adjusted_value + 4.6592);
    s32 rounded = (s32)ceil((double)narrowed / 0.0043);
    u16 reference = (u16)(1000u + (u16)rounded);
    u16 delta = (u16)(reference - 1000u);
    double value = (double)delta * 0.001;

    value = value - (double)output_measurement;
    value = value + 12.0;
    value = value / (((double)delta * 0.0089 + 14.3) * 0.01);
    value = value + (double)(s8)memory[0x00f06aul];
    narrowed = (float)value;
    rounded = (s32)ceil((double)narrowed);
    *reference_word = reference;
    *output_code = (u16)(u8)rounded;
}

static void CheckClassificationAndGate(void)
{
    Reset();
    WriteWord(0x00ef87ul, 999u);
    memory[0x00ef73ul] = 5u;
    ClassifyAndAdjustControllerMeasurementForMode6();
    assert(memory[0x00ef80ul] == 3u);
    assert(memory[0x00ef72ul] == 0x41u);

    WriteWord(0x00ef87ul, 1000u);
    memory[0x00ef73ul] = 6u;
    ClassifyAndAdjustControllerMeasurementForMode6();
    assert(memory[0x00ef80ul] == 1u);
    assert(memory[0x00ef72ul] == 0x41u);

    WriteWord(0x00ef87ul, 2001u);
    memory[0x00ef73ul] = 5u;
    ClassifyAndAdjustControllerMeasurementForMode6();
    assert(memory[0x00ef80ul] == 4u);
    assert(memory[0x00ef72ul] == 0x41u);
}

static void CheckSpecialHigh(void)
{
    u16 reference;
    u16 output;
    float adjusted;

    Reset();
    memory[0x00fb88ul] = 2u;
    WriteWord(0x00ef87ul, 2001u);
    adjusted = (float)((double)(2.5f * 120.0f) * 0.001);
    ExpectedOutputs(adjusted, 2.5f - 3.0f, &reference, &output);
    ClassifyAndAdjustControllerMeasurementForMode6();
    assert(memory[0x00ef80ul] == 4u);
    assert(memory[0x00f08bul] == 1u);
    assert(ReadWord(0x00f089ul) == 120u);
    assert(ReadWord(0x00f117ul) == reference);
    assert(ReadWord(0x00f115ul) == output);
    assert(memory[0x00ef72ul] == 0x61u);
}

static void CheckSpecialLowWithoutWordAdjustment(void)
{
    u16 reference;
    u16 output;
    float adjusted;

    Reset();
    memory[0x00fb88ul] = 3u;
    memory[0x00f08bul] = 1u;
    WriteWord(0x00ef87ul, 999u);
    adjusted = (float)((double)(2.5f * 100.0f) * 0.001);
    ExpectedOutputs(adjusted, 2.5f + 2.0f, &reference, &output);
    ClassifyAndAdjustControllerMeasurementForMode6();
    assert(memory[0x00ef80ul] == 3u);
    assert(memory[0x00f08bul] == 2u);
    assert(ReadWord(0x00f089ul) == 100u);
    assert(ReadWord(0x00f117ul) == reference);
    assert(ReadWord(0x00f115ul) == output);
}

static void CheckOrdinaryHighAndLow(void)
{
    u16 reference;
    u16 output;
    float adjusted;

    Reset();
    WriteWord(0x00ef87ul, 2001u);
    adjusted = (float)(((double)80 * 0.001 + 0.04) * 2.5);
    ExpectedOutputs(adjusted, 2.5f, &reference, &output);
    ClassifyAndAdjustControllerMeasurementForMode6();
    assert(memory[0x00ef80ul] == 4u);
    assert(ReadWord(0x00f117ul) == reference);
    assert(ReadWord(0x00f115ul) == output);

    Reset();
    WriteWord(0x00ef87ul, 999u);
    memory[0x00f06aul] = (u8)-4;
    adjusted = (float)(((double)80 * 0.001 - 0.03) * 2.5);
    ExpectedOutputs(adjusted, 2.5f, &reference, &output);
    ClassifyAndAdjustControllerMeasurementForMode6();
    assert(memory[0x00ef80ul] == 3u);
    assert(ReadWord(0x00f117ul) == reference);
    assert(ReadWord(0x00f115ul) == output);
}

int main(void)
{
    CheckClassificationAndGate();
    CheckSpecialHigh();
    CheckSpecialLowWithoutWordAdjustment();
    CheckOrdinaryHighAndLow();
    return 0;
}
