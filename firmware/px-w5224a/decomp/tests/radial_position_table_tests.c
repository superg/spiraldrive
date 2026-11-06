#include <assert.h>
#include <math.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/radial_position_table.h"

static u8 memory[1u << 24];
static s32 map_arguments[4];
static float map_results[4];
static unsigned map_count, recalculate_count, build_count;
static u16 build_argument;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

float MapFramePositionToRadialModelFloat(s32 frame_position)
{
    map_arguments[map_count] = frame_position;
    return map_results[map_count++];
}

void RecalculateRadialOperatingThresholdState(void)
{
    ++recalculate_count;
}

void BuildFixedAndGeneratedControllerRegisterProfiles(u16 profile_input)
{
    ++build_count;
    build_argument = profile_input;
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

static u16 ExpectedThreshold(float radial_value, float factor, float numerator_source)
{
    volatile float scaled = radial_value * factor;
    volatile float numerator = numerator_source * 500.0f;
    volatile float result = numerator / scaled;

    return (u16)(s32)result;
}

static u16 ExpectedTableValue(u8 index, u32 scale, u16 span)
{
    volatile float table_scale = (float)scale;
    volatile float value;
    volatile float span_value;
    volatile double root;

    table_scale = table_scale / 65.536f;
    value = (float)index;
    value = value * 6.0f;
    value = value * table_scale;
    span_value = (float)span;
    value = value * span_value;
    value = value / 1000.0f;
    value = value / 3.1415927f;
    value = value + 62500.0f;
    root = sqrt((double)value);
    value = (float)root;
    value = value / 10.0f;
    value = value * 1000.0f;
    value = value / 11.72f;
    return (u16)(s32)value;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(map_arguments, 0, sizeof(map_arguments));
    memset(map_results, 0, sizeof(map_results));
    map_results[0] = 1000.0f;
    map_results[1] = 2000.0f;
    map_count = recalculate_count = build_count = 0u;
    build_argument = 0u;
    WriteU32(0x000cf5ul, 65536ul);
    WriteU16(0x000cf9ul, 1500u);
    WriteU16(0x00f9feul, 0x3456u);
}

static void AssertCompleteTable(u32 scale, u16 span)
{
    unsigned index;

    for(index = 0u; index < 160u; ++index)
        assert(ReadU16(0x000cfbul + (fw_addr_t)(index * 2u)) == ExpectedTableValue((u8)index, scale, span));
}

static void TestDefaultEndpointAndStateOneProfile(void)
{
    ResetState();
    WriteU32(0x0012c1ul, 400000ul);
    memory[0x001008ul] = 1u;

    RebuildRadialPositionTableAndThresholds();
    assert(map_count == 1u && map_arguments[0] == 306000l);
    assert(ReadU16(0x00105aul) == 10000u);
    assert(ReadU16(0x000ce9ul) == ExpectedThreshold(1000.0f, 10.0f, 7056.0f));
    assert(ReadU16(0x000cebul) == ExpectedThreshold(1000.0f, 24.0f, 28224.0f));
    assert(ReadU16(0x000cedul) == ExpectedThreshold(1000.0f, 32.2f, 28224.0f));
    assert(ReadU16(0x000ceful) == ExpectedThreshold(1000.0f, 38.0f, 28224.0f));
    assert(recalculate_count == 1u);
    assert(memory[0x00fa05ul] == 0x34u);
    assert(build_count == 1u && build_argument == 0x3456u);
    AssertCompleteTable(65536ul, 1500u);
}

static void TestFlaggedEndpointAnd402Factor(void)
{
    ResetState();
    memory[0x00f9e9ul] = 4u;
    WriteU32(0x0012c1ul, 306001ul);

    RebuildRadialPositionTableAndThresholds();
    assert(map_count == 2u);
    assert(map_arguments[0] == 306000l && map_arguments[1] == 306001l);
    assert(ReadU16(0x00105aul) == 20000u);
    assert(ReadU16(0x000ceful) == ExpectedThreshold(2000.0f, 40.2f, 28224.0f));
    assert(build_count == 0u);
}

static void TestNegativeEndpointUses38Factor(void)
{
    ResetState();
    memory[0x00f9e9ul] = 4u;
    WriteU32(0x0012c1ul, (u32)-1l);

    RebuildRadialPositionTableAndThresholds();
    assert(map_count == 1u);
    assert(ReadU16(0x000ceful) == ExpectedThreshold(1000.0f, 38.0f, 28224.0f));
}

int main(void)
{
    TestDefaultEndpointAndStateOneProfile();
    TestFlaggedEndpointAnd402Factor();
    TestNegativeEndpointUses38Factor();
    return 0;
}
