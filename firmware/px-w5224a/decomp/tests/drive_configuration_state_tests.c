#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/drive_configuration_state.h"

typedef struct ConfigurationEvent
{
    u8 event;
    u32 argument0;
} ConfigurationEvent;

static u8 firmware_memory[0x20000];
static u8 sfr_memory[0x100];
static u8 external_selector;
static u16 configuration_word;
static u8 validator_result;
static ConfigurationEvent events[64];
static unsigned event_count;
static unsigned delay_count;
static unsigned p1_bit6_read_count;
static unsigned p1_bit6_raise_after;
static u8 countdown_reads[8];
static unsigned countdown_read_count;
static unsigned countdown_read_index;

static void WriteU32(u16 address, u32 value)
{
    firmware_memory[address] = (u8)(value >> 24);
    firmware_memory[address + 1u] = (u8)(value >> 16);
    firmware_memory[address + 2u] = (u8)(value >> 8);
    firmware_memory[address + 3u] = (u8)value;
}

static u32 ReadU32(u16 address)
{
    return ((u32)firmware_memory[address] << 24) | ((u32)firmware_memory[address + 1u] << 16) | ((u32)firmware_memory[address + 2u] << 8) | firmware_memory[address + 3u];
}

static u16 ReadU16(u16 address)
{
    return (u16)(((u16)firmware_memory[address] << 8) | firmware_memory[address + 1u]);
}

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0xf7c040ul)
        return external_selector;
    if(address == 0x001220ul && countdown_read_index < countdown_read_count)
        return countdown_reads[countdown_read_index++];
    assert(address < sizeof(firmware_memory));
    return firmware_memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware_memory));
    firmware_memory[address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    u8 value = sfr_memory[address];

    if(address == 0x90u && p1_bit6_raise_after != ~0u)
    {
        if(p1_bit6_read_count >= p1_bit6_raise_after)
            value |= 0x40u;
        else
            value &= 0xbfu;
        ++p1_bit6_read_count;
    }
    return value;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    sfr_memory[address] = value;
}

u16 ObserveDriveConfigurationState(u8 event, u32 argument0, u32 argument1)
{
    (void)argument1;
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].event = event;
    events[event_count].argument0 = argument0;
    ++event_count;

    if(event == DRIVE_CONFIGURATION_READ_WORD)
        return configuration_word;
    if(event == DRIVE_CONFIGURATION_VALIDATE_STARTUP_INPUTS)
    {
        firmware_memory[0x1308u] = validator_result;
        return 0u;
    }
    if(event == DRIVE_CONFIGURATION_DELAY)
    {
        assert(argument0 == 0x09a6ul);
        ++delay_count;
    }
    return 0u;
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0xa5, sizeof(firmware_memory));
    memset(sfr_memory, 0, sizeof(sfr_memory));
    memset(events, 0, sizeof(events));
    memset(countdown_reads, 0, sizeof(countdown_reads));
    event_count = 0u;
    delay_count = 0u;
    p1_bit6_read_count = 0u;
    p1_bit6_raise_after = ~0u;
    countdown_read_count = 0u;
    countdown_read_index = 0u;
    external_selector = 0u;
    configuration_word = 0u;
    validator_result = 0u;
    firmware_memory[0x0c86u] = 0x15u;
    firmware_memory[0x11efu] = 0xffu;
    sfr_memory[0xb0u] = 0xffu;
    WriteU32(0x156au, 0x00000200ul);
    WriteU32(0x1562u, 0x00126100ul);
}

static void AssertCommonState(void)
{
    assert(firmware_memory[0x0e44u] == 1u);
    assert(firmware_memory[0x0e45u] == 1u);
    assert(firmware_memory[0x1309u] == 0u);
    assert(firmware_memory[0x0e43u] == 1u);
    assert(ReadU32(0x1598u) == 0x96ul);
    assert(firmware_memory[0x0fbeu] == 2u);
    assert(firmware_memory[0x11efu] == 0xf7u);
    assert(firmware_memory[0x15f6u] == 0u);
    assert(ReadU16(0x1298u) == 0u);
    assert(ReadU16(0x1294u) == 0u);
    assert(ReadU16(0x1296u) == 0u);
    assert(ReadU16(0x1292u) == 0u);
    assert(ReadU16(0x129au) == 0x01ffu);
    assert(ReadU16(0x129cu) == 0x01fau);
    assert(ReadU32(0x129eu) == 0x000088bbul);
    assert(ReadU32(0x1262u) == 0ul);
    assert(ReadU32(0x1266u) == 0ul);
    assert(ReadU32(0x126au) == 0x00126100ul);
    assert(ReadU32(0x126eu) == 0ul);
    assert(events[0].event == DRIVE_CONFIGURATION_READ_WORD && events[0].argument0 == 0x0ful);
    assert(events[1].event == DRIVE_CONFIGURATION_VALIDATE_STARTUP_INPUTS && events[1].argument0 == (u8)configuration_word);
}

static void TestConfigurationZeroSpecialSelectors(void)
{
    ResetFixture();
    external_selector = 1u;
    sfr_memory[0x80u] = 0x02u;
    InitializeDriveConfigurationState();
    assert(firmware_memory[0x1308u] == 1u);
    assert(firmware_memory[0xfbf1u] == 1u);
    assert(firmware_memory[0x130au] == 4u);
    assert(firmware_memory[0x1220u] == 0xfau);
    assert(firmware_memory[0x0c86u] == 0x15u);
    assert(delay_count == 0u);
    AssertCommonState();

    ResetFixture();
    external_selector = 0u;
    sfr_memory[0x80u] = 0x04u;
    InitializeDriveConfigurationState();
    assert(firmware_memory[0x1308u] == 1u);
    assert(firmware_memory[0xfbf1u] == 1u);
    assert(firmware_memory[0x130au] == 4u);
}

static void TestConfigurationZeroRejections(void)
{
    ResetFixture();
    external_selector = 1u;
    sfr_memory[0x80u] = 0x04u;
    InitializeDriveConfigurationState();
    assert(firmware_memory[0x1308u] == 0u);
    assert(firmware_memory[0xfbf1u] == 0u);
    assert(firmware_memory[0x130au] == 0xa5u);

    ResetFixture();
    external_selector = 1u;
    sfr_memory[0x80u] = 0x02u;
    countdown_reads[0] = 1u;
    countdown_read_count = 1u;
    p1_bit6_raise_after = 1u;
    InitializeDriveConfigurationState();
    assert(firmware_memory[0x1308u] == 0u);
    assert(firmware_memory[0xfbf1u] == 0u);
    assert(p1_bit6_read_count == 2u);
}

static void TestExplicitConfigurationModes(void)
{
    ResetFixture();
    configuration_word = 0xab11u;
    validator_result = 0u;
    InitializeDriveConfigurationState();
    assert(firmware_memory[0x1308u] == 1u);
    assert(firmware_memory[0x130au] == 0x0fu);
    assert(delay_count == 36u);
    assert((sfr_memory[0xb0u] & 3u) == 0u);
    AssertCommonState();

    ResetFixture();
    configuration_word = 0xcd12u;
    InitializeDriveConfigurationState();
    assert(firmware_memory[0x1308u] == 1u);
    assert(firmware_memory[0x130au] == 0x1fu);
    assert(delay_count == 36u);
}

static void TestValidatorResultAndGeometryWrap(void)
{
    ResetFixture();
    configuration_word = 5u;
    validator_result = 0u;
    InitializeDriveConfigurationState();
    assert(firmware_memory[0x1308u] == 0u);
    assert(firmware_memory[0x130au] == 0xa5u);

    ResetFixture();
    configuration_word = 5u;
    validator_result = 1u;
    sfr_memory[0x80u] = 0x46u;
    sfr_memory[0x90u] = 0x40u;
    WriteU32(0x156au, 3ul);
    InitializeDriveConfigurationState();
    assert(firmware_memory[0x1308u] == 1u);
    assert(firmware_memory[0x130au] == 0u);
    assert(ReadU16(0x129au) == 2u);
    assert(ReadU16(0x129cu) == 0xfffdu);
    assert(delay_count == 0u);
}

int main(void)
{
    TestConfigurationZeroSpecialSelectors();
    TestConfigurationZeroRejections();
    TestExplicitConfigurationModes();
    TestValidatorResultAndGeometryWrap();
    puts("drive configuration state tests passed");
    return 0;
}
