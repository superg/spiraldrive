#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/synthetic_raw_read_q_record_table.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x10000];
static unsigned pack_count;
static unsigned frame_to_msf_count;
static unsigned msf_to_frame_count;

static fw_addr_t RawTrackRecord(u16 index)
{
    return 0xb4bdul + (fw_addr_t)(index * 8u);
}

static fw_addr_t RawQRecord(u16 index)
{
    return 0x8012ul + (fw_addr_t)(index * 10u);
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_image[address] << 8) | memory_image[address + 1u]);
}

static u8 ToBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

static void SetRawTrackRecord(u16 index, u8 control, u8 track, u8 track_index, u8 minute, u8 second, u8 frame)
{
    fw_addr_t record = RawTrackRecord(index);

    memory_image[record] = control;
    memory_image[record + 1u] = track;
    memory_image[record + 2u] = track_index;
    memory_image[record + 4u] = minute;
    memory_image[record + 6u] = second;
    memory_image[record + 7u] = frame;
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    pack_count = 0u;
    frame_to_msf_count = 0u;
    msf_to_frame_count = 0u;
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

u8 BinaryByteToPackedBcd(u8 value)
{
    return ToBcd(value);
}

u16 FindType1RawTrackRecordIndexByTrackNumberOrLeadOut(u8 track_number)
{
    u16 index = 0u;

    for(;;)
    {
        fw_addr_t record = RawTrackRecord(index);

        if(memory_image[record + 1u] == 0xaau)
            return index;
        if(memory_image[record + 1u] == track_number && memory_image[record + 2u] == 1u && (memory_image[record] & 0x0fu) == 1u)
            return index;
        ++index;
    }
}

void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address)
{
    u8 index;

    ++pack_count;
    for(index = 0u; index < 3u; ++index)
        memory_image[(u16)(tuple_address + index)] = ToBcd(memory_image[(u16)(tuple_address + index)]);
}

void CopyThreeBytes16(u16 destination, u16 source)
{
    u8 index;

    for(index = 0u; index < 3u; ++index)
        memory_image[(u16)(destination + index)] = memory_image[(u16)(source + index)];
}

s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address)
{
    u16 seconds;

    ++msf_to_frame_count;
    seconds = (u16)((u16)memory_image[tuple_address] * 60u + memory_image[tuple_address + 1u]);
    return (s32)((u32)seconds * 75ul + memory_image[tuple_address + 2u]);
}

void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t tuple_address)
{
    u32 value = (u32)frame_position;
    u32 seconds = value / 75ul;

    ++frame_to_msf_count;
    memory_image[tuple_address] = (u8)(seconds / 60ul);
    memory_image[tuple_address + 1u] = (u8)(seconds % 60ul);
    memory_image[tuple_address + 2u] = (u8)(value % 75ul);
}

static void ConfigureSummaryAndLeadOut(void)
{
    memory_image[0x1400u] = 2u;
    memory_image[0x1401u] = 3u;
    memory_image[0x1402u] = 0x20u;
    memory_image[0x1406u] = 0x11u;
    memory_image[0x1407u] = 0x22u;
    memory_image[0x1408u] = 0x33u;
}

static void TestOrdinaryTrackTable(void)
{
    fw_addr_t record;

    ResetHarness();
    ConfigureSummaryAndLeadOut();
    SetRawTrackRecord(0u, 0x02u, 0u, 0u, 0u, 0u, 0u);
    SetRawTrackRecord(1u, 0x01u, 0u, 1u, 0u, 0u, 0u);
    SetRawTrackRecord(2u, 0x41u, 1u, 1u, 1u, 2u, 3u);
    SetRawTrackRecord(3u, 0x11u, 2u, 1u, 4u, 5u, 6u);
    SetRawTrackRecord(4u, 0x21u, 2u, 2u, 8u, 8u, 8u);
    SetRawTrackRecord(5u, 0x31u, 3u, 1u, 7u, 8u, 9u);
    SetRawTrackRecord(6u, 0x51u, 0xaau, 1u, 10u, 11u, 12u);
    WriteWord(0xb4bbu, 8u);

    BuildSyntheticRawReadQRecordTableFromTrackState();

    assert(memory_image[RawQRecord(0u)] == 0x11u);
    assert(memory_image[RawQRecord(0u) + 2u] == 0xa0u);
    assert(memory_image[RawQRecord(0u) + 7u] == 0x02u);
    assert(memory_image[RawQRecord(0u) + 8u] == 0x20u);
    assert(memory_image[RawQRecord(1u)] == 0x31u);
    assert(memory_image[RawQRecord(1u) + 2u] == 0xa1u);
    assert(memory_image[RawQRecord(1u) + 7u] == 0x03u);
    assert(memory_image[RawQRecord(2u)] == 0x51u);
    assert(memory_image[RawQRecord(2u) + 2u] == 0xa2u);
    assert(memory_image[RawQRecord(2u) + 7u] == 0x11u);
    assert(memory_image[RawQRecord(2u) + 8u] == 0x22u);
    assert(memory_image[RawQRecord(2u) + 9u] == 0x33u);

    record = RawQRecord(3u);
    assert(memory_image[record] == 0x41u);
    assert(memory_image[record + 2u] == 0x01u);
    assert(memory_image[record + 7u] == 0x01u);
    assert(memory_image[record + 8u] == 0x02u);
    assert(memory_image[record + 9u] == 0x03u);
    record = RawQRecord(4u);
    assert(memory_image[record] == 0x11u);
    assert(memory_image[record + 2u] == 0x02u);
    assert(memory_image[record + 7u] == 0x04u);
    assert(memory_image[record + 8u] == 0x05u);
    assert(memory_image[record + 9u] == 0x06u);
    record = RawQRecord(5u);
    assert(memory_image[record] == 0x31u);
    assert(memory_image[record + 2u] == 0x03u);
    assert(memory_image[record + 7u] == 0x07u);
    assert(memory_image[record + 8u] == 0x08u);
    assert(memory_image[record + 9u] == 0x09u);
    assert(pack_count == 3u);
    assert(ReadWord(0x8008u) == 6u);
    assert(ReadWord(0x800au) == 6u);
    assert(ReadWord(0x800cu) == 6u);
}

static void TestMode10PrefixFiltering(void)
{
    ResetHarness();
    ConfigureSummaryAndLeadOut();
    memory_image[0x0c4au] = 0x10u;
    SetRawTrackRecord(0u, 0x41u, 1u, 1u, 1u, 1u, 1u);
    SetRawTrackRecord(1u, 0x01u, 1u, 1u, 2u, 2u, 2u);
    SetRawTrackRecord(2u, 0x01u, 2u, 1u, 3u, 4u, 5u);
    SetRawTrackRecord(3u, 0x01u, 0xaau, 1u, 6u, 7u, 8u);
    WriteWord(0xb4bbu, 5u);

    BuildSyntheticRawReadQRecordTableFromTrackState();

    assert(memory_image[RawQRecord(3u) + 2u] == 0x02u);
    assert(memory_image[RawQRecord(3u) + 7u] == 0x03u);
    assert(ReadWord(0x8008u) == 4u);
    assert(ReadWord(0x800au) == 4u);
}

static void ConfigureOnlyLeadOutTrack(void)
{
    ConfigureSummaryAndLeadOut();
    SetRawTrackRecord(0u, 0x51u, 0xaau, 1u, 1u, 2u, 3u);
    WriteWord(0xb4bbu, 2u);
}

static void TestB0AllOnesTuple(void)
{
    fw_addr_t b0 = RawQRecord(3u);

    ResetHarness();
    ConfigureOnlyLeadOutTrack();
    memory_image[0x0c45u] = 0x40u;
    memory_image[0x0c47u] = 0x0au;
    memory_image[0x113bu] = 12u;
    memory_image[0x113cu] = 34u;
    memory_image[0x113du] = 56u;
    WriteDword(0xdd69u, 1ul);

    BuildSyntheticRawReadQRecordTableFromTrackState();

    assert(memory_image[b0] == 0xa5u);
    assert(memory_image[b0 + 1u] == 0u);
    assert(memory_image[b0 + 2u] == 0xb0u);
    assert(memory_image[b0 + 3u] == 0xffu);
    assert(memory_image[b0 + 4u] == 0xffu);
    assert(memory_image[b0 + 5u] == 0xffu);
    assert(memory_image[b0 + 6u] == 1u);
    assert(memory_image[b0 + 7u] == 0x12u);
    assert(memory_image[b0 + 8u] == 0x34u);
    assert(memory_image[b0 + 9u] == 0x56u);
    assert(msf_to_frame_count == 0u);
    assert(ReadWord(0x8008u) == 3u);
    assert(ReadWord(0x800au) == 4u);
    assert(ReadWord(0x800cu) == 4u);
}

static void TestComputedB0AndC0C1Records(void)
{
    fw_addr_t b0 = RawQRecord(3u);
    fw_addr_t c0 = RawQRecord(4u);
    fw_addr_t c1 = RawQRecord(5u);

    ResetHarness();
    ConfigureOnlyLeadOutTrack();
    memory_image[0x0c45u] = 0x80u;
    memory_image[0x0c47u] = 3u;
    memory_image[0x1008u] = 2u;
    memory_image[0xfb5bu] = 0x10u;
    memory_image[0x113bu] = 12u;
    memory_image[0x113cu] = 34u;
    memory_image[0x113du] = 56u;
    memory_image[0x113fu] = 7u;
    memory_image[0x1140u] = 8u;
    memory_image[0x1141u] = 9u;
    memory_image[0xfb7au] = 1u;
    memory_image[0xfb7bu] = 2u;
    memory_image[0xfb7cu] = 4u;
    memory_image[0xfb7du] = 8u;
    memory_image[0xfb7eu] = 0x10u;
    memory_image[0xfb7fu] = 1u;
    memory_image[0xfb80u] = 4u;
    memory_image[0xfb81u] = 2u;
    memory_image[0xfb82u] = 8u;
    memory_image[0xfb83u] = 3u;
    WriteWord(0x149fu, 0u);
    WriteDword(0xdd69u, 0xfffffffful);

    BuildSyntheticRawReadQRecordTableFromTrackState();

    assert(memory_image[b0] == 0x35u);
    assert(memory_image[b0 + 2u] == 0xb0u);
    assert(memory_image[b0 + 3u] == 0x03u);
    assert(memory_image[b0 + 4u] == 0x32u);
    assert(memory_image[b0 + 5u] == 0x03u);
    assert(memory_image[b0 + 6u] == 3u);
    assert(memory_image[c0] == 0x35u);
    assert(memory_image[c0 + 2u] == 0xc0u);
    assert(memory_image[c0 + 3u] == 6u);
    assert(memory_image[c0 + 4u] == 8u);
    assert(memory_image[c0 + 5u] == 0x30u);
    assert(memory_image[c0 + 7u] == 0x07u);
    assert(memory_image[c0 + 8u] == 0x08u);
    assert(memory_image[c0 + 9u] == 0x09u);
    assert(memory_image[c1] == 0x35u);
    assert(memory_image[c1 + 2u] == 0xc1u);
    assert(memory_image[c1 + 3u] == 10u);
    assert(memory_image[c1 + 4u] == 20u);
    assert(memory_image[c1 + 5u] == 6u);
    assert(msf_to_frame_count == 1u);
    assert(frame_to_msf_count == 1u);
    assert(ReadWord(0x8008u) == 3u);
    assert(ReadWord(0x800au) == 6u);
    assert(ReadWord(0x800cu) == 6u);
}

static void TestComputedB0ExistingSessionOffset(void)
{
    fw_addr_t b0 = RawQRecord(3u);

    ResetHarness();
    ConfigureOnlyLeadOutTrack();
    memory_image[0x0c45u] = 0x80u;
    memory_image[0x12c9u] = 1u;
    WriteWord(0x149fu, 0u);
    WriteDword(0xdd69u, 1ul);

    BuildSyntheticRawReadQRecordTableFromTrackState();

    assert(memory_image[b0 + 3u] == 0x02u);
    assert(memory_image[b0 + 4u] == 0x32u);
    assert(memory_image[b0 + 5u] == 0x03u);
}

int main(void)
{
    TestOrdinaryTrackTable();
    TestMode10PrefixFiltering();
    TestB0AllOnesTuple();
    TestComputedB0AndC0C1Records();
    TestComputedB0ExistingSessionOffset();
    puts("synthetic raw read Q-record table tests passed");
    return 0;
}
