#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/read_format_capacities.h"

#define MEMORY_SIZE 0x2c0000ul

static u8 memory[MEMORY_SIZE];
static u8 validate_result;
static u8 transfer_result;
static u8 unsupported_calls;
static u8 no_sense_calls;
static u8 load_calls;
static u8 loaded_tracks[8];
static u8 load_track_types[8];
static u8 next_writable_calls;
static s32 next_writable_result;
static u8 convert_calls;
static u8 converted_tuples[4][3];
static s32 convert_result;
static u8 geometry_calls;
static u32 geometry_results[3];
static fw_addr_t transfer_source;
static u32 transfer_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory[address] = value;
}

u8 ObserveValidateReadFormatCapacitiesCdb(fw_addr_t mask_descriptor)
{
    assert(mask_descriptor == 0xff8d0bul);
    return validate_result;
}

void ObserveUnsupportedReadFormatCapacitiesCommand(void)
{
    unsupported_calls++;
}

void ObserveLoadReadFormatCapacitiesTrack(u8 track_number)
{
    loaded_tracks[load_calls] = track_number;
    memory[0x00dd34ul] = load_track_types[load_calls];
    load_calls++;
}

s32 ObserveComputeReadFormatCapacitiesNextWritablePosition(void)
{
    next_writable_calls++;
    return next_writable_result;
}

s32 ObserveConvertReadFormatCapacitiesMsf(u8 minute, u8 second, u8 frame)
{
    converted_tuples[convert_calls][0] = minute;
    converted_tuples[convert_calls][1] = second;
    converted_tuples[convert_calls][2] = frame;
    convert_calls++;
    return convert_result;
}

u32 ObserveMultiplyReadFormatCapacities(u32 factor_a, u32 factor_b)
{
    return factor_a * factor_b;
}

u32 ObserveDivideReadFormatCapacities(u32 dividend, u32 divisor)
{
    assert(divisor != 0ul);
    return dividend / divisor;
}

u32 ObserveComputeReadFormatCapacitiesGeometry(void)
{
    return geometry_results[geometry_calls++];
}

u8 ObserveTransferReadFormatCapacitiesResponse(fw_addr_t source_address, u32 byte_count)
{
    transfer_source = source_address;
    transfer_count = byte_count;
    return transfer_result;
}

void ObserveSetNoSenseForReadFormatCapacities(void)
{
    no_sense_calls++;
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

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    validate_result = 1u;
    transfer_result = 4u;
    unsupported_calls = 0u;
    no_sense_calls = 0u;
    load_calls = 0u;
    memset(loaded_tracks, 0, sizeof(loaded_tracks));
    memset(load_track_types, 0, sizeof(load_track_types));
    next_writable_calls = 0u;
    next_writable_result = 0;
    convert_calls = 0u;
    memset(converted_tuples, 0, sizeof(converted_tuples));
    convert_result = 0;
    geometry_calls = 0u;
    memset(geometry_results, 0, sizeof(geometry_results));
    transfer_source = 0ul;
    transfer_count = 0ul;
    WriteWord(0x000b11ul, 0xffffu);
    memory[0x00dd35ul] = 9u;
}

static void AssertDescriptor(u8 offset, u32 blocks, u8 type, u32 length)
{
    fw_addr_t address = 0x000b2aul + offset;

    assert(ReadDword(address) == blocks);
    assert(memory[address + 4u] == type);
    assert(memory[address + 5u] == (u8)(length >> 16));
    assert(memory[address + 6u] == (u8)(length >> 8));
    assert(memory[address + 7u] == (u8)length);
}

static void TestInvalidCdb(void)
{
    ResetState();
    validate_result = 0u;
    HandleReadFormatCapacitiesCommand();
    assert(unsupported_calls == 1u);
    assert(transfer_count == 0ul);
}

static void TestNonzeroMediaModeResponse(void)
{
    ResetState();
    memory[0x000c9eul] = 1u;
    WriteDword(0x000c96ul, 0xa1234567ul);
    geometry_results[0] = 0x01020304ul;
    geometry_results[1] = 0x05060708ul;
    HandleReadFormatCapacitiesCommand();
    assert(ReadDword(0x000b2aul) == 16ul);
    AssertDescriptor(4u, 0x01020304ul, 0x92u, 0x00234567ul);
    AssertDescriptor(12u, 0x05060708ul, 0x90u, 0ul);
    assert(geometry_calls == 2u);
    assert(load_calls == 0u);
    assert(transfer_source == 0x000b2aul);
    assert(transfer_count == 20ul);
}

static void TestUnavailableMediaResponse(void)
{
    ResetState();
    memory[0x000faeul] = 1u;
    HandleReadFormatCapacitiesCommand();
    assert(ReadDword(0x000b2aul) == 8ul);
    AssertDescriptor(4u, 0ul, 3u, 0x800ul);
    assert(load_calls == 1u && loaded_tracks[0] == 9u);
    assert(transfer_count == 12ul);
}

static void TestTrackCapacityRoutes(void)
{
    ResetState();
    memory[0x001149ul] = 5u;
    memory[0x001148ul] = 4u;
    load_track_types[0] = 2u;
    next_writable_result = 0x12345;
    WriteDword(0x001153ul, 3000ul);
    HandleReadFormatCapacitiesCommand();
    assert(load_calls == 2u);
    assert(loaded_tracks[0] == 5u && loaded_tracks[1] == 9u);
    assert(next_writable_calls == 1u);
    AssertDescriptor(4u, 0x12345ul, 3u, 0x800ul);

    ResetState();
    memory[0x001149ul] = 5u;
    memory[0x001148ul] = 4u;
    load_track_types[0] = 0u;
    load_track_types[1] = 7u;
    next_writable_result = 1000;
    WriteDword(0x001153ul, 3000ul);
    HandleReadFormatCapacitiesCommand();
    assert(load_calls == 3u);
    assert(loaded_tracks[0] == 5u && loaded_tracks[1] == 4u && loaded_tracks[2] == 9u);
    AssertDescriptor(4u, 848ul, 3u, 0x800ul);

    ResetState();
    memory[0x001149ul] = 5u;
    memory[0x001148ul] = 4u;
    load_track_types[0] = 9u;
    memory[0x2bb03bul + 53u] = 1u;
    memory[0x2bb03bul + 54u] = 2u;
    memory[0x2bb03bul + 55u] = 3u;
    convert_result = 10000;
    WriteDword(0x001153ul, 3000ul);
    HandleReadFormatCapacitiesCommand();
    assert(convert_calls == 1u);
    assert(converted_tuples[0][0] == 1u && converted_tuples[0][1] == 2u && converted_tuples[0][2] == 3u);
    AssertDescriptor(4u, 9848ul, 3u, 0x800ul);
}

static void TestFullTocLeadOut(void)
{
    fw_addr_t first = 0x2b8003ul + 11u;
    fw_addr_t second_base = 0x2b8001ul + 22u;

    ResetState();
    memory[0x0012c9ul] = 1u;
    WriteWord(0x0012caul, 22u);
    memory[first] = 0xa2u;
    memory[0x2b8003ul + 22u] = 0xa2u;
    memory[second_base + 7u] = 11u;
    memory[second_base + 8u] = 22u;
    memory[second_base + 9u] = 33u;
    convert_result = 20000;
    HandleReadFormatCapacitiesCommand();
    assert(convert_calls == 1u);
    assert(converted_tuples[0][0] == 11u && converted_tuples[0][1] == 22u && converted_tuples[0][2] == 33u);
    AssertDescriptor(4u, 19850ul, 2u, 0x800ul);
    assert(transfer_count == 12ul);
}

static void TestWritableDescriptorsAndClamp(void)
{
    u32 packet_size = 32ul;
    u32 source_capacity = 10000ul + 7ul - 150ul;
    u32 formatted = (source_capacity * packet_size) / (packet_size + 7ul);

    formatted = (formatted / packet_size) * packet_size;
    ResetState();
    memory[0x001149ul] = 0u;
    WriteDword(0x001153ul, 10000ul);
    geometry_results[0] = 0x55667788ul;
    WriteWord(0x000b11ul, 17u);
    transfer_result = 3u;
    HandleReadFormatCapacitiesCommand();
    assert(ReadDword(0x000b2aul) == 48ul);
    AssertDescriptor(4u, 0ul, 3u, 0x800ul);
    AssertDescriptor(12u, formatted, 0u, 0x800ul);
    AssertDescriptor(20u, formatted, 0x40u, packet_size);
    AssertDescriptor(28u, formatted, 0x44u, packet_size);
    AssertDescriptor(36u, formatted, 0x48u, packet_size);
    AssertDescriptor(44u, 0x55667788ul, 0x90u, 0ul);
    assert(transfer_count == 17ul);
    assert(no_sense_calls == 1u);
}

static void TestConfiguredPacketSizeAndZero(void)
{
    ResetState();
    memory[0x001149ul] = 2u;
    memory[0x001148ul] = 1u;
    load_track_types[0] = 1u;
    next_writable_result = 100u;
    memory[0x2bb03bul + 20u] = 0x50u;
    WriteDword(0x000c4bul, 16ul);
    WriteDword(0x001153ul, 1000ul);
    HandleReadFormatCapacitiesCommand();
    AssertDescriptor(20u, 592ul, 0x40u, 16ul);
    AssertDescriptor(28u, 492ul, 0x44u, 16ul);

    ResetState();
    memory[0x001149ul] = 2u;
    memory[0x001148ul] = 1u;
    load_track_types[0] = 1u;
    memory[0x001132ul] = 0x10u;
    WriteDword(0x000c4bul, 0ul);
    HandleReadFormatCapacitiesCommand();
    AssertDescriptor(12u, 0ul, 0u, 0x800ul);
    AssertDescriptor(20u, 0ul, 0x40u, 0ul);
}

int main(void)
{
    TestInvalidCdb();
    TestNonzeroMediaModeResponse();
    TestUnavailableMediaResponse();
    TestTrackCapacityRoutes();
    TestFullTocLeadOut();
    TestWritableDescriptorsAndClamp();
    TestConfiguredPacketSizeAndZero();
    puts("read format capacities tests passed");
    return 0;
}
