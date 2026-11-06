#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_operating_adjustment_response.h"
#include "platform/firmware_memory.h"

typedef struct Event
{
    u8 code;
    u32 argument;
} Event;
static u8 firmware_memory[0x1000000ul];
static Event events[32];
static unsigned event_count;
static u8 bit31_values[4];
static unsigned bit31_index;
static u8 bit33_value;
static u8 mapped_index;
static u32 scaled_result;

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

u32 ObserveControllerOperatingAdjustment(u8 event, u32 argument)
{
    events[event_count].code = event;
    events[event_count++].argument = argument;
    if(event == CONTROLLER_ADJUSTMENT_READ_BIT31)
        return bit31_values[bit31_index++];
    if(event == CONTROLLER_ADJUSTMENT_READ_BIT33)
        return bit33_value;
    if(event == CONTROLLER_ADJUSTMENT_DIVIDE_BY_SEVEN)
        return (u16)((s16)(u16)argument / 7);
    if(event == CONTROLLER_ADJUSTMENT_SCALE_83_POINT_5)
        return scaled_result;
    if(event == CONTROLLER_ADJUSTMENT_MAP_OPERATING_CODE)
        return mapped_index;
    return 0ul;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    firmware_memory[address] = (u8)(value >> 8);
    firmware_memory[address + 1u] = (u8)value;
}
static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)firmware_memory[address] << 8) | firmware_memory[address + 1u]);
}
static unsigned FindEvent(u8 code, u32 argument)
{
    unsigned i;
    for(i = 0u; i < event_count; ++i)
        if(events[i].code == code && events[i].argument == argument)
            return i;
    return event_count;
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0, sizeof(firmware_memory));
    memset(events, 0, sizeof(events));
    memset(bit31_values, 0, sizeof(bit31_values));
    event_count = bit31_index = 0u;
    bit33_value = mapped_index = 0u;
    scaled_result = 0ul;
    firmware_memory[0x1039u] = 0x11u;
    firmware_memory[0xfab5u] = 0x22u;
    WriteU16(0xef83u, 0x1234u);
    WriteU16(0xfbc3u, 0x5678u);
}

static void TestModeOneQualifiedAdjustment(void)
{
    ResetFixture();
    firmware_memory[0x1008u] = 1u;
    firmware_memory[0x0e47u] = 4u;
    firmware_memory[0x0e51u] = 0x14u;
    WriteU16(0x10b4u, 70u);
    WriteU16(0x102au, 9u);
    bit31_values[0] = 1u;
    bit31_values[1] = 1u;
    mapped_index = 3u;
    firmware_memory[0xff9208u] = 0xabu;

    ApplyControllerOperatingAdjustmentAndSelectResponse();

    assert(FindEvent(CONTROLLER_ADJUSTMENT_WRITE_E8E9, 0x22u) < event_count);
    assert(FindEvent(CONTROLLER_ADJUSTMENT_DIVIDE_BY_SEVEN, 140u) < event_count);
    assert(FindEvent(CONTROLLER_ADJUSTMENT_WRITE_ECED, 29u) < event_count);
    assert(FindEvent(CONTROLLER_ADJUSTMENT_WRITE_EAEB, 0x5678u) < event_count);
    assert(firmware_memory[0x18007fu] == 0u && firmware_memory[0x1802eu] == 8u);
    assert(firmware_memory[0x1055u] == 0xabu);
    assert(firmware_memory[0x1039u] == 0x91u);
}

static void TestNonModeScaledAndLowBytePublication(void)
{
    ResetFixture();
    firmware_memory[0x1008u] = 2u;
    firmware_memory[0x0e4fu] = 0x18u;
    firmware_memory[0xf086u] = 5u;
    WriteU16(0x10b4u, 11u);
    bit33_value = 0u;
    bit31_values[0] = 0u;
    mapped_index = 2u;
    scaled_result = 0x12345678ul;
    firmware_memory[0xff91c6u] = 0x5au;

    ApplyControllerOperatingAdjustmentAndSelectResponse();

    assert(FindEvent(CONTROLLER_ADJUSTMENT_WRITE_EAEB, 0x1234u) < event_count);
    assert(FindEvent(CONTROLLER_ADJUSTMENT_SCALE_83_POINT_5, 16u) < event_count);
    assert(ReadU16(0x102au) == 0x5678u);
    assert(FindEvent(CONTROLLER_ADJUSTMENT_WRITE_ECED, 0x5678u) < event_count);
    assert(FindEvent(CONTROLLER_ADJUSTMENT_WRITE_EF00, 0x78u) < event_count);
    assert(firmware_memory[0x1055u] == 0x5au);
}

int main(void)
{
    TestModeOneQualifiedAdjustment();
    TestNonModeScaledAndLowBytePublication();
    puts("controller operating-adjustment response tests passed");
    return 0;
}
