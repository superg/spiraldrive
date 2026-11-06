#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_track_record_advance.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x300000];
static s32 distances[4];
static u8 distance_index;
static u8 decoded_q_type;
static u16 configure_calls;
static u16 restore_isrc_calls;
static u16 restore_mcn_calls;
static u16 set_msf_calls;
static u16 select_isrc_calls;
static u16 boundary_calls;
static u8 restore_isrc_argument;
static u8 restore_mcn_argument;
static s32 msf_relative;
static s32 msf_absolute;
static u16 boundary_index;

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory_image[address] << 8) | memory_image[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static void SetRecord(u16 index, u8 control, u8 track, u8 point)
{
    fw_addr_t address = 0xb4bdu + (u16)(index << 3);
    memory_image[address] = control;
    memory_image[address + 1u] = track;
    memory_image[address + 2u] = point;
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(distances, 0, sizeof(distances));
    distance_index = 0u;
    decoded_q_type = 2u;
    configure_calls = 0u;
    restore_isrc_calls = 0u;
    restore_mcn_calls = 0u;
    set_msf_calls = 0u;
    select_isrc_calls = 0u;
    boundary_calls = 0u;
    restore_isrc_argument = 0xffu;
    restore_mcn_argument = 0xffu;
    msf_relative = 0l;
    msf_absolute = 0l;
    boundary_index = 0u;
    WriteU32(0x1598u, 123456ul);
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

void ObserveTrackAdvanceConfigureLayout(void)
{
    ++configure_calls;
}
s32 ObserveTrackAdvanceDistance(u16 record_index)
{
    (void)record_index;
    return distances[distance_index++];
}
u8 ObserveTrackAdvanceBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}
void ObserveTrackAdvanceRestoreIsrc(u8 value)
{
    ++restore_isrc_calls;
    restore_isrc_argument = value;
}
void ObserveTrackAdvanceRestoreMcn(u8 value)
{
    ++restore_mcn_calls;
    restore_mcn_argument = value;
}
void ObserveTrackAdvanceSetMsf(s32 relative, s32 absolute)
{
    ++set_msf_calls;
    msf_relative = relative;
    msf_absolute = absolute;
}
u8 ObserveTrackAdvanceDecode(u16 record_index)
{
    (void)record_index;
    return decoded_q_type;
}
void ObserveTrackAdvanceSelectIsrc(void)
{
    ++select_isrc_calls;
}
s32 ObserveTrackAdvanceBoundary(u16 record_index)
{
    ++boundary_calls;
    boundary_index = record_index;
    return 2000l;
}

static void TestSpecialLeadInScan(void)
{
    ResetHarness();
    memory_image[0x88b5u] = 1u;
    memory_image[0xc44u] = 2u;
    memory_image[0x14a5u] = 4u;
    WriteU16(0x149bu, 2u);
    SetRecord(2u, 1u, 3u, 1u);
    SetRecord(3u, 2u, 4u, 1u);
    SetRecord(4u, 1u, 4u, 1u);
    AdvanceRawReadTrackRecordAndUpdateCurrentQPacket();
    assert(configure_calls == 1u);
    assert(ReadU16(0x149du) == 2u && ReadU16(0x149bu) == 4u);
    assert(distance_index == 0u);
}

static void TestZeroDistanceIndexZeroAndRestoration(void)
{
    u8 i;

    ResetHarness();
    distances[0] = 0l;
    distances[1] = 301l;
    WriteU16(0x149bu, 1u);
    SetRecord(2u, 2u, 8u, 2u);
    SetRecord(3u, 0x41u, 8u, 0u);
    SetRecord(4u, 1u, 9u, 1u);
    memory_image[0x14a8u] = 7u;
    memory_image[0x1405u] = 1u;
    memory_image[0x1403u] = 1u;
    for(i = 0u; i < 6u; ++i)
        memory_image[0x1275u + i] = (u8)(0xa0u + i);
    decoded_q_type = 5u;
    AdvanceRawReadTrackRecordAndUpdateCurrentQPacket();
    assert(distance_index == 2u && ReadU16(0x149bu) == 4u);
    assert(memory_image[0x1272u] == 0x41u && memory_image[0x1273u] == 0x08u && memory_image[0x1274u] == 0u && memory_image[0x1278u] == 0u);
    assert(memory_image[0x14abu] == 1u);
    for(i = 0u; i < 10u; ++i)
        assert(memory_image[0x88aau + i] == memory_image[0x1272u + i]);
    assert(restore_isrc_calls == 1u && restore_isrc_argument == 1u);
    assert(restore_mcn_calls == 1u && restore_mcn_argument == 1u);
    assert(set_msf_calls == 1u && msf_relative == 300l && msf_absolute == 123456l);
    assert(memory_image[0x127cu] == 1u && memory_image[0x14a7u] == 1u);
    assert(memory_image[0x14a8u] == 8u && memory_image[0x1285u] == 5u);
    assert(select_isrc_calls == 1u);
    assert(boundary_calls == 1u && boundary_index == 4u);
    assert(memory_image[0x14cfu] == 1u && ReadU32(0x14d1u) == 1850ul);
}

static void TestIndexOneAndLaterIndex(void)
{
    ResetHarness();
    distances[0] = 50l;
    SetRecord(0u, 1u, 3u, 1u);
    SetRecord(1u, 1u, 4u, 1u);
    decoded_q_type = 2u;
    AdvanceRawReadTrackRecordAndUpdateCurrentQPacket();
    assert(set_msf_calls == 1u && msf_relative == 0l);
    assert(memory_image[0x14d0u] == 1u && boundary_calls == 0u);

    ResetHarness();
    distances[0] = 50l;
    SetRecord(0u, 1u, 3u, 2u);
    SetRecord(1u, 1u, 4u, 1u);
    AdvanceRawReadTrackRecordAndUpdateCurrentQPacket();
    assert(set_msf_calls == 0u && memory_image[0x14a9u] == 1u);
    assert(select_isrc_calls == 0u);
}

int main(void)
{
    TestSpecialLeadInScan();
    TestZeroDistanceIndexZeroAndRestoration();
    TestIndexOneAndLaterIndex();
    puts("raw-read track-record advance tests passed");
    return 0;
}
