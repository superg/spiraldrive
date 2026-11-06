#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/raw_read_preparation_segments_clone.h"

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

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    WriteU32(0x1430u, 0x11223344ul);
    memory[0x1438u] = 0x55u;
    WriteU32(0x1434u, 0x66778899ul);
    memory[0x1439u] = 0xaau;
    memory[0x0c44u] = 1u;
    WriteU16(0x0c4fu, 0x1234u);
}

static void AssertSegments(u32 first_count, u8 first_mode, u32 second_count, u8 second_mode)
{
    assert(ReadU32(0x1430u) == first_count);
    assert(memory[0x1438u] == first_mode);
    assert(ReadU32(0x1434u) == second_count);
    assert(memory[0x1439u] == second_mode);
}

static void TestEveryInitialPlanCode(void)
{
    unsigned plan;

    for(plan = 0u; plan < 256u; ++plan)
    {
        ResetFixture();
        InitializeRawReadPreparationSegmentsFromPlanCodeClone((u8)plan);
        if(plan == 2u)
            AssertSegments(75ul, 1u, 150ul, 2u);
        else if(plan == 3u || plan == 4u)
            AssertSegments(75ul, 3u, 150ul, 2u);
        else if(plan == 5u || plan == 6u || plan == 8u)
            AssertSegments(0ul, 0u, 150ul, 2u);
        else if(plan == 7u || plan == 9u || plan == 10u)
            AssertSegments(0ul, 0u, 0x1234ul, 1u);
        else if(plan == 11u)
            AssertSegments(0ul, 0u, 5ul, 2u);
        else
            AssertSegments(0x11223344ul, 0x55u, 0x66778899ul, 0xaau);
    }
}

static void TestWriteAndMediaOverrides(void)
{
    ResetFixture();
    memory[0x0c44u] = 0u;
    memory[0xdd34u] = 3u;
    memory[0x0c48u] = 5u;
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(0u);
    AssertSegments(0ul, 0u, 5ul, 2u);

    ResetFixture();
    memory[0x0c44u] = 0u;
    memory[0x14b3u] = 1u;
    memory[0x0c48u] = 0x80u;
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(0u);
    AssertSegments(0ul, 0u, 5ul, 3u);

    ResetFixture();
    memory[0x0c9eu] = 1u;
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(2u);
    AssertSegments(0ul, 0u, 5ul, 2u);
}

static void SetTypeTwoDistance(u32 reference_frame, u32 start_frame)
{
    memory[0xdd34u] = 2u;
    WriteU32(0xdd39u, reference_frame);
    WriteU32(0xdd3du, start_frame);
}

static void TestTypeTwoDistanceRoutes(void)
{
    ResetFixture();
    SetTypeTwoDistance(1000ul, 1151ul);
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(2u);
    AssertSegments(1ul, 1u, 150ul, 2u);

    ResetFixture();
    SetTypeTwoDistance(1000ul, 1151ul);
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(3u);
    AssertSegments(1ul, 0u, 150ul, 2u);

    ResetFixture();
    SetTypeTwoDistance(1000ul, 1151ul);
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(7u);
    AssertSegments(0ul, 0u, 151ul, 1u);

    ResetFixture();
    SetTypeTwoDistance(1000ul, 1151ul);
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(11u);
    AssertSegments(1ul, 0u, 150ul, 2u);

    ResetFixture();
    SetTypeTwoDistance(1000ul, 1150ul);
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(8u);
    AssertSegments(0ul, 0u, 150ul, 2u);

    ResetFixture();
    SetTypeTwoDistance(1000ul, 999ul);
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(10u);
    AssertSegments(0ul, 0u, 0xfffffffful, 1u);

    ResetFixture();
    SetTypeTwoDistance(0ul, 0x80000000ul);
    InitializeRawReadPreparationSegmentsFromPlanCodeClone(1u);
    AssertSegments(0ul, 0u, 0x80000000ul, 0xaau);
}

int main(void)
{
    TestEveryInitialPlanCode();
    TestWriteAndMediaOverrides();
    TestTypeTwoDistanceRoutes();
    puts("raw-read preparation segment clone tests passed");
    return 0;
}
