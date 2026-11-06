#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_mode6_transform.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static float gradients[15];
static u8 derived_count;

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

float ComputeControllerMeasurementSeries04ValueAtIndex(u8 series_index)
{
    assert(series_index < 15u);
    return gradients[series_index];
}

void DeriveMode6ControllerMeasurementStateFromSeries05(void)
{
    ++derived_count;
}

void DeriveControllerMeasurementStateFromSeries05(void)
{
    assert(0);
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

static s16 ClampRaw(s32 value)
{
    if(value > 0x7fffL)
        return 0x7fff;
    if(value < -0x8000L)
        return (s16)-0x8000L;
    return (s16)value;
}

static void Reset(void)
{
    u8 index;

    memset(memory, 0, sizeof(memory));
    memset(gradients, 0, sizeof(gradients));
    derived_count = 0u;
    WriteWord(0x00f074ul, 1000u);
    for(index = 0u; index < 15u; ++index)
    {
        WriteWord(0x00efa7ul + (fw_addr_t)((u16)index * 2u), 1000u);
        WriteWord(0x00ef89ul + (fw_addr_t)((u16)index * 2u), 100u);
        WriteWord(0x00f001ul + (fw_addr_t)((u16)index * 2u), 10u);
        gradients[index] = (float)(index + 1u);
    }
}

static s16 ExpectedRaw(u8 index)
{
    u16 offset = (u16)index * 2u;
    u16 reference_delta = (u16)(ReadWord(0x00efa7ul + offset) - ReadWord(0x00f074ul));
    double scale = ((double)reference_delta * 0.0089 + 14.3) * 0.01;
    u16 measurement_delta = (u16)(ReadWord(0x00ef89ul + offset) - memory[0x00f06aul]);
    double coordinate = (double)measurement_delta * scale;
    float narrowed;
    u16 denominator = ReadWord(0x00f001ul + offset);

    reference_delta = (u16)(ReadWord(0x00efa7ul + offset) - ReadWord(0x00f074ul));
    coordinate = coordinate + (double)reference_delta * 0.001;
    coordinate = coordinate - 12.0;
    narrowed = (float)coordinate;
    if(denominator == 0u)
        return 0x7fff;
    narrowed = gradients[index] * narrowed;
    narrowed = narrowed / (float)(s16)denominator;
    narrowed = narrowed * 1000.0f;
    return ClampRaw((s32)narrowed);
}

static void CheckTransformSaturationAndSnapshotSmoothing(void)
{
    s16 raw[15];
    u8 index;

    Reset();
    memory[0x00f06aul] = 1u;
    WriteWord(0x00efa7ul + 2u * 4u, 1100u);
    WriteWord(0x00efa7ul + 2u * 5u, 1100u);
    WriteWord(0x00f001ul + 2u * 7u, 0u);
    gradients[4] = 200.0f;
    gradients[5] = -200.0f;
    for(index = 0u; index < 15u; ++index)
        raw[index] = ExpectedRaw(index);

    TransformControllerMeasurementSeries04To05ForMode6();

    assert(ReadWord(0x00f01ful) == 0x7fffu);
    assert(ReadWord(0x00f03bul) == 0x8000u);
    for(index = 1u; index < 14u; ++index)
    {
        s32 expected = ((s32)raw[index - 1u] + raw[index] + raw[index + 1u]) / 3L;

        assert((s16)ReadWord(0x00f01ful + 2u * index) == ClampRaw(expected));
    }
    assert(raw[4] == 0x7fff);
    assert(raw[5] == (s16)-0x8000L);
    assert(raw[7] == 0x7fff);
    assert(derived_count == 1u);
}

int main(void)
{
    CheckTransformSaturationAndSnapshotSmoothing();
    return 0;
}
