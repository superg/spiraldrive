#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/drive_firmware_initialization.h"

typedef struct InitializationEvent
{
    u8 event;
    u32 argument0;
    u32 argument1;
    u32 argument2;
} InitializationEvent;

static u8 firmware_memory[0x20000];
static u8 sfr_memory[0x100];
static u8 bit0032;
static InitializationEvent events[32];
static unsigned event_count;
static u8 probe20_result;
static u8 probe300_result;
static u8 boot_memory_result;
static u8 primary_configuration_result;
static u8 erased_configuration_result;
static u8 clear_bit0032_during_probe_clear;

u8 FirmwareRead8(fw_addr_t address)
{
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
    return sfr_memory[address];
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    sfr_memory[address] = value;
}

u8 FirmwareReadBit0032(void)
{
    return bit0032;
}

void FirmwareWriteBit0032(u8 value)
{
    bit0032 = (u8)(value != 0u);
}

u8 ObserveDriveFirmwareInitialization(u8 event, u32 argument0, u32 argument1, u32 argument2)
{
    InitializationEvent *record;

    assert(event_count < sizeof(events) / sizeof(events[0]));
    record = &events[event_count++];
    record->event = event;
    record->argument0 = argument0;
    record->argument1 = argument1;
    record->argument2 = argument2;

    switch(event)
    {
    case DRIVE_FIRMWARE_RUN_MEMORY_PROBE_20_36:
        return probe20_result;
    case DRIVE_FIRMWARE_RUN_MEMORY_PROBE_300_540:
        return probe300_result;
    case DRIVE_FIRMWARE_VALIDATE_BOOT_MEMORY:
        return boot_memory_result;
    case DRIVE_FIRMWARE_VALIDATE_PRIMARY_CONFIGURATION:
        return primary_configuration_result;
    case DRIVE_FIRMWARE_TRY_ERASED_CONFIGURATION:
        return erased_configuration_result;
    case DRIVE_FIRMWARE_ZERO_MEMORY:
        if(argument0 == 0x20ul && clear_bit0032_during_probe_clear != 0u)
            bit0032 = 0u;
        break;
    default:
        break;
    }
    return 0u;
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0xa5, sizeof(firmware_memory));
    memset(sfr_memory, 0, sizeof(sfr_memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    bit0032 = 0u;
    probe20_result = 0u;
    probe300_result = 1u;
    boot_memory_result = 1u;
    primary_configuration_result = 1u;
    erased_configuration_result = 0u;
    clear_bit0032_during_probe_clear = 1u;
    firmware_memory[0x00fau] = 0xffu;
    firmware_memory[0xfaefu] = 0x03u;
    sfr_memory[0x88u] = 0xefu;
    sfr_memory[0x91u] = 0xffu;
    sfr_memory[0xa8u] = 0x20u;
    sfr_memory[0xa9u] = 0x10u;
    sfr_memory[0xb0u] = 0xffu;
}

static unsigned CountEvent(u8 event)
{
    unsigned index;
    unsigned count = 0u;

    for(index = 0u; index < event_count; ++index)
        if(events[index].event == event)
            ++count;
    return count;
}

static InitializationEvent *FindEvent(u8 event, unsigned occurrence)
{
    unsigned index;

    for(index = 0u; index < event_count; ++index)
    {
        if(events[index].event == event)
        {
            if(occurrence == 0u)
                return &events[index];
            --occurrence;
        }
    }
    assert(0);
    return 0;
}

static void AssertCommonInitializationState(void)
{
    InitializationEvent *zero0 = FindEvent(DRIVE_FIRMWARE_ZERO_MEMORY, 0u);
    InitializationEvent *zero1 = FindEvent(DRIVE_FIRMWARE_ZERO_MEMORY, 1u);
    InitializationEvent *zero2 = FindEvent(DRIVE_FIRMWARE_ZERO_MEMORY, 2u);
    InitializationEvent *fill = FindEvent(DRIVE_FIRMWARE_FILL_CONTROLLER_BUFFER, 0u);

    assert(firmware_memory[0x0fcau] == 0u);
    assert(firmware_memory[0x18002u] == 0u);
    assert(firmware_memory[0x0ca0u] == 0u);
    assert(firmware_memory[0x00fau] == 0u);
    assert(firmware_memory[0xfaefu] == 0x83u);
    assert(firmware_memory[0x12f7u] == 0u);
    assert(firmware_memory[0x12f8u] == 0u && firmware_memory[0x12f9u] == 0u && firmware_memory[0x12fau] == 0u && firmware_memory[0x12fbu] == 0u);
    assert(firmware_memory[0x130eu] == 0u && firmware_memory[0x130fu] == 0xffu && firmware_memory[0x1310u] == 0x50u && firmware_memory[0x1311u] == 0u);
    assert(sfr_memory[0x85u] == 0u && sfr_memory[0x86u] == 0x1bu);
    assert((sfr_memory[0xb0u] & 3u) == 1u);
    assert(zero0->argument0 == 0x20ul && zero0->argument1 == 0x60ul);
    assert(zero1->argument0 == 0x0b00ul && zero1->argument1 == 0x0d00ul);
    assert(zero2->argument0 == 0x8000ul && zero2->argument1 == 0x8000ul);
    assert(fill->argument0 == 0x1b0000ul && fill->argument1 == 0x10000ul && fill->argument2 == 0u);
    assert(FindEvent(DRIVE_FIRMWARE_RUN_MEMORY_PROBE_20_36, 0u)->argument0 == 0x20ul);
    assert(FindEvent(DRIVE_FIRMWARE_RUN_MEMORY_PROBE_20_36, 0u)->argument1 == 0x36ul);
    assert(FindEvent(DRIVE_FIRMWARE_RUN_MEMORY_PROBE_300_540, 0u)->argument0 == 0x300ul);
    assert(FindEvent(DRIVE_FIRMWARE_RUN_MEMORY_PROBE_300_540, 0u)->argument1 == 0x540ul);
}

static void TestAcceptedInitialization(void)
{
    InitializationEvent *tail_fill;

    ResetFixture();
    InitializeDriveFirmware();

    AssertCommonInitializationState();
    assert(firmware_memory[0x1009u] == 0u);
    assert(CountEvent(DRIVE_FIRMWARE_TRY_ERASED_CONFIGURATION) == 0u);
    assert(CountEvent(DRIVE_FIRMWARE_INITIALIZE_RUNTIME_STATE) == 1u);
    assert(CountEvent(DRIVE_FIRMWARE_FILL_CONTROLLER_BUFFER) == 2u);
    tail_fill = FindEvent(DRIVE_FIRMWARE_FILL_CONTROLLER_BUFFER, 1u);
    assert(tail_fill->argument0 == 0ul && tail_fill->argument1 == 0x1b0000ul && tail_fill->argument2 == 0u);
    assert((sfr_memory[0xa8u] & 0xc1u) == 0xc1u);
    assert((sfr_memory[0xa9u] & 4u) != 0u);
    assert(sfr_memory[0x88u] == 0xf5u && sfr_memory[0x91u] == 0xf5u);
}

static void TestErasedConfigurationFallback(void)
{
    ResetFixture();
    primary_configuration_result = 0u;
    erased_configuration_result = 0x7fu;
    InitializeDriveFirmware();

    assert(firmware_memory[0x1009u] == 0u);
    assert(CountEvent(DRIVE_FIRMWARE_TRY_ERASED_CONFIGURATION) == 1u);
    assert(CountEvent(DRIVE_FIRMWARE_INITIALIZE_RUNTIME_STATE) == 1u);
}

static void TestStartupProbeFailures(void)
{
    ResetFixture();
    probe20_result = 0xffu;
    probe300_result = 0u;
    InitializeDriveFirmware();
    assert(firmware_memory[0x1009u] == 1u);
    assert(CountEvent(DRIVE_FIRMWARE_VALIDATE_PRIMARY_CONFIGURATION) == 0u);
    assert(CountEvent(DRIVE_FIRMWARE_INITIALIZE_RUNTIME_STATE) == 0u);

    ResetFixture();
    probe20_result = 0xffu;
    clear_bit0032_during_probe_clear = 0u;
    InitializeDriveFirmware();
    assert(firmware_memory[0x1009u] == 1u);
    assert(bit0032 == 1u);
}

static void TestBootAndConfigurationFailures(void)
{
    ResetFixture();
    firmware_memory[0x00fau] = 0x22u;
    boot_memory_result = 0u;
    InitializeDriveFirmware();
    assert(firmware_memory[0x1009u] == 2u);
    assert(firmware_memory[0x00fau] == 0x22u);
    assert((sfr_memory[0xb0u] & 3u) == 0u);
    assert(CountEvent(DRIVE_FIRMWARE_VALIDATE_PRIMARY_CONFIGURATION) == 0u);

    ResetFixture();
    primary_configuration_result = 0u;
    erased_configuration_result = 0u;
    InitializeDriveFirmware();
    assert(firmware_memory[0x1009u] == 10u);
    assert(CountEvent(DRIVE_FIRMWARE_TRY_ERASED_CONFIGURATION) == 1u);
    assert(CountEvent(DRIVE_FIRMWARE_INITIALIZE_RUNTIME_STATE) == 0u);
    assert((sfr_memory[0xa8u] & 0x81u) == 0u);
}

int main(void)
{
    TestAcceptedInitialization();
    TestErasedConfigurationFallback();
    TestStartupProbeFailures();
    TestBootAndConfigurationFailures();
    puts("drive firmware initialization tests passed");
    return 0;
}
