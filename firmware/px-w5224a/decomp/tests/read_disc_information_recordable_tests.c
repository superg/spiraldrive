#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_disc_information_recordable.h"

static u8 low_memory[1u << 16];
static u8 far_2b_memory[1u << 16];
static u8 tail_uninitialized;
static unsigned tail_call_count;
static u8 first_track_last_session;
static unsigned first_track_call_count;
static u8 disc_type;
static unsigned disc_type_call_count;
static unsigned multiply_count;
static u32 multiply_left[2];
static u32 multiply_right[2];
static unsigned convert_count;
static s32 convert_position[2];
static fw_addr_t convert_destination[2];
static unsigned copy_count;
static fw_addr_t copy_source;
static fw_addr_t copy_destination;
static u16 copy_length;

u8 FirmwareRead8(fw_addr_t address)
{
    if((address & 0xff0000ul) == 0x2b0000ul)
        return far_2b_memory[(u16)address];
    return low_memory[(u16)address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    if((address & 0xff0000ul) == 0x2b0000ul)
        far_2b_memory[(u16)address] = value;
    else
        low_memory[(u16)address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    (void)address;
    return 0u;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    (void)address;
    (void)value;
}

u8 ObserveRecordableDiscTailUninitialized(void)
{
    tail_call_count++;
    return tail_uninitialized;
}

u8 ObserveRecordableDiscFirstTrackForLastSession(void)
{
    first_track_call_count++;
    return first_track_last_session;
}

u8 ObserveRecordableDiscType(void)
{
    disc_type_call_count++;
    return disc_type;
}

u32 ObserveRecordableDiscMultiply(u32 factor_a, u32 factor_b)
{
    multiply_left[multiply_count] = factor_a;
    multiply_right[multiply_count] = factor_b;
    multiply_count++;
    return factor_a * factor_b;
}

void ObserveRecordableDiscConvertMsf(s32 frame_position, fw_addr_t destination)
{
    convert_position[convert_count] = frame_position;
    convert_destination[convert_count] = destination;
    convert_count++;
}

void ObserveRecordableDiscCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    copy_count++;
    copy_source = source;
    copy_destination = destination;
    copy_length = byte_count;
}

static void WriteDword(u16 address, u32 value)
{
    low_memory[address] = (u8)(value >> 24);
    low_memory[(u16)(address + 1u)] = (u8)(value >> 16);
    low_memory[(u16)(address + 2u)] = (u8)(value >> 8);
    low_memory[(u16)(address + 3u)] = (u8)value;
}

static u32 ReadDword(u16 address)
{
    return ((u32)low_memory[address] << 24) | ((u32)low_memory[(u16)(address + 1u)] << 16) | ((u32)low_memory[(u16)(address + 2u)] << 8) | low_memory[(u16)(address + 3u)];
}

static void ResetFixture(void)
{
    memset(low_memory, 0, sizeof(low_memory));
    memset(far_2b_memory, 0, sizeof(far_2b_memory));
    tail_uninitialized = 1u;
    tail_call_count = 0u;
    first_track_last_session = 4u;
    first_track_call_count = 0u;
    disc_type = 0x20u;
    disc_type_call_count = 0u;
    multiply_count = 0u;
    memset(multiply_left, 0, sizeof(multiply_left));
    memset(multiply_right, 0, sizeof(multiply_right));
    convert_count = 0u;
    memset(convert_position, 0, sizeof(convert_position));
    memset(convert_destination, 0, sizeof(convert_destination));
    copy_count = 0u;
    copy_source = 0ul;
    copy_destination = 0ul;
    copy_length = 0u;
    low_memory[0xdd7eu] = 2u;
    low_memory[0xdd7fu] = 5u;
    low_memory[0xe4ecu] = 6u;
    low_memory[0x12bfu] = 1u;
    low_memory[0x12c0u] = 9u;
    WriteDword(0xdd69u, 12345ul);
    WriteDword(0x1153u, 54321ul);
}

static void AssertCommonTail(u8 opc_count)
{
    assert(low_memory[0x0b2au] == 0u);
    assert(low_memory[0x0b2bu] == (u8)(opc_count * 8u + 0x20u));
    assert(low_memory[0x0b32u] == disc_type);
    assert(disc_type_call_count == 1u);
    assert(low_memory[0x0b4bu] == opc_count);
    assert(copy_count == 1u && copy_source == 0x00e72bul);
    assert(copy_destination == 0x000b4cul);
    assert(copy_length == (u16)(opc_count * 8u));
}

static void TestOpenNormalResponse(void)
{
    ResetFixture();
    low_memory[0xe74bu] = 3u;
    low_memory[0x1008u] = 1u;

    assert(BuildRecordableMediaReadDiscInformationResponse() == 1u);
    assert(low_memory[0x0b2cu] == 0u);
    assert(low_memory[0x0b2du] == 1u && low_memory[0x0b2eu] == 1u);
    assert(low_memory[0x0b2fu] == 2u && low_memory[0x0b30u] == 5u);
    assert(tail_call_count == 2u && first_track_call_count == 0u);
    assert(convert_count == 2u);
    assert(convert_position[0] == 12345l && convert_destination[0] == 0x000b3bul);
    assert(convert_position[1] == 54321l && convert_destination[1] == 0x000b3ful);
    assert(low_memory[0x0b42u] == 0u);
    AssertCommonTail(3u);
}

static void TestSpecialFixedPacketTrackSelection(void)
{
    ResetFixture();
    low_memory[0x0c46u] = 1u;
    low_memory[0x0c44u] = 1u;

    assert(BuildRecordableMediaReadDiscInformationResponse() == 1u);
    assert(low_memory[0x0b2cu] == 5u);
    assert(low_memory[0x0b30u] == 6u);
    assert(tail_call_count == 2u);

    ResetFixture();
    low_memory[0x0c46u] = 1u;
    low_memory[0x0c44u] = 1u;
    low_memory[0xe4ecu] = 100u;
    assert(BuildRecordableMediaReadDiscInformationResponse() == 1u);
    assert(low_memory[0x0b30u] == 99u);
}

static void TestClosedSessionUsesSentinels(void)
{
    ResetFixture();
    low_memory[0xdd79u] = 1u;
    low_memory[0x12c9u] = 2u;

    assert(BuildRecordableMediaReadDiscInformationResponse() == 1u);
    assert(low_memory[0x0b2cu] == 0x0eu);
    assert(low_memory[0x0b2du] == 1u && low_memory[0x0b2eu] == 2u);
    assert(low_memory[0x0b2fu] == 4u && low_memory[0x0b30u] == 9u);
    assert(first_track_call_count == 1u && convert_count == 0u);
    assert(low_memory[0x0b3bu] == 0xffu);
    assert(low_memory[0x0b3cu] == 0xffu);
    assert(low_memory[0x0b3du] == 0xffu);
    assert(low_memory[0x0b3fu] == 0xffu);
    assert(low_memory[0x0b40u] == 0xffu);
    assert(low_memory[0x0b41u] == 0xffu);
}

static void TestModeTwoStatusAndPmaDiscId(void)
{
    ResetFixture();
    low_memory[0x1008u] = 2u;
    low_memory[0x0c9eu] = 3u;
    low_memory[0x0c9fu] = 0x40u;
    low_memory[0xfb7cu] = 0x40u;
    low_memory[0xdd79u] = 1u;
    low_memory[0x114eu] = 1u;
    far_2b_memory[0xb5deu] = 12u;
    far_2b_memory[0xb5dfu] = 34u;
    far_2b_memory[0xb5e0u] = 56u;

    assert(BuildRecordableMediaReadDiscInformationResponse() == 1u);
    assert(low_memory[0x0b2cu] == 0x1eu);
    assert(low_memory[0x0b2eu] == 1u);
    assert(low_memory[0x0b2fu] == 1u && low_memory[0x0b30u] == 1u);
    assert(low_memory[0x0b31u] == 0xe3u);
    assert(multiply_count == 2u);
    assert(multiply_left[0] == 100ul && multiply_right[0] == 34ul);
    assert(multiply_left[1] == 10000ul && multiply_right[1] == 12ul);
    assert(ReadDword(0x0b36u) == 123456ul);
    assert(convert_count == 2u);
}

int main(void)
{
    TestOpenNormalResponse();
    TestSpecialFixedPacketTrackSelection();
    TestClosedSessionUsesSentinels();
    TestModeTwoStatusAndPmaDiscId();
    return 0;
}
