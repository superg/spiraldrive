#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/fixed_packet_overhead.h"
#include "platform/firmware_memory.h"

typedef struct Conversion
{
    fw_addr_t address;
    s32 result;
} Conversion;

static u8 firmware_memory[0x1000000ul];
static Conversion conversions[16];
static unsigned conversion_count;
static unsigned conversion_index;
static u8 state_valid;
static u8 last_track;
static unsigned last_track_calls;

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

u8 ObserveFixedPacketStateValid(void)
{
    return state_valid;
}

u8 ObserveFixedPacketLastTrackNumber(void)
{
    ++last_track_calls;
    return last_track;
}

s32 ObserveFixedPacketMsfConversion(fw_addr_t tuple_address)
{
    assert(conversion_index < conversion_count);
    assert(conversions[conversion_index].address == tuple_address);
    return conversions[conversion_index++].result;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    firmware_memory[address] = (u8)(value >> 24);
    firmware_memory[address + 1u] = (u8)(value >> 16);
    firmware_memory[address + 2u] = (u8)(value >> 8);
    firmware_memory[address + 3u] = (u8)value;
}

static fw_addr_t TrackAddress(u8 track, fw_addr_t field)
{
    return field + (u16)track * 0x13u;
}

static void AddConversion(fw_addr_t address, s32 result)
{
    assert(conversion_count < sizeof(conversions) / sizeof(conversions[0]));
    conversions[conversion_count].address = address;
    conversions[conversion_count].result = result;
    ++conversion_count;
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0, sizeof(firmware_memory));
    memset(conversions, 0, sizeof(conversions));
    conversion_count = 0u;
    conversion_index = 0u;
    state_valid = 1u;
    last_track = 1u;
    last_track_calls = 0u;
}

static void ConfigureFixedTrack(u8 track, u32 packet_size, s32 start)
{
    firmware_memory[TrackAddress(track, 0xdd89ul)] = 1u;
    WriteU32(TrackAddress(track, 0xdd8aul), packet_size);
    AddConversion(TrackAddress(track, 0xdd81ul), start);
}

static void TestInvalidStateReturnsFfWithoutReads(void)
{
    ResetFixture();
    state_valid = 0u;

    assert(GetFixedPacketOverheadFramesRemaining(100l) == 0xffu);
    assert(last_track_calls == 0u);
    assert(conversion_index == 0u);
}

static void TestInitialTrackSelectionAndScan(void)
{
    ResetFixture();
    firmware_memory[0x12c9u] = 1u;
    firmware_memory[0x12bfu] = 3u;
    last_track = 4u;
    AddConversion(TrackAddress(3u, 0xdd94ul), 200l);
    AddConversion(TrackAddress(4u, 0xdd94ul), 500l);
    ConfigureFixedTrack(4u, 10ul, 250l);

    assert(GetFixedPacketOverheadFramesRemaining(260l) == 7u);
    assert(last_track_calls == 1u);
    assert(conversion_index == conversion_count);
}

static void TestDefaultAndLeadInTrackSelection(void)
{
    ResetFixture();
    AddConversion(TrackAddress(1u, 0xdd94ul), 1000l);
    ConfigureFixedTrack(1u, 8ul, 0l);
    assert(GetFixedPacketOverheadFramesRemaining(3l) == 0u);
    assert(conversion_index == conversion_count);

    ResetFixture();
    firmware_memory[0x1149u] = 1u;
    firmware_memory[0x1147u] = 7u;
    last_track = 7u;
    AddConversion(TrackAddress(7u, 0xdd94ul), 1000l);
    ConfigureFixedTrack(7u, 8ul, 0l);
    assert(GetFixedPacketOverheadFramesRemaining(8l) == 7u);
    assert(conversion_index == conversion_count);
}

static void TestZeroEndUsesFallbackPosition(void)
{
    ResetFixture();
    AddConversion(TrackAddress(1u, 0xdd94ul), 0l);
    WriteU32(0x1153ul, 300ul);
    ConfigureFixedTrack(1u, 20ul, 100l);

    assert(GetFixedPacketOverheadFramesRemaining(120l) == 7u);
    assert(conversion_index == conversion_count);
}

static void TestInvalidTrackAndPacketSize(void)
{
    ResetFixture();
    AddConversion(TrackAddress(1u, 0xdd94ul), 1000l);
    assert(GetFixedPacketOverheadFramesRemaining(100l) == 0xffu);
    assert(conversion_index == 1u);

    ResetFixture();
    AddConversion(TrackAddress(1u, 0xdd94ul), 1000l);
    firmware_memory[TrackAddress(1u, 0xdd89ul)] = 1u;
    assert(GetFixedPacketOverheadFramesRemaining(100l) == 0xffu);
    assert(conversion_index == 1u);
}

static void TestBeforeStartAndOverheadCountdown(void)
{
    u32 overhead;

    ResetFixture();
    AddConversion(TrackAddress(1u, 0xdd94ul), 1000l);
    ConfigureFixedTrack(1u, 10ul, 200l);
    assert(GetFixedPacketOverheadFramesRemaining(199l) == 0u);

    for(overhead = 0ul; overhead < 7ul; ++overhead)
    {
        ResetFixture();
        AddConversion(TrackAddress(1u, 0xdd94ul), 1000l);
        ConfigureFixedTrack(1u, 10ul, 200l);
        assert(GetFixedPacketOverheadFramesRemaining((s32)(210ul + overhead)) == (u8)(7ul - overhead));
        assert(conversion_index == conversion_count);
    }

    ResetFixture();
    AddConversion(TrackAddress(1u, 0xdd94ul), 1000l);
    ConfigureFixedTrack(1u, 10ul, 200l);
    assert(GetFixedPacketOverheadFramesRemaining(217l) == 0u);
}

int main(void)
{
    TestInvalidStateReturnsFfWithoutReads();
    TestInitialTrackSelectionAndScan();
    TestDefaultAndLeadInTrackSelection();
    TestZeroEndUsesFallbackPosition();
    TestInvalidTrackAndPacketSize();
    TestBeforeStartAndOverheadCountdown();
    puts("fixed packet overhead tests passed");
    return 0;
}
