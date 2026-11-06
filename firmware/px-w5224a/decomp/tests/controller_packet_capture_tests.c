#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_packet_capture.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_INDEXED_STATE,
    EVENT_FLAG_CONSUMERS,
    EVENT_MEASUREMENT_CONSUMER
};

struct event_record
{
    enum event_kind kind;
    u8 argument;
};

static u8 memory[1u << 24];
static struct event_record events[8];
static size_t event_count;
static u8 p0_bit7;
static u8 indexed_state_result;
static unsigned media_flags_read_count;
static unsigned lead_out_read_count;
static unsigned decoded_position_read_count;

u8 FirmwareRead8(fw_addr_t address)
{
    address &= 0xfffffful;
    if(address >= 0x000ff5ul && address <= 0x000ff6ul)
        media_flags_read_count++;
    if(address >= 0x001153ul && address <= 0x001156ul)
        lead_out_read_count++;
    if(address >= 0x001235ul && address <= 0x001238ul)
        decoded_position_read_count++;
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 FirmwareReadBit0407(void)
{
    return p0_bit7;
}

static void RecordEvent(enum event_kind kind, u8 argument)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].argument = argument;
    event_count++;
}

u8 ObserveControllerPacketIndexedState(u8 state_code)
{
    RecordEvent(EVENT_INDEXED_STATE, state_code);
    return indexed_state_result;
}

void ObserveControllerPacketFlagConsumers(void)
{
    RecordEvent(EVENT_FLAG_CONSUMERS, 0u);
}

void ObserveControllerPacketMeasurementConsumer(void)
{
    RecordEvent(EVENT_MEASUREMENT_CONSUMER, 0u);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    p0_bit7 = 0u;
    indexed_state_result = 0u;
    media_flags_read_count = 0u;
    lead_out_read_count = 0u;
    decoded_position_read_count = 0u;
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

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void AssertEvent(size_t index, enum event_kind kind, u8 argument)
{
    assert(index < event_count);
    assert(events[index].kind == kind);
    assert(events[index].argument == argument);
}

static void TestP0RejectionAndMeasurementConsumers(void)
{
    ResetState();
    memory[0x000fadu] = 0x10u;
    WriteU16(0x000ff5ul, 0x000cu);

    CaptureControllerThreeBytePacket();

    assert(memory[0x000fadu] == 0x14u);
    assert(memory[0x018003u] == 0x14u);
    assert(event_count == 2u);
    AssertEvent(0u, EVENT_FLAG_CONSUMERS, 0u);
    AssertEvent(1u, EVENT_MEASUREMENT_CONSUMER, 0u);
    assert(media_flags_read_count == 6u);
}

static void TestNegativeControllerStatusStillCaptures(void)
{
    ResetState();
    p0_bit7 = 1u;
    memory[0x01802au] = 0x80u;
    memory[0x01802bu] = 0x12u;
    memory[0x01802cu] = 0x34u;
    memory[0x01802du] = 0x56u;
    memory[0x001136u] = 0xffu;
    memory[0x00fb5cu] = 1u;

    CaptureControllerThreeBytePacket();

    assert(memory[0x001133u] == 0x12u);
    assert(memory[0x001134u] == 0x34u);
    assert(memory[0x001135u] == 0x56u);
    assert(memory[0x001136u] == 0u);
    assert(memory[0x00fb5cu] == 0x81u);
    assert(memory[0x018003u] == 0u);
}

static void TestAllCountersAndInclusiveWindow(void)
{
    ResetState();
    p0_bit7 = 1u;
    memory[0x00fc21u] = 1u;
    WriteU16(0x00fc22ul, 0xffffu);
    memory[0x00fb69u] = 1u;
    WriteU32(0x00fb6aul, 0xfffffffful);
    memory[0x001308u] = 1u;
    memory[0x00fb95u] = 0xfeu;
    indexed_state_result = 1u;
    WriteU32(0x001153ul, 100000ul);
    WriteU32(0x001235ul, 90000ul);
    WriteU16(0x00fb96ul, 0xfffeu);
    memory[0x001301u] = 3u;
    memory[0x01802bu] = 0xa1u;
    memory[0x01802cu] = 0xb2u;
    memory[0x01802du] = 0xc3u;

    CaptureControllerThreeBytePacket();

    assert(ReadU16(0x00fc22ul) == 0u);
    assert(ReadU32(0x00fb6aul) == 0ul);
    assert(memory[0x00fb95u] == 0xffu);
    assert(ReadU16(0x00fb96ul) == 0xffffu);
    assert(ReadU32(0x001235ul) == 90001ul);
    assert(memory[0x001133u] == 0xa1u);
    assert(memory[0x001134u] == 0xb2u);
    assert(memory[0x001135u] == 0xc3u);
    assert(event_count == 1u);
    AssertEvent(0u, EVENT_INDEXED_STATE, 4u);
    assert(lead_out_read_count == 8u);
    assert(decoded_position_read_count == 12u);
}

static void TestDriveStateRejectionStillAdvancesPosition(void)
{
    ResetState();
    p0_bit7 = 1u;
    memory[0x001300u] = 1u;
    memory[0x001301u] = 2u;
    WriteU32(0x001235ul, 0xfffffffful);
    memory[0x000fadu] = 0x20u;
    memory[0x01802bu] = 0xaau;

    CaptureControllerThreeBytePacket();

    assert(ReadU32(0x001235ul) == 0ul);
    assert(memory[0x001133u] == 0u);
    assert(memory[0x000fadu] == 0x24u);
    assert(memory[0x018003u] == 0x24u);
}

static void TestSaturatedCountersAndFlagShortCircuit(void)
{
    ResetState();
    p0_bit7 = 1u;
    memory[0x001308u] = 1u;
    memory[0x00fb95u] = 0xffu;
    indexed_state_result = 1u;
    WriteU32(0x001153ul, 100000ul);
    WriteU32(0x001235ul, 90000ul);
    WriteU16(0x00fb96ul, 0xffffu);
    memory[0x001301u] = 1u;
    WriteU16(0x000ff5ul, 2u);

    CaptureControllerThreeBytePacket();

    assert(memory[0x00fb95u] == 0xffu);
    assert(ReadU16(0x00fb96ul) == 0xffffu);
    assert(event_count == 2u);
    AssertEvent(0u, EVENT_INDEXED_STATE, 4u);
    AssertEvent(1u, EVENT_FLAG_CONSUMERS, 0u);
    assert(media_flags_read_count == 4u);
}

int main(void)
{
    TestP0RejectionAndMeasurementConsumers();
    TestNegativeControllerStatusStillCaptures();
    TestAllCountersAndInclusiveWindow();
    TestDriveStateRejectionStillAdvancesPosition();
    TestSaturatedCountersAndFlagShortCircuit();
    return 0;
}
