#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/strategy_profile_initialization.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 applied_speeds[8];
static unsigned applied_speed_count;
static unsigned multiply_count;
static u8 mutate_indices[3];
static unsigned mutate_index_count;

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

u32 ObserveStrategyProfileMultiply(u32 factor_a, u32 factor_b)
{
    u32 result;

    assert(factor_a == 15ul);
    result = factor_a * factor_b;
    if(multiply_count < mutate_index_count)
        memory[0xfb5du] = mutate_indices[multiply_count];
    ++multiply_count;
    return result;
}

void ObserveStrategyProfileWriteSpeed(u8 write_speed_x)
{
    assert(applied_speed_count < 8u);
    applied_speeds[applied_speed_count++] = write_speed_x;
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

static void ResetFixture(void)
{
    memset(memory, 0xa5, sizeof(memory));
    memset(applied_speeds, 0, sizeof(applied_speeds));
    memset(mutate_indices, 0, sizeof(mutate_indices));
    applied_speed_count = 0u;
    multiply_count = 0u;
    mutate_index_count = 0u;
    memory[0x1159u] = 0u;
}

static void TestNonMediaStateDefaultsAndSpeedSelection(void)
{
    static const struct
    {
        u8 selector;
        u8 expected_speed;
    } cases[] = {
        { 2u, 24u },
        { 1u, 10u },
        { 0u, 4u  },
        { 3u, 4u  }
    };
    unsigned case_index;
    unsigned index;

    for(case_index = 0u; case_index < sizeof(cases) / sizeof(cases[0]); ++case_index)
    {
        ResetFixture();
        memory[0x1008u] = 0u;
        memory[0xfb88u] = cases[case_index].selector;
        WriteU16(0xfad1u, 0xffffu);
        WriteU16(0xf9fau, 0xffffu);

        InitializeMediaStrategyProfileAndWriteSpeedSelection();

        for(index = 0u; index < 4u; ++index)
        {
            assert(ReadU16(0xfb61u + index * 2u) == 40u);
            assert(memory[0xf963u + index] == 0u);
            assert(memory[0xf968u + index] == 0u);
        }
        assert(ReadU16(0xfad1u) == 0u && ReadU16(0xf9fau) == 0u);
        assert(applied_speed_count == 1u);
        assert(applied_speeds[0] == cases[case_index].expected_speed);
        assert(multiply_count == 0u);
    }
}

static void TestSnapshotSuppressesFinalSpeedApplication(void)
{
    ResetFixture();
    memory[0x1008u] = 0u;
    memory[0x1159u] = 1u;

    InitializeMediaStrategyProfileAndWriteSpeedSelection();

    assert(applied_speed_count == 0u);
}

static void FillRecord(u8 record_index, u8 seed)
{
    fw_addr_t record = 0x8000ul + (u32)record_index * 15ul;
    u8 index;

    for(index = 0u; index < 15u; ++index)
        memory[record + index] = (u8)(seed + index);
}

static void TestMediaStateRecordFieldsAndFreshIndexReads(void)
{
    unsigned index;

    ResetFixture();
    memory[0x1008u] = 1u;
    WriteU32(0x1153u, 0x4ce78ul);
    memory[0xfb5du] = 1u;
    memory[0xfae0u] = 0x08u;
    WriteU16(0xfad7u, 0x0002u);
    FillRecord(1u, 1u);
    FillRecord(2u, 0x20u);
    FillRecord(3u, 0x40u);
    memory[0x8000u + 2u * 15u + 12u] = 0x18u;
    WriteU16(0x8000u + 3u * 15u + 13u, 0x1234u);
    mutate_indices[0] = 2u;
    mutate_indices[1] = 3u;
    mutate_indices[2] = 4u;
    mutate_index_count = 3u;

    InitializeMediaStrategyProfileAndWriteSpeedSelection();

    for(index = 0u; index < 4u; ++index)
        assert(ReadU16(0xfb61u + index * 2u) == (u16)((1u + index) * 10u));
    assert(memory[0xf963u] == 5u && memory[0xf964u] == 6u && memory[0xf965u] == 7u && memory[0xf966u] == 8u);
    assert(memory[0xf968u] == 9u && memory[0xf969u] == 10u && memory[0xf96au] == 11u && memory[0xf96bu] == 12u);
    assert(ReadU16(0xf9fau) == 0x1234u);
    assert(applied_speed_count == 1u && applied_speeds[0] == 0x18u);
    assert(multiply_count == 3u && memory[0xfb5du] == 4u);
}

static void TestCapacityFlagAndCandidateOverrides(void)
{
    fw_addr_t record = 0x8000ul + 10ul * 15ul;

    ResetFixture();
    memory[0x1008u] = 1u;
    memory[0xfb5du] = 10u;
    memory[0xfb5eu] = 0u;
    memory[0xfbecu] = 0u;
    WriteU32(0x1153u, 0x20000ul);
    WriteU16(0xfad7u, 0x0002u);
    memory[0xfae0u] = 0u;
    memory[record + 12u] = 0x18u;
    WriteU16(record + 13u, 0u);

    InitializeMediaStrategyProfileAndWriteSpeedSelection();

    assert((ReadU16(0xfad7u) & 0x10u) != 0u);
    assert(ReadU16(0xf9fau) == 3000u);
    assert(applied_speeds[0] == 0x34u);

    ResetFixture();
    memory[0x1008u] = 1u;
    memory[0xfb5du] = 10u;
    memory[0xfbecu] = 1u;
    memory[0xfb5eu] = 1u;
    memory[0xfae0u] = 0x08u;
    memory[0xf7c01fu] = 1u;
    WriteU32(0x1153u, 0x15f90ul);
    WriteU16(0xfad7u, 0x0012u);
    memory[record + 12u] = 0x18u;
    WriteU16(record + 13u, 0u);

    InitializeMediaStrategyProfileAndWriteSpeedSelection();

    assert((ReadU16(0xfad7u) & 0x10u) == 0u);
    assert(ReadU16(0xf9fau) == 0x0af0u);
    assert(applied_speeds[0] == 0x20u);
}

int main(void)
{
    TestNonMediaStateDefaultsAndSpeedSelection();
    TestSnapshotSuppressesFinalSpeedApplication();
    TestMediaStateRecordFieldsAndFreshIndexReads();
    TestCapacityFlagAndCandidateOverrides();
    puts("strategy profile initialization tests passed");
    return 0;
}
