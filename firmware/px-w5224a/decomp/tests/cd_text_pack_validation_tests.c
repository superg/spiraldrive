#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/cd_text_pack_validation.h"

#define MEMORY_SIZE 0x200000u

static u8 memory[MEMORY_SIZE];
static u8 bit_001e;
static unsigned bit_001e_write_count;
static unsigned multiply_count;
static u32 multiply_left;
static u32 multiply_right;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory[address] = value;
}

void FirmwareWriteBit001E(u8 value)
{
    bit_001e = (u8)(value != 0u);
    bit_001e_write_count++;
}

u32 MultiplyU32Low(u32 factor_a, u32 factor_b)
{
    multiply_count++;
    multiply_left = factor_a;
    multiply_right = factor_b;
    return factor_a * factor_b;
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

static void BuildCrcTable(void)
{
    u16 table_index;

    for(table_index = 0u; table_index != 0x100u; ++table_index)
    {
        u16 value = (u16)(table_index << 8);
        u8 bit_index;

        for(bit_index = 0u; bit_index < 8u; ++bit_index)
            if((value & 0x8000u) != 0u)
                value = (u16)((value << 1) ^ 0x1021u);
            else
                value = (u16)(value << 1);
        WriteU16(0xf596u + (u16)(table_index * 2u), value);
    }
}

static fw_addr_t PackAddress(u32 pack_index)
{
    u32 address = 0x00010000ul - 0x8ffcul + pack_index * 18ul;
    u16 high_word = (u16)((u16)(address >> 16) + 0x10u);

    return ((u32)high_word << 16) | (u16)address;
}

static void FinalizePackCrc(fw_addr_t pack_address)
{
    u8 transformed[18];
    u8 index;

    for(index = 0u; index < 16u; ++index)
        transformed[index] = memory[pack_address + index];
    transformed[16] = 0u;
    transformed[17] = 0u;
    for(index = 0u; index < 16u; ++index)
    {
        fw_addr_t table_address = 0xf596u + (u16)transformed[index] * 2u;

        transformed[index + 1u] ^= memory[table_address];
        transformed[index + 2u] ^= memory[table_address + 1u];
    }
    memory[pack_address + 16u] = (u8)(transformed[16] ^ 0xffu);
    memory[pack_address + 17u] = (u8)(transformed[17] ^ 0xffu);
}

static fw_addr_t PreparePack(u32 pack_index, u8 header, u8 type, u8 sequence)
{
    fw_addr_t address = PackAddress(pack_index);

    memset(&memory[address], 0, 18u);
    memory[address] = header;
    memory[address + 1u] = type;
    memory[address + 2u] = sequence;
    return address;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    bit_001e = 1u;
    bit_001e_write_count = 0u;
    multiply_count = 0u;
    multiply_left = 0u;
    multiply_right = 0u;
    WriteU32(0x1556u, 0x00010000ul);
    BuildCrcTable();
}

static void test_type_one_payload_lengths_and_addressing(void)
{
    fw_addr_t address;

    ResetFixture();
    address = PreparePack(3u, 0x8fu, 1u, 0u);
    memory[address + 12u] = 0u;
    memory[address + 13u] = 1u;
    memory[address + 14u] = 2u;
    memory[address + 15u] = 0xffu;
    FinalizePackCrc(address);
    WriteU16(0xf796u, 0xffffu);
    WriteU16(0xf798u, 0xabcdu);

    assert(ValidateCdTextPackAndAccumulatePayloadLengths(3u) == 1u);
    assert(multiply_count == 1u && multiply_left == 3u && multiply_right == 18u);
    assert(memory[0x15f1u] == 1u);
    assert(ReadU16(0xf796u) == 0x0105u);
    assert(ReadU16(0xf798u) == 0xabcdu);
    assert(bit_001e_write_count == 0u);
}

static void test_type_two_payload_lengths(void)
{
    fw_addr_t address;

    ResetFixture();
    address = PreparePack(0u, 0x8fu, 2u, 0u);
    memory[address + 4u] = 1u;
    memory[address + 5u] = 0u;
    memory[address + 6u] = 0xffu;
    memory[address + 7u] = 2u;
    FinalizePackCrc(address);
    WriteU16(0xf796u, 0x1234u);
    WriteU16(0xf798u, 0xffffu);

    assert(ValidateCdTextPackAndAccumulatePayloadLengths(0u) == 1u);
    assert(ReadU16(0xf796u) == 0x1234u);
    assert(ReadU16(0xf798u) == 0x0105u);
}

static void test_non_exact_header_can_validate_without_accumulating(void)
{
    fw_addr_t address;

    ResetFixture();
    address = PreparePack(0u, 0x80u, 1u, 0u);
    memory[address + 12u] = 5u;
    FinalizePackCrc(address);
    WriteU16(0xf796u, 0x4567u);

    assert(ValidateCdTextPackAndAccumulatePayloadLengths(0u) == 1u);
    assert(ReadU16(0xf796u) == 0x4567u);
    assert(memory[0x15f1u] == 1u);
}

static void test_header_rejection_does_not_advance_sequence(void)
{
    fw_addr_t address;

    ResetFixture();
    memory[0x15f1u] = 7u;
    address = PreparePack(0u, 0x70u, 1u, 7u);
    FinalizePackCrc(address);

    assert(ValidateCdTextPackAndAccumulatePayloadLengths(0u) == 0u);
    assert(memory[0x15f1u] == 7u && bit_001e_write_count == 0u);
}

static void test_nonzero_sequence_discontinuity_clears_partial_bit(void)
{
    fw_addr_t address;

    ResetFixture();
    memory[0x15f1u] = 5u;
    address = PreparePack(0u, 0x8fu, 1u, 4u);
    FinalizePackCrc(address);

    assert(ValidateCdTextPackAndAccumulatePayloadLengths(0u) == 0u);
    assert(memory[0x15f1u] == 5u);
    assert(bit_001e == 0u && bit_001e_write_count == 1u);
}

static void test_zero_sequence_restarts_continuity(void)
{
    fw_addr_t address;

    ResetFixture();
    memory[0x15f1u] = 5u;
    address = PreparePack(0u, 0x8fu, 3u, 0u);
    FinalizePackCrc(address);

    assert(ValidateCdTextPackAndAccumulatePayloadLengths(0u) == 1u);
    assert(memory[0x15f1u] == 1u && bit_001e_write_count == 0u);
}

static void test_crc_failure_occurs_after_sequence_advance(void)
{
    fw_addr_t address;

    ResetFixture();
    address = PreparePack(0u, 0x8fu, 3u, 0u);
    FinalizePackCrc(address);
    memory[address + 17u] ^= 1u;

    assert(ValidateCdTextPackAndAccumulatePayloadLengths(0u) == 0u);
    assert(memory[0x15f1u] == 1u);
    assert(bit_001e_write_count == 0u);
}

int main(void)
{
    test_type_one_payload_lengths_and_addressing();
    test_type_two_payload_lengths();
    test_non_exact_header_can_validate_without_accumulating();
    test_header_rejection_does_not_advance_sequence();
    test_nonzero_sequence_discontinuity_clears_partial_bit();
    test_zero_sequence_restarts_continuity();
    test_crc_failure_occurs_after_sequence_advance();
    puts("CD-TEXT pack validation tests passed");
    return 0;
}
