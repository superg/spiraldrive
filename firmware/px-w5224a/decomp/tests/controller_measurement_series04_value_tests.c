#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_measurement_series04_value.h"
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

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    WriteDword(0x00f1bbul, 0x00002000ul);
    memory[0x2003ul] = 10u;
    WriteWord(0x00efa7ul, 100u);
    WriteWord(0x00f074ul, 100u);
    WriteWord(0x00f001ul, 10u);
    WriteWord(0x00f003ul, 20u);
    WriteWord(0x00f005ul, 30u);
    WriteWord(0x00f019ul, 10u);
    WriteWord(0x00f01bul, 20u);
    WriteWord(0x00f01dul, 30u);
    WriteWord(0x00f00dul, 10u);
    WriteWord(0x00f011ul, 30u);
}

static void CheckAllIndexForms(void)
{
    float expected = 10.0f / 1.43f;

    Reset();
    assert(AbsoluteFloat(ComputeControllerMeasurementSeries04ValueAtIndex(0u) - expected) < 0.0001f);
    assert(AbsoluteFloat(ComputeControllerMeasurementSeries04ValueAtIndex(7u) - expected) < 0.0001f);
    assert(AbsoluteFloat(ComputeControllerMeasurementSeries04ValueAtIndex(14u) - expected) < 0.0001f);
}

static void CheckReferenceDeltaAndSignedProfile(void)
{
    float expected;

    Reset();
    WriteWord(0x00efa7ul, 200u);
    memory[0x2003ul] = (u8)-10;
    expected = 10.0f / -1.519f;
    assert(AbsoluteFloat(ComputeControllerMeasurementSeries04ValueAtIndex(7u) - expected) < 0.0001f);
}

int main(void)
{
    CheckAllIndexForms();
    CheckReferenceDeltaAndSignedProfile();
    puts("controller_measurement_series04_value_tests: ok");
    return 0;
}
