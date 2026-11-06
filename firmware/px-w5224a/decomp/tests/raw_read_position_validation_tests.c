#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_position_validation.h"
#include "platform/firmware_memory.h"

typedef struct EventRecord
{
    u8 event;
    u32 argument0;
    u32 argument1;
} EventRecord;

static u8 firmware_memory[0x1000000ul];
static EventRecord events[32];
static u32 scripted_results[RAW_READ_POSITION_SET_TERMINAL + 1u][8];
static u8 scripted_counts[RAW_READ_POSITION_SET_TERMINAL + 1u];
static u8 scripted_indices[RAW_READ_POSITION_SET_TERMINAL + 1u];
static unsigned event_count;

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

u32 ObserveRawReadPositionValidation(u8 event, u32 argument0, u32 argument1)
{
    u8 index;

    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].event = event;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    ++event_count;
    index = scripted_indices[event]++;
    if(index < scripted_counts[event])
        return scripted_results[event][index];
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

static void WriteU32(fw_addr_t address, u32 value)
{
    firmware_memory[address] = (u8)(value >> 24);
    firmware_memory[address + 1u] = (u8)(value >> 16);
    firmware_memory[address + 2u] = (u8)(value >> 8);
    firmware_memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)firmware_memory[address] << 24) | ((u32)firmware_memory[address + 1u] << 16) | ((u32)firmware_memory[address + 2u] << 8) | firmware_memory[address + 3u];
}

static void ScriptResult(u8 event, u32 value)
{
    u8 count = scripted_counts[event];

    assert(count < 8u);
    scripted_results[event][count] = value;
    scripted_counts[event] = (u8)(count + 1u);
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

static const EventRecord *FindEvent(u8 event)
{
    unsigned index;

    for(index = 0u; index < event_count; ++index)
        if(events[index].event == event)
            return &events[index];
    return 0;
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0, sizeof(firmware_memory));
    memset(events, 0, sizeof(events));
    memset(scripted_results, 0, sizeof(scripted_results));
    memset(scripted_counts, 0, sizeof(scripted_counts));
    memset(scripted_indices, 0, sizeof(scripted_indices));
    event_count = 0u;
    WriteU32(0x1598u, 100ul);
}

static void TestDelayedPositionExpiryEmitsStatusNine(void)
{
    const EventRecord *enqueue;

    ResetFixture();
    firmware_memory[0x0f1fu] = 1u;
    firmware_memory[0x0f24u] = 8u;
    firmware_memory[0x0c3du] = 10u;
    ScriptResult(RAW_READ_POSITION_CHECK_P1, 0u);

    ServiceRawReadPositionValidationAndDescriptorEnqueue();

    enqueue = FindEvent(RAW_READ_POSITION_ENQUEUE_TIMING_STATUS);
    assert(enqueue != 0 && enqueue->argument0 == 0x800bu && enqueue->argument1 == 9u);
    assert(firmware_memory[0x0f24u] == 7u);
    assert(ReadU32(0x0f41u) == 100ul);
    assert(CountEvent(RAW_READ_POSITION_BUILD_DESCRIPTOR) == 0u);
}

static void TestDelayedPositionAboveThresholdReinitializes(void)
{
    ResetFixture();
    firmware_memory[0x0f1fu] = 1u;
    firmware_memory[0x0f24u] = 9u;
    firmware_memory[0x0c3du] = 10u;
    ScriptResult(RAW_READ_POSITION_CHECK_P1, 0u);

    ServiceRawReadPositionValidationAndDescriptorEnqueue();

    assert(firmware_memory[0x0f24u] == 8u);
    assert(CountEvent(RAW_READ_POSITION_INITIALIZE_CURSORS) == 1u);
    assert(CountEvent(RAW_READ_POSITION_ENQUEUE_TIMING_STATUS) == 0u);
}

static void TestDelayedStablePositionAdvancesModeTwoCursor(void)
{
    ResetFixture();
    firmware_memory[0x0f1fu] = 2u;
    firmware_memory[0x0f07u] = 2u;
    WriteU32(0x0f36u, 200ul);
    WriteU16(0x1298u, 3u);
    WriteU16(0x129au, 7u);
    ScriptResult(RAW_READ_POSITION_CHECK_P1, 1u);

    ServiceRawReadPositionValidationAndDescriptorEnqueue();

    assert(firmware_memory[0x0f1eu] == 30u);
    assert(firmware_memory[0x0f1fu] == 1u);
    assert(ReadU16(0x1298u) == 4u);
    assert(CountEvent(RAW_READ_POSITION_LIMIT_BACKLOG) == 1u);
    assert(CountEvent(RAW_READ_POSITION_BUILD_DESCRIPTOR) == 1u);
    assert(CountEvent(RAW_READ_POSITION_ENQUEUE_DESCRIPTOR) == 1u);
    assert(ReadU32(0x1598u) == 101ul);
}

static void TestP1GateWithoutDelayEmitsStatusNine(void)
{
    ResetFixture();
    firmware_memory[0x0f2fu] = 0x40u;
    ScriptResult(RAW_READ_POSITION_CHECK_P1, 0u);

    ServiceRawReadPositionValidationAndDescriptorEnqueue();

    assert(FindEvent(RAW_READ_POSITION_ENQUEUE_TIMING_STATUS)->argument0 == 0x800bu);
    assert(CountEvent(RAW_READ_POSITION_DECODE) == 0u);
}

static void TestDecodeFailureExpiryEmitsStatusFour(void)
{
    const EventRecord *enqueue;

    ResetFixture();
    firmware_memory[0x0f1eu] = 0u;
    ScriptResult(RAW_READ_POSITION_DECODE, 0u);

    ServiceRawReadPositionValidationAndDescriptorEnqueue();

    enqueue = FindEvent(RAW_READ_POSITION_ENQUEUE_TIMING_STATUS);
    assert(enqueue != 0 && enqueue->argument0 == 0x82u && enqueue->argument1 == 4u);
    assert(firmware_memory[0x0f1eu] == 0xffu);
    assert(ReadU32(0x0f41u) == 100ul);
}

static void TestAcceptedDecodedPositionAndModeOneHighWater(void)
{
    ResetFixture();
    firmware_memory[0x0f07u] = 1u;
    firmware_memory[0x0f1eu] = 3u;
    WriteU32(0x0f6eu, 80ul);
    WriteU16(0x1292u, 31u);
    ScriptResult(RAW_READ_POSITION_DECODE, 1u);

    ServiceRawReadPositionValidationAndDescriptorEnqueue();

    assert(firmware_memory[0x0f1eu] == 30u);
    assert(ReadU32(0x1598u) == 81ul);
    assert(CountEvent(RAW_READ_POSITION_BUILD_DESCRIPTOR) == 1u);
    assert(CountEvent(RAW_READ_POSITION_ENQUEUE_DESCRIPTOR) == 1u);
    assert(CountEvent(RAW_READ_POSITION_INITIALIZE_CURSORS) == 1u);
    assert(CountEvent(RAW_READ_POSITION_SET_TERMINAL) == 1u);
}

static void TestModeTwoLimitEmitsTerminalDescriptor(void)
{
    const EventRecord *enqueue;

    ResetFixture();
    firmware_memory[0x0f07u] = 2u;
    firmware_memory[0x0f1eu] = 2u;
    WriteU32(0x0f36u, 100ul);
    ScriptResult(RAW_READ_POSITION_DECODE, 0u);

    ServiceRawReadPositionValidationAndDescriptorEnqueue();

    enqueue = FindEvent(RAW_READ_POSITION_ENQUEUE_TIMING_STATUS);
    assert(enqueue != 0 && enqueue->argument0 == 0u && enqueue->argument1 == 0u);
    assert(ReadU32(0x0f41u) == 100ul);
    assert(CountEvent(RAW_READ_POSITION_BUILD_DESCRIPTOR) == 0u);
}

int main(void)
{
    TestDelayedPositionExpiryEmitsStatusNine();
    TestDelayedPositionAboveThresholdReinitializes();
    TestDelayedStablePositionAdvancesModeTwoCursor();
    TestP1GateWithoutDelayEmitsStatusNine();
    TestDecodeFailureExpiryEmitsStatusFour();
    TestAcceptedDecodedPositionAndModeOneHighWater();
    TestModeTwoLimitEmitsTerminalDescriptor();
    puts("raw-read position validation tests passed");
    return 0;
}
