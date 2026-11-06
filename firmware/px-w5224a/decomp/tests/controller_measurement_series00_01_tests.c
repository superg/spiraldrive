#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_series00_01.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 solve_count;
static u8 classify_count;
static u8 select_count;
static u32 observed_x2[2];
static u32 observed_x[2];
static u32 observed_y[2];
static u32 observed_xy[2];
static u8 observed_n[2];

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

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
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

void SetControllerMeasurementTerminalStatus0E02(void)
{
    memory[0x00f05dul] = 14u;
    memory[0x00f05eul] = 2u;
}

void SolveControllerMeasurementInterpolationAtSelectedInterval(void)
{
    u8 slot = solve_count++;

    assert(slot < 2u);
    observed_x2[slot] = ReadDword(0x0012e6ul);
    observed_x[slot] = ReadDword(0x0012eaul);
    observed_y[slot] = ReadDword(0x0012eeul);
    observed_xy[slot] = ReadDword(0x0012f2ul);
    observed_n[slot] = memory[0x0012f6ul];
    if(memory[0x000100ul] != 0u)
    {
        memory[0x00ef80ul] = 2u;
        return;
    }
    if(slot == 0u)
    {
        WriteFloat(0x0012e2ul, 0.0f);
        WriteFloat(0x0012deul, 0.5f);
    }
    else
    {
        WriteFloat(0x0012e2ul, 10.0f);
        WriteFloat(0x0012deul, 2.0f);
    }
}

void ClassifyAndAdjustControllerMeasurementForMode2(void)
{
    ++classify_count;
}

void SelectControllerMeasurementSeries00AverageOffset(void)
{
    ++select_count;
}

static void Reset(void)
{
    u8 index;

    memset(memory, 0, sizeof(memory));
    solve_count = 0u;
    classify_count = 0u;
    select_count = 0u;
    memset(observed_x2, 0, sizeof(observed_x2));
    memset(observed_x, 0, sizeof(observed_x));
    memset(observed_y, 0, sizeof(observed_y));
    memset(observed_xy, 0, sizeof(observed_xy));
    WriteDword(0x00f1bbul, 0x002000ul);
    memory[0x002008ul] = 100u;
    WriteWord(0x00f074ul, 100u);
    for(index = 0u; index < 15u; ++index)
    {
        WriteWord(0x00ef89ul + 2u * index, 55u);
        WriteWord(0x00efa7ul + 2u * index, 77u);
        WriteWord(0x00f03ful + 2u * index, (u16)(10 + (s16)index * (s16)index));
    }
}

static s16 SnapshotAverage(u8 index)
{
    s32 left = 10 + (s32)(index - 1u) * (index - 1u);
    s32 center = 10 + (s32)index * index;
    s32 right = 10 + (s32)(index + 1u) * (index + 1u);

    return (s16)((left + center + right) / 3L);
}

static void CheckConstantSeries(void)
{
    u8 index;

    Reset();
    EvaluateControllerMeasurementSeries00And01();
    assert(memory[0x00f05dul] == 14u && memory[0x00f05eul] == 2u);
    assert(ReadWord(0x00ef85ul) == 55u);
    assert(ReadWord(0x00ef87ul) == 77u);
    assert(memory[0x00ef80ul] == 1u);
    assert(solve_count == 0u && classify_count == 0u && select_count == 0u);
    for(index = 1u; index < 14u; ++index)
        assert((s16)ReadWord(0x00f03ful + 2u * index) == SnapshotAverage(index));
}

static void CheckFirstRegressionFailure(void)
{
    Reset();
    WriteWord(0x00ef89ul + 2u, 56u);
    memory[0x000100ul] = 1u;
    EvaluateControllerMeasurementSeries00And01();
    assert(solve_count == 1u);
    assert(memory[0x00ef80ul] == 2u);
    assert(classify_count == 0u && select_count == 0u);
}

static void CheckBothRegressions(void)
{
    u8 index;
    u32 expected_x2_0 = 0u;
    u32 expected_x_0 = 0u;
    u32 expected_x2_1 = 0u;
    u32 expected_x_1 = 0u;
    u32 expected_y = 0u;
    u32 expected_xy_0 = 0u;
    u32 expected_xy_1 = 0u;

    Reset();
    for(index = 0u; index < 15u; ++index)
    {
        WriteWord(0x00ef89ul + 2u * index, (u16)(index + 1u));
        WriteWord(0x00efa7ul + 2u * index, (u16)(2u * index + 3u));
        WriteWord(0x00f03ful + 2u * index, (u16)(30u + 3u * index));
    }
    for(index = 2u; index <= 14u; ++index)
    {
        u32 x0 = index + 1u;
        u32 x1 = 2u * index + 3u;
        u32 y = 30u + 3u * index;

        expected_x2_0 += x0 * x0;
        expected_x_0 += x0;
        expected_x2_1 += x1 * x1;
        expected_x_1 += x1;
        expected_y += y;
        expected_xy_0 += x0 * y;
        expected_xy_1 += x1 * y;
    }

    EvaluateControllerMeasurementSeries00And01();
    assert(solve_count == 2u);
    assert(observed_n[0] == 13u && observed_n[1] == 13u);
    assert(observed_x2[0] == expected_x2_0 && observed_x[0] == expected_x_0 && observed_y[0] == expected_y && observed_xy[0] == expected_xy_0);
    assert(observed_x2[1] == expected_x2_1 && observed_x[1] == expected_x_1 && observed_y[1] == expected_y && observed_xy[1] == expected_xy_1);
    assert(ReadWord(0x00ef85ul) == 126u);
    assert(ReadWord(0x00ef87ul) == 45u);
    assert(classify_count == 1u && select_count == 1u);
}

int main(void)
{
    CheckConstantSeries();
    CheckFirstRegressionFailure();
    CheckBothRegressions();
    return 0;
}
