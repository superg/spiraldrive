#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_subchannel_identification_acquisition.h"

static u8 memory[0x1000000ul];
static fw_addr_t converted_sources[4];
static s32 converted_values[4];
static u8 converted_count;
static u8 load_tracks[8];
static u8 load_count;
static u8 effective_last_track;
static s32 selected_end;
static u8 selected_count;
static u8 dispatch_count;
static u8 execute_count;
static u8 execute_argument;
static u8 clear_bit_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t source)
{
    unsigned index;

    converted_sources[converted_count++] = source;
    for(index = 0u; index < 4u; ++index)
        if(source == 0x00f2b9ul + (fw_addr_t)(index * 6u))
            return converted_values[index];
    return 0l;
}

u8 GetEffectiveLastTrackNumber(void)
{
    return effective_last_track;
}

void LoadTrackInformationByNumber(u8 track_number)
{
    load_tracks[load_count++] = track_number;
    WriteDword(0x00dd3dul, (u32)track_number * 1000ul);
}

s32 SelectWorkingTrackFramePosition(void)
{
    selected_count++;
    return selected_end;
}

u8 DispatchAndWaitForReadAcquisitionResult(void)
{
    dispatch_count++;
    return 1u;
}

void ExecuteReadAcquisitionAndWaitForCompletion(u8 argument)
{
    u8 index;

    execute_count++;
    execute_argument = argument;
    for(index = 0u; index < 10u; ++index)
        memory[0x000f51eul + index] = (u8)(0x80u + index);
}

void ObserveClearReadSubChannelCaptureBit(void)
{
    clear_bit_count++;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(converted_sources, 0, sizeof(converted_sources));
    memset(converted_values, 0, sizeof(converted_values));
    converted_count = 0u;
    memset(load_tracks, 0, sizeof(load_tracks));
    load_count = 0u;
    effective_last_track = 0u;
    selected_end = 4000l;
    selected_count = 0u;
    dispatch_count = 0u;
    execute_count = 0u;
    execute_argument = 0xffu;
    clear_bit_count = 0u;
    memory[0x000fbaul] = 0xffu;
    memory[0x001259ul] = 0xffu;
    memory[0x00f79bul] = 0xffu;
}

static void CheckCachedTrackPath(void)
{
    u8 index;

    Reset();
    memory[0x0012c0ul] = 3u;
    converted_values[2] = 1000l;
    converted_values[3] = 2000l;
    WriteDword(0x0012c1ul, 3000ul);
    AcquireReadSubChannelIdentificationForTrack(2u);
    assert(converted_count == 2u);
    assert(converted_sources[0] == 0x00f2c5ul);
    assert(converted_sources[1] == 0x00f2cbul);
    assert(ReadDword(0x000c8eul) == 1075ul);
    assert(ReadDword(0x000c92ul) == 1495ul);
    assert(dispatch_count == 1u && execute_count == 1u && execute_argument == 0u && clear_bit_count == 1u);
    assert(memory[0x000c8cul] == 2u && memory[0x000c8dul] == 1u);
    assert(memory[0x000e44ul] == 8u);
    assert(memory[0x001259ul] == 0xf9u);
    assert(memory[0x000fbaul] == 0xf9u);
    assert(memory[0x00f79bul] == 0xf0u);
    for(index = 0u; index < 10u; ++index)
        assert(memory[0x000f51eul + index] == (u8)(0x80u + index));
}

static void CheckCachedLeadOutClamp(void)
{
    Reset();
    memory[0x0012c0ul] = 2u;
    converted_values[2] = 1000l;
    WriteDword(0x0012c1ul, 1200ul);
    AcquireReadSubChannelIdentificationForTrack(2u);
    assert(converted_count == 1u);
    assert(ReadDword(0x000c92ul) == 1200ul);
}

static void CheckWorkingTrackPaths(void)
{
    Reset();
    memory[0x001008ul] = 1u;
    effective_last_track = 4u;
    AcquireReadSubChannelIdentificationForTrack(2u);
    assert(load_count == 3u && load_tracks[0] == 2u && load_tracks[1] == 3u && load_tracks[2] == 2u);
    assert(ReadDword(0x000c8eul) == 2075ul);
    assert(ReadDword(0x000c92ul) == 2495ul);
    assert(selected_count == 1u);

    Reset();
    memory[0x001008ul] = 1u;
    effective_last_track = 2u;
    memory[0x00dd34ul] = 5u;
    AcquireReadSubChannelIdentificationForTrack(2u);
    assert(load_count == 1u && selected_count == 0u);
    assert(ReadDword(0x000c92ul) == 2295ul);
}

int main(void)
{
    CheckCachedTrackPath();
    CheckCachedLeadOutClamp();
    CheckWorkingTrackPaths();
    puts("read_subchannel_identification_acquisition_tests: ok");
    return 0;
}
