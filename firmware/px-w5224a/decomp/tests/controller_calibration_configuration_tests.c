#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/controller_calibration_configuration.h"

static u8 memory_bytes[0x14000u];
static u8 profile_byte;
static u8 erased_flag_byte;
static u8 indexed_results[2];
static unsigned indexed_count;
static unsigned read_count;
static fw_addr_t read_offsets[3];
static fw_addr_t read_destinations[3];
static u16 read_lengths[3];
static u8 e8e9_value;
static u8 bit44_value;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0xf7c01ful)
        return profile_byte;
    if(address == 0xf7c04bul)
        return erased_flag_byte;
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

void ObserveCalibrationConfigurationRead(fw_addr_t offset, fw_addr_t destination, u16 count)
{
    assert(read_count < 3u);
    read_offsets[read_count] = offset;
    read_destinations[read_count] = destination;
    read_lengths[read_count++] = count;
}

void ObserveCalibrationConfigurationWriteE8E9(u8 value)
{
    e8e9_value = value;
}

u8 ObserveCalibrationConfigurationIndexedState(u8 state_code)
{
    assert(state_code == 4u && indexed_count < 2u);
    return indexed_results[indexed_count++];
}

void ObserveCalibrationConfigurationWriteBit44(u8 value)
{
    bit44_value = value;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_bytes[address] = (u8)(value >> 8);
    memory_bytes[address + 1u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    profile_byte = 0u;
    erased_flag_byte = 0u;
    indexed_results[0] = indexed_results[1] = 0u;
    indexed_count = read_count = 0u;
    memset(read_offsets, 0, sizeof(read_offsets));
    memset(read_destinations, 0, sizeof(read_destinations));
    memset(read_lengths, 0, sizeof(read_lengths));
    e8e9_value = bit44_value = 0xffu;
    WriteWord(0xfad7u, 0xa5a7u);
    WriteWord(0xfaacu, 0x1234u);
    memory_bytes[0x1304u] = 0xffu;
    memory_bytes[0xc88u] = 0u;
    memory_bytes[0xfab7u] = 0x10u;
    memory_bytes[0xfac2u] = 0x20u;
    memory_bytes[0xfacau] = 0x22u;
    memory_bytes[0xfabcu] = 50u;
    memory_bytes[0xfabdu] = 0x5au;
    memory_bytes[0xfae0u] = 4u;
}

static void AssertCommonResults(void)
{
    assert(read_count == 3u);
    assert(read_offsets[0] == 0x20ul && read_destinations[0] == 0xfab4ul && read_lengths[0] == 0x1au);
    assert(read_offsets[1] == 0x30ul && read_destinations[1] == 0xfac4ul && read_lengths[1] == 4u);
    assert(read_offsets[2] == 0x3ful && read_destinations[2] == 0xfae0ul && read_lengths[2] == 1u);
    assert(e8e9_value == 0u && bit44_value == 1u);
    assert(ReadWord(0xffeu) == 117u);
    assert(memory_bytes[0xfaceu] == 0x5au);
}

static void TestProfileFlagNormalization(void)
{
    ResetFixture();
    profile_byte = 0u;
    erased_flag_byte = 0xffu;
    LoadControllerCalibrationConfigurationState();
    assert(ReadWord(0xfaacu) == 0x1234u);
    assert(ReadWord(0xfad7u) == 0xa5a7u);
    assert(memory_bytes[0x1304u] == 0xdfu);
    assert(indexed_count == 1u);
    AssertCommonResults();

    ResetFixture();
    profile_byte = 0x11u;
    erased_flag_byte = 0xffu;
    LoadControllerCalibrationConfigurationState();
    assert(ReadWord(0xfaacu) == 0x1234u);
    assert(ReadWord(0xfad7u) == 0xa5a5u);
    assert(indexed_count == 1u);

    ResetFixture();
    profile_byte = 1u;
    indexed_results[0] = 0u;
    indexed_results[1] = 1u;
    LoadControllerCalibrationConfigurationState();
    assert(ReadWord(0xfaacu) == 0x1236u);
    assert(ReadWord(0xfad7u) == 0xa5a5u);
    assert(memory_bytes[0xc88u] == 4u && memory_bytes[0x1304u] == 0xffu);
    assert((memory_bytes[0xfae0u] & 0x40u) != 0u && indexed_count == 2u);

    ResetFixture();
    profile_byte = 0x80u;
    LoadControllerCalibrationConfigurationState();
    assert(ReadWord(0xfaacu) == 0x1236u);
    assert(ReadWord(0xfad7u) == 0xa5a7u);
    assert(indexed_count == 1u);
}

static void TestErasedDefaultsAndPortFlags(void)
{
    ResetFixture();
    memory_bytes[0xfab7u] = 0xffu;
    memory_bytes[0xfac2u] = 0xffu;
    memory_bytes[0xfacau] = 0xffu;
    memory_bytes[0xfabcu] = 0xffu;
    memory_bytes[0xfae0u] = 0u;
    LoadControllerCalibrationConfigurationState();
    assert(memory_bytes[0xfab7u] == 0xc0u);
    assert(memory_bytes[0xfac2u] == 0x80u);
    assert(memory_bytes[0xfc14u] == 0u);
    assert(memory_bytes[0xfabcu] == 100u);
    assert(ReadWord(0xffeu) == 234u);
    assert(bit44_value == 0u);

    ResetFixture();
    memory_bytes[0xfacau] = 0x20u;
    LoadControllerCalibrationConfigurationState();
    assert(memory_bytes[0xfc14u] == 0x21u);
}

int main(void)
{
    TestProfileFlagNormalization();
    TestErasedDefaultsAndPortFlags();
    puts("controller_calibration_configuration_tests: ok");
    return 0;
}
