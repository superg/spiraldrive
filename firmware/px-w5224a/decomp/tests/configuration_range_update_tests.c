#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/configuration_range_update.h"

#define MEMORY_SIZE 0x20000u

static u8 memory[MEMORY_SIZE];
static u8 persistent_image[0xa0];
static u8 interrupt_clear_count;
static u8 persist_count;
static u8 persisted_code;
static u8 recalculate_count;
static u8 mutate_after_recalculate;
static u8 program_result;
static u8 program_count;
static u8 program_extended_only;
static u8 mismatch_primary_word;
static u8 mismatch_extended_word;
static fw_addr_t read_offset;
static fw_addr_t read_destination;
static u16 read_count;
static u8 volatile_source_reads;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x00008000ul && volatile_source_reads != 0u)
    {
        ++volatile_source_reads;
        return volatile_source_reads == 2u ? 0x55u : 0xaau;
    }
    assert(address < MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory[address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    (void)address;
    return 0u;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    (void)address;
    (void)value;
}

void FirmwareWriteBit0547(u8 value)
{
    assert(value == 0u);
    ++interrupt_clear_count;
}

void ReadDriveConfigurationBytes(fw_addr_t offset, fw_addr_t destination, u16 count)
{
    read_offset = offset;
    read_destination = destination;
    read_count = count;
    assert(offset == 0ul && destination == 0x001620ul && count == 0xa0u);
    memcpy(&memory[0x1620u], persistent_image, sizeof(persistent_image));
}

static u16 SumConfigurationBytes(u16 count)
{
    u16 index;
    u16 sum = 0u;

    for(index = 0u; index < count; ++index)
        sum = (u16)(sum + memory[0x1620u + index]);
    return sum;
}

void RecalculateDriveConfigurationChecksums(void)
{
    u16 checksum;

    ++recalculate_count;
    checksum = SumConfigurationBytes(0x5eu);
    memory[0x12fcu] = (u8)(checksum >> 8);
    memory[0x12fdu] = (u8)checksum;
    memory[0x167eu] = (u8)(checksum >> 8);
    memory[0x167fu] = (u8)checksum;
    checksum = SumConfigurationBytes(0x7eu);
    memory[0x12feu] = (u8)(checksum >> 8);
    memory[0x12ffu] = (u8)checksum;
    memory[0x169eu] = (u8)(checksum >> 8);
    memory[0x169fu] = (u8)checksum;
    if(mutate_after_recalculate != 0u)
    {
        memory[0x1640u] = 0u;
        memory[0x164cu] = 0u;
        memory[0x164du] = 0u;
    }
}

u8 ProgramAndVerifyDriveConfiguration(u8 extended_only)
{
    ++program_count;
    program_extended_only = extended_only;
    return program_result;
}

u16 ReadDriveConfigurationWord(u16 word_index)
{
    if(word_index == 0x2fu)
    {
        u16 value = (u16)(((u16)memory[0x12fcu] << 8) | memory[0x12fdu]);
        return mismatch_primary_word != 0u ? (u16)(value + 1u) : value;
    }
    assert(word_index == 0x3fu);
    {
        u16 value = (u16)(((u16)memory[0x12feu] << 8) | memory[0x12ffu]);
        return mismatch_extended_word != 0u ? (u16)(value + 1u) : value;
    }
}

void PersistDriveConfigurationValidationFailureCode(u8 failure_code)
{
    ++persist_count;
    persisted_code = failure_code;
}

static void StoreU32(u16 address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 LoadU32(u16 address)
{
    u32 value = (u32)memory[address] << 24;

    value |= (u32)memory[address + 1u] << 16;
    value |= (u32)memory[address + 2u] << 8;
    value |= memory[address + 3u];
    return value;
}

static void SetPersistentChecksum(void)
{
    u16 checksum;

    memcpy(&memory[0x1620u], persistent_image, sizeof(persistent_image));
    checksum = SumConfigurationBytes(0x5eu);
    persistent_image[0x5eu] = (u8)(checksum >> 8);
    persistent_image[0x5fu] = (u8)checksum;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(persistent_image, 0, sizeof(persistent_image));
    persistent_image[0x20u] = 1u;
    SetPersistentChecksum();
    interrupt_clear_count = 0u;
    persist_count = 0u;
    persisted_code = 0u;
    recalculate_count = 0u;
    mutate_after_recalculate = 0u;
    program_result = 1u;
    program_count = 0u;
    program_extended_only = 0xffu;
    mismatch_primary_word = 0u;
    mismatch_extended_word = 0u;
    read_offset = 1ul;
    read_destination = 0ul;
    read_count = 0u;
    volatile_source_reads = 0u;
    memory[0x8000u] = 0x11u;
    memory[0x8001u] = 0x22u;
    memory[0x8002u] = 0x33u;
    StoreU32(0x12f8u, 0x01020304ul);
}

static void AssertFailure(u8 code)
{
    assert(memory[0x1009u] == 1u);
    assert(memory[0x12f7u] == 1u);
    assert(persist_count == 1u && persisted_code == code);
    assert(interrupt_clear_count == 1u);
    assert(read_offset == 0ul && read_destination == 0x001620ul);
    assert(read_count == 0xa0u);
}

static void TestSuccessfulPrimaryAndExtendedUpdates(void)
{
    Reset();
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x10u, 3u) == 1u);
    assert(memory[0x1630u] == 0x11u);
    assert(memory[0x1631u] == 0x22u);
    assert(memory[0x1632u] == 0x33u);
    assert(LoadU32(0x12f8u) == 0x01020305ul);
    assert(memory[0x12f7u] == 0u);
    assert(recalculate_count == 1u && program_count == 1u);
    assert(program_extended_only == 0u && persist_count == 0u);

    Reset();
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x61u, 0u) == 1u);
    assert(program_extended_only == 1u);
}

static void TestErasedImageSkipsInitialChecksumValidation(void)
{
    Reset();
    memset(persistent_image, 0xff, sizeof(persistent_image));
    memory[0x8000u] = 1u;
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x20u, 1u) == 1u);
    assert(memory[0x1640u] == 1u);
    assert(recalculate_count == 1u && persist_count == 0u);
}

static void TestInitialValidationFailures(void)
{
    Reset();
    persistent_image[0x5fu] ^= 1u;
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0u, 0u) == 0u);
    AssertFailure(1u);

    Reset();
    persistent_image[0x20u] = 0u;
    persistent_image[0x2cu] = 0u;
    persistent_image[0x2du] = 0u;
    SetPersistentChecksum();
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0u, 0u) == 0u);
    AssertFailure(2u);

    Reset();
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0xa0u, 0u) == 0u);
    AssertFailure(3u);

    Reset();
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0u, 0xa1u) == 0u);
    AssertFailure(5u);
}

static void TestCopyAndInvariantFailures(void)
{
    Reset();
    volatile_source_reads = 1u;
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x10u, 1u) == 0u);
    AssertFailure(6u);

    Reset();
    memory[0x8000u] = 0u;
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x20u, 1u) == 0u);
    AssertFailure(7u);

    Reset();
    mutate_after_recalculate = 1u;
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x10u, 1u) == 0u);
    AssertFailure(8u);
}

static void TestProgrammingAndStoredChecksumFailures(void)
{
    Reset();
    program_result = 0u;
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x10u, 1u) == 0u);
    AssertFailure(9u);

    Reset();
    mismatch_primary_word = 1u;
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x10u, 1u) == 0u);
    AssertFailure(10u);

    Reset();
    mismatch_extended_word = 1u;
    assert(UpdateAndProgramDriveConfigurationRange(0x8000ul, 0x10u, 1u) == 0u);
    AssertFailure(10u);
}

int main(void)
{
    TestSuccessfulPrimaryAndExtendedUpdates();
    TestErasedImageSkipsInitialChecksumValidation();
    TestInitialValidationFailures();
    TestCopyAndInvariantFailures();
    TestProgrammingAndStoredChecksumFailures();
    puts("configuration range update tests passed");
    return 0;
}
