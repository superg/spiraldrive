#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_formatted_toc_response.h"

static u8 memory[0x1000000ul];
static u8 unsupported;
static u8 no_sense;
static u8 combined_result;
static u8 execute_result;
static fw_addr_t combined_source;
static u32 combined_count;
static u32 execute_count;
static u32 capacity_value;
static s32 converted_frame;
static fw_addr_t converted_tuple_source;
static unsigned copy_count;
static u32 copy_offsets[8];
static u32 copy_lengths[8];

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

void HandleUnsupportedScsiCommand(void)
{
    unsupported = 1u;
}
void SetNoSenseData(void)
{
    no_sense = 1u;
}

u16 MinU16(u16 value, u16 limit)
{
    return value < limit ? value : limit;
}

u16 ReadBigEndianU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source, u32 byte_count)
{
    combined_source = source;
    combined_count = byte_count;
    return combined_result;
}

u32 CopyToControllerBufferWindow(fw_addr_t source, u32 offset, u32 byte_count)
{
    u32 index;

    assert(copy_count < 8u);
    copy_offsets[copy_count] = offset;
    copy_lengths[copy_count] = byte_count;
    copy_count++;
    for(index = 0ul; index < byte_count; ++index)
        memory[0x100000ul + offset + index] = memory[source + index];
    return offset + byte_count;
}

u8 ExecuteControllerDataInAndMapStatus(u32 transfer_length)
{
    execute_count = transfer_length;
    return execute_result;
}

u32 ComputeAndPublishMediaCapacityGeometry(void)
{
    return capacity_value;
}

void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t destination)
{
    converted_frame = frame_position;
    memory[destination] = 0x12u;
    memory[destination + 1u] = 0x34u;
    memory[destination + 2u] = 0x56u;
}

s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t source)
{
    converted_tuple_source = source;
    return (s32)(0x1000ul + memory[source]);
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
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

static fw_addr_t TrackRecord(u8 track)
{
    return 0x00f2b6ul + (fw_addr_t)((u16)track * 6u);
}

static void FillTrack(u8 track, u8 control, u8 minute, u8 second, u8 frame)
{
    fw_addr_t record = TrackRecord(track);

    memory[record] = track;
    memory[record + 1u] = control;
    memory[record + 3u] = minute;
    memory[record + 4u] = second;
    memory[record + 5u] = frame;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    unsupported = 0u;
    no_sense = 0u;
    combined_result = 4u;
    execute_result = 4u;
    combined_source = 0u;
    combined_count = 0ul;
    execute_count = 0ul;
    capacity_value = 0x11223344ul;
    converted_frame = -1l;
    converted_tuple_source = 0u;
    copy_count = 0u;
    memset(copy_offsets, 0, sizeof(copy_offsets));
    memset(copy_lengths, 0, sizeof(copy_lengths));
    WriteWord(0x000b11ul, 0xffffu);
    WriteDword(0x00155aul, 0x200ul);
    WriteDword(0x002ba58dul, 0x12345678ul);
}

static void CheckActiveOrdinaryMatrix(void)
{
    u8 high_media;
    u8 msf;

    for(high_media = 0u; high_media <= 1u; ++high_media)
    {
        for(msf = 0u; msf <= 1u; ++msf)
        {
            Reset();
            memory[0x000c9eul] = 1u;
            memory[0x000c88ul] = high_media != 0u ? 0x80u : 0u;
            memory[0x000b0bul] = msf != 0u ? 2u : 0u;
            ExecuteReadFormattedTocResponse();
            assert(ReadBigEndianU16(0x000b2aul) == 18u);
            assert(ReadBigEndianU16(0x000b2cul) == 0x0101u);
            assert(ReadDword(0x000b2eul) == 0x00170100ul);
            assert(ReadDword(0x000b32ul) == (msf == 0u ? 0ul : (high_media != 0u ? 0x00000200ul : 0x0000163cul)));
            assert(ReadDword(0x000b36ul) == 0x0016aa00ul);
            assert(ReadDword(0x000b3aul) == (high_media != 0u ? (msf != 0u ? 0x0000122bul : 0x00000400ul) : (msf != 0u ? 0x00123456ul : capacity_value)));
            assert(combined_source == 0x000b2aul && combined_count == 20ul);
        }
    }
}

static void CheckActiveLeadOutAndFailure(void)
{
    Reset();
    memory[0x000c9eul] = 1u;
    memory[0x000b10ul] = 0xaau;
    memory[0x000b0bul] = 2u;
    WriteWord(0x000b11ul, 7u);
    combined_result = 3u;
    ExecuteReadFormattedTocResponse();
    assert(ReadBigEndianU16(0x000b2aul) == 10u);
    assert(ReadDword(0x000b2eul) == 0x0016aa00ul);
    assert(ReadDword(0x000b32ul) == 0x00123456ul);
    assert(converted_frame == (s32)0x12345678ul);
    assert(combined_count == 7ul && no_sense == 1u);
}

static void CheckCachedValidationAndHeaderOnly(void)
{
    Reset();
    ExecuteReadFormattedTocResponse();
    assert(unsupported == 1u && copy_count == 0u);

    Reset();
    memory[0x0012c9ul] = 1u;
    memory[0x0012bful] = 1u;
    memory[0x0012c0ul] = 3u;
    memory[0x000b10ul] = 4u;
    ExecuteReadFormattedTocResponse();
    assert(unsupported == 1u && copy_count == 0u);

    Reset();
    memory[0x0012c9ul] = 1u;
    memory[0x0012bful] = 1u;
    memory[0x0012c0ul] = 1u;
    WriteWord(0x000b11ul, 3u);
    ExecuteReadFormattedTocResponse();
    assert(copy_count == 1u && copy_lengths[0] == 3ul);
    assert(execute_count == 3ul);
    assert(memory[0x100200ul] == 0u && memory[0x100201ul] == 18u);
}

static void CheckCachedMsfAndLba(void)
{
    Reset();
    memory[0x0012c9ul] = 1u;
    memory[0x0012bful] = 1u;
    memory[0x0012c0ul] = 2u;
    memory[0x000b0bul] = 2u;
    FillTrack(1u, 0x41u, 1u, 2u, 3u);
    FillTrack(2u, 0x42u, 4u, 5u, 6u);
    FillTrack(3u, 0x14u, 7u, 8u, 9u);
    ExecuteReadFormattedTocResponse();
    assert(copy_count == 2u && copy_lengths[0] == 4ul && copy_lengths[1] == 24ul);
    assert(memory[0x100204ul] == 0u && memory[0x100205ul] == 0x14u);
    assert(memory[0x100206ul] == 1u && memory[0x100207ul] == 0u);
    assert(memory[0x100208ul] == 0u && memory[0x100209ul] == 1u && memory[0x10020aul] == 2u && memory[0x10020bul] == 3u);
    assert(memory[0x100215ul] == 0x41u && memory[0x100216ul] == 3u);
    assert(execute_count == 28ul);

    Reset();
    memory[0x0012c9ul] = 1u;
    memory[0x0012bful] = 1u;
    memory[0x0012c0ul] = 1u;
    FillTrack(1u, 0x41u, 5u, 0u, 0u);
    FillTrack(2u, 0x14u, 9u, 0u, 0u);
    ExecuteReadFormattedTocResponse();
    assert(converted_tuple_source == TrackRecord(2u) + 3u);
    assert(ReadDword(0x100210ul) == 0x00000f73ul);
}

static void CheckCachedSpecialStartAndPartial(void)
{
    Reset();
    memory[0x0012c9ul] = 1u;
    memory[0x0012bful] = 1u;
    memory[0x0012c0ul] = 3u;
    memory[0x0012beul] = 8u;
    FillTrack(2u, 0x22u, 2u, 3u, 4u);
    FillTrack(3u, 0x33u, 3u, 4u, 5u);
    FillTrack(4u, 0x44u, 4u, 5u, 6u);
    WriteWord(0x000b11ul, 9u);
    ExecuteReadFormattedTocResponse();
    assert(ReadBigEndianU16(0x100200ul) == 26u);
    assert(copy_count == 2u && copy_lengths[0] == 4ul && copy_lengths[1] == 5ul);
    assert(memory[0x100205ul] == 0x22u && memory[0x100206ul] == 2u);
}

static void CheckCachedOneHundredSixtyByteFlush(void)
{
    u8 track;

    Reset();
    memory[0x0012c9ul] = 1u;
    memory[0x0012bful] = 1u;
    memory[0x0012c0ul] = 20u;
    memory[0x000b0bul] = 2u;
    for(track = 1u; track <= 21u; ++track)
        FillTrack(track, (u8)(0x40u | track), track, 0u, 0u);
    ExecuteReadFormattedTocResponse();
    assert(copy_count == 3u);
    assert(copy_lengths[0] == 4ul && copy_lengths[1] == 160ul && copy_lengths[2] == 8ul);
    assert(copy_offsets[0] == 0x200ul && copy_offsets[1] == 0x204ul && copy_offsets[2] == 0x2a4ul);
    assert(execute_count == 172ul);
}

int main(void)
{
    CheckActiveOrdinaryMatrix();
    CheckActiveLeadOutAndFailure();
    CheckCachedValidationAndHeaderOnly();
    CheckCachedMsfAndLba();
    CheckCachedSpecialStartAndPartial();
    CheckCachedOneHundredSixtyByteFlush();
    puts("read_formatted_toc_response_tests: ok");
    return 0;
}
