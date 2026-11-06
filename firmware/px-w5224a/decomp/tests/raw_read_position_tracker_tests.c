#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_position_tracker.h"
#include "platform/firmware_memory.h"

typedef struct EventRecord
{
    u8 event;
    u32 argument0;
    u32 argument1;
} EventRecord;

static u8 firmware_memory[0x1000000ul];
static EventRecord events[32];
static unsigned event_count;
static u8 special_mode;
static u8 sequence_valid;
static s32 converted_msf;
static u8 port26_values[10];
static unsigned port26_read_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(firmware_memory));
    if(address == 0x018026ul)
    {
        assert(port26_read_count < 10u);
        return port26_values[port26_read_count++];
    }
    return firmware_memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware_memory));
    firmware_memory[address] = value;
}

u8 FirmwareReadBit0013(void)
{
    return special_mode;
}

u32 ObserveRawReadPositionTracker(u8 event, u32 argument0, u32 argument1)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].event = event;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    ++event_count;

    if(event == RAW_READ_TRACKER_READ_SEQUENCE_VALID)
        return sequence_valid;
    if(event == RAW_READ_TRACKER_SET_SEQUENCE_VALID)
    {
        sequence_valid = 1u;
        return 0ul;
    }
    if(event == RAW_READ_TRACKER_CONVERT_BCD)
        return ((argument0 >> 4) * 10u) + (argument0 & 0x0fu);
    if(event == RAW_READ_TRACKER_CONVERT_MSF)
        return (u32)converted_msf;
    return 0ul;
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

static unsigned CountEvent(u8 event)
{
    unsigned index;
    unsigned count = 0u;

    for(index = 0u; index < event_count; ++index)
        if(events[index].event == event)
            ++count;
    return count;
}

static const EventRecord *FindEvent(u8 event, unsigned occurrence)
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
    return 0;
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0, sizeof(firmware_memory));
    memset(events, 0, sizeof(events));
    memset(port26_values, 0, sizeof(port26_values));
    event_count = 0u;
    special_mode = 0u;
    sequence_valid = 0u;
    converted_msf = 0;
    port26_read_count = 0u;
    WriteU32(0x1598u, 100ul);
}

static void TestSpecialModeArmsImmediately(void)
{
    ResetFixture();
    special_mode = 1u;

    TrackRawReadPositionAndArmTargetCapture();

    assert(CountEvent(RAW_READ_TRACKER_ARM_CAPTURE) == 1u);
    assert(port26_read_count == 0u);
}

static void TestRetryCountdownDefersAndThenServices(void)
{
    ResetFixture();
    firmware_memory[0x0f23u] = 16u;
    firmware_memory[0x0f1eu] = 1u;

    TrackRawReadPositionAndArmTargetCapture();
    assert(firmware_memory[0x0f1eu] == 0u);
    assert(CountEvent(RAW_READ_TRACKER_SERVICE_COUNTDOWN) == 0u);

    TrackRawReadPositionAndArmTargetCapture();
    assert(firmware_memory[0x0f1eu] == 0xffu);
    assert(CountEvent(RAW_READ_TRACKER_SERVICE_COUNTDOWN) == 1u);
}

static void TestDecodedPositionSeedArmsAtModeTwoTarget(void)
{
    ResetFixture();
    firmware_memory[0x0f23u] = 15u;
    firmware_memory[0x0fafu] = 2u;
    WriteU32(0x1235u, 99ul);

    TrackRawReadPositionAndArmTargetCapture();

    assert(sequence_valid == 1u);
    assert(ReadU32(0x15aeu) == 100ul);
    assert(CountEvent(RAW_READ_TRACKER_ARM_CAPTURE) == 1u);
}

static void TestAdrOnePacketUsesTenFreshReadsAndFinalMsf(void)
{
    const EventRecord *enqueue;
    static const u8 samples[10] = { 0x41u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66u, 0x77u, 0x01u, 0x12u, 0x34u };

    ResetFixture();
    memcpy(port26_values, samples, sizeof(samples));
    firmware_memory[0x18022u] = 1u;
    firmware_memory[0x0fafu] = 3u;
    converted_msf = 0;

    TrackRawReadPositionAndArmTargetCapture();

    assert(port26_read_count == 10u);
    assert(memcmp(&firmware_memory[0x0fa3u], samples, sizeof(samples)) == 0);
    assert(sequence_valid == 1u);
    assert(firmware_memory[0x15a0u] == 1u);
    assert(firmware_memory[0x15a1u] == 12u);
    assert(firmware_memory[0x15a2u] == 34u);
    assert(FindEvent(RAW_READ_TRACKER_CONVERT_MSF, 0u)->argument0 == 0x15a0u);
    assert(firmware_memory[0x0f1eu] == 30u);
    enqueue = FindEvent(RAW_READ_TRACKER_ENQUEUE_TIMING_STATUS, 0u);
    assert(enqueue != 0 && enqueue->argument0 == 0x0200u && enqueue->argument1 == 3u);
}

static void TestInvalidPacketAdvancesKnownSequence(void)
{
    ResetFixture();
    sequence_valid = 1u;
    firmware_memory[0x18022u] = 1u;
    firmware_memory[0x0fafu] = 2u;
    WriteU32(0x15aeu, 99ul);

    TrackRawReadPositionAndArmTargetCapture();

    assert(port26_read_count == 10u);
    assert(ReadU32(0x15aeu) == 100ul);
    assert(firmware_memory[0x0f1eu] == 30u);
    assert(CountEvent(RAW_READ_TRACKER_ARM_CAPTURE) == 1u);
}

static void TestWindowRejectsAboveAndAtLowerBoundary(void)
{
    ResetFixture();
    sequence_valid = 1u;
    WriteU32(0x15aeu, 100ul);
    WriteU32(0x1598u, 100ul);

    TrackRawReadPositionAndArmTargetCapture();
    assert(ReadU32(0x15aeu) == 101ul);
    assert(CountEvent(RAW_READ_TRACKER_SERVICE_COUNTDOWN) == 1u);

    ResetFixture();
    sequence_valid = 1u;
    WriteU32(0x15aeu, (u32)-51L);
    WriteU32(0x1598u, 100ul);
    TrackRawReadPositionAndArmTargetCapture();
    assert(ReadU32(0x15aeu) == (u32)-50L);
    assert(CountEvent(RAW_READ_TRACKER_SERVICE_COUNTDOWN) == 1u);
}

static void TestFormatOffsetMinusOneExactCapture(void)
{
    ResetFixture();
    sequence_valid = 1u;
    firmware_memory[0x0f06u] = 1u;
    firmware_memory[0x0fafu] = 2u;
    WriteU32(0x15aeu, 98ul);

    TrackRawReadPositionAndArmTargetCapture();

    assert(ReadU32(0x15aeu) == 99ul);
    assert(CountEvent(RAW_READ_TRACKER_ARM_CAPTURE) == 1u);
}

int main(void)
{
    TestSpecialModeArmsImmediately();
    TestRetryCountdownDefersAndThenServices();
    TestDecodedPositionSeedArmsAtModeTwoTarget();
    TestAdrOnePacketUsesTenFreshReadsAndFinalMsf();
    TestInvalidPacketAdvancesKnownSequence();
    TestWindowRejectsAboveAndAtLowerBoundary();
    TestFormatOffsetMinusOneExactCapture();
    puts("raw-read position tracker tests passed");
    return 0;
}
