#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_q_record_table_population.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x1000000];
static u8 configuration_bytes[4];
static u8 selected_track_result;
static u16 random_result;
static u16 random_seed;

static fw_addr_t RecordAddress(u8 index)
{
    return 0x8012ul + (fw_addr_t)((u16)index * 10u);
}

static fw_addr_t PmaAddress(fw_addr_t base, u8 index)
{
    return base + (fw_addr_t)((u16)index * 10u);
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

static u8 FromBcd(u8 value)
{
    return (u8)(((value >> 4) * 10u) + (value & 0x0fu));
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(configuration_bytes, 0, sizeof(configuration_bytes));
    selected_track_result = 0u;
    random_result = 0u;
    random_seed = 0xffffu;
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

void ObserveRawQPopulationReadConfiguration(fw_addr_t offset, u8 *destination, u16 count)
{
    assert(offset == 0x7aul);
    assert(count == 4u);
    memcpy(destination, configuration_bytes, 4u);
}

void ObserveRawQPopulationResetRandom(u16 seed)
{
    random_seed = seed;
}

u16 ObserveRawQPopulationNextRandom(void)
{
    return random_result;
}

u32 ObserveRawQPopulationMultiply(u32 left, u32 right)
{
    return left * right;
}

u8 ObserveRawQPopulationSelectTrack(void)
{
    return selected_track_result;
}

u8 ObserveRawQPopulationBinaryToBcd(u8 value)
{
    return ToBcd(value);
}

void ObserveRawQPopulationFrameToMsf(s32 frame_position, fw_addr_t destination)
{
    u32 position = (u32)frame_position;
    u32 total_seconds = position / 75ul;

    memory_image[destination] = (u8)(total_seconds / 60ul);
    memory_image[destination + 1u] = (u8)(total_seconds % 60ul);
    memory_image[destination + 2u] = (u8)(position % 75ul);
}

void ObserveRawQPopulationPackTuple(u16 address)
{
    u8 index;

    for(index = 0u; index < 3u; ++index)
        memory_image[(u16)(address + index)] = ToBcd(memory_image[(u16)(address + index)]);
}

void ObserveRawQPopulationUnpackTuple(u16 address)
{
    u8 index;

    for(index = 0u; index < 3u; ++index)
        memory_image[(u16)(address + index)] = FromBcd(memory_image[(u16)(address + index)]);
}

static void FillRecord(fw_addr_t address, u8 control, u8 point, u8 tuple1, u8 tuple2)
{
    u8 index;

    for(index = 0u; index < 10u; ++index)
        memory_image[address + index] = 0u;
    memory_image[address] = control;
    memory_image[address + 2u] = point;
    memory_image[address + 3u] = tuple1;
    memory_image[address + 4u] = tuple1;
    memory_image[address + 5u] = tuple1;
    memory_image[address + 7u] = tuple2;
    memory_image[address + 8u] = tuple2;
    memory_image[address + 9u] = tuple2;
}

static void TestTailClearAndEmptySelection(void)
{
    fw_addr_t table_end = RecordAddress(220u);
    fw_addr_t address;

    ResetHarness();
    for(address = RecordAddress(0u); address < table_end; ++address)
        memory_image[address] = 0xa5u;
    memory_image[0x1149u] = 1u;
    memory_image[0x14ccu] = 0u;

    assert(PopulateRawReadQRecordTableFromIndex(5u) == 1u);
    assert(memory_image[RecordAddress(4u)] == 0xa5u);
    assert(memory_image[RecordAddress(5u)] == 0u);
    assert(memory_image[RecordAddress(219u) + 9u] == 0u);
    assert(ReadWord(0x800eu) == 0u);
    assert(ReadWord(0x8010u) == 0u);
}

static void TestRejectedTrackSelection(void)
{
    ResetHarness();
    memory_image[0x1149u] = 1u;
    memory_image[0x14ccu] = 0xffu;
    memory_image[0x800eu] = 0x12u;
    memory_image[0x800fu] = 0x34u;

    assert(PopulateRawReadQRecordTableFromIndex(0u) == 0u);
    assert(ReadWord(0x800eu) == 0x1234u);
}

static void TestSyntheticSeedAndWindowRecord(void)
{
    fw_addr_t first = RecordAddress(0u);
    fw_addr_t second = RecordAddress(1u);

    ResetHarness();
    memory_image[0x0c9eu] = 1u;
    configuration_bytes[2] = 0x12u;
    configuration_bytes[3] = 0x34u;
    WriteDword(0x2ba58dul, 4500ul);

    assert(PopulateRawReadQRecordTableFromIndex(0u) == 1u);
    assert(random_seed == 0x1234u);
    assert(memory_image[first] == 2u);
    assert(memory_image[first + 3u] == 0x0cu);
    assert(memory_image[first + 4u] == 0x22u);
    assert(memory_image[first + 5u] == 0x38u);
    assert(memory_image[first + 8u] == 0x20u);
    assert(memory_image[second] == 0x71u);
    assert(memory_image[second + 2u] == 1u);
    assert(memory_image[second + 3u] == 0x01u);
    assert(memory_image[second + 4u] == 0x00u);
    assert(memory_image[second + 5u] == 0x00u);
    assert(memory_image[second + 7u] == 0x00u);
    assert(memory_image[second + 8u] == 0x02u);
    assert(memory_image[second + 9u] == 0x00u);
    assert(ReadWord(0x800eu) == 2u);
    assert(ReadWord(0x8010u) == 20u);
}

static void TestAdrMergeFilteringAndOverwrite(void)
{
    fw_addr_t adr1_record1 = PmaAddress(0x2bb03bul, 1u);
    fw_addr_t adr1_record2 = PmaAddress(0x2bb03bul, 2u);
    fw_addr_t adr3_record1 = PmaAddress(0x2bb423ul, 1u);
    fw_addr_t adr3_record2 = PmaAddress(0x2bb423ul, 2u);
    fw_addr_t adr4_record1 = PmaAddress(0x2bb4fful, 1u);
    fw_addr_t record0 = RecordAddress(0u);
    fw_addr_t record1 = RecordAddress(1u);
    fw_addr_t record2 = RecordAddress(2u);
    fw_addr_t record3 = RecordAddress(3u);

    ResetHarness();
    memory_image[0x1149u] = 2u;
    memory_image[0x114au] = 2u;
    memory_image[0x114bu] = 1u;
    memory_image[0x14ccu] = 2u;
    memory_image[0x0c44u] = 0u;
    memory_image[0xdd35u] = 2u;
    memory_image[0xdd36u] = 0x45u;
    WriteDword(0x8000u, 150ul);
    WriteDword(0xdd3du, 225ul);
    FillRecord(adr1_record1, 0x41u, 1u, 12u, 15u);
    FillRecord(adr1_record2, 0x41u, 2u, 20u, 21u);
    FillRecord(adr3_record1, 3u, 3u, 1u, 0u);
    FillRecord(adr3_record2, 3u, 4u, 2u, 2u);
    FillRecord(adr4_record1, 4u, 5u, 1u, 1u);

    assert(PopulateRawReadQRecordTableFromIndex(1u) == 1u);
    assert(memory_image[record0] == 0x41u);
    assert(memory_image[record0 + 2u] == 0x01u);
    assert(memory_image[record0 + 3u] == 0x12u);
    assert(memory_image[record0 + 7u] == 0x15u);
    assert(memory_image[record1] == 0x45u);
    assert(memory_image[record1 + 2u] == 0x02u);
    assert(memory_image[record1 + 4u] == 0x02u);
    assert(memory_image[record1 + 8u] == 0x03u);
    assert(memory_image[record2] == 3u);
    assert(memory_image[record3] == 4u);
    assert(memory_image[record3 + 3u] == 1u);
    assert(memory_image[record3 + 7u] == 1u);
    assert(ReadWord(0x800eu) == 4u);
    assert(ReadWord(0x8010u) == 40u);
}

static void TestPackedPmaTupleConversion(void)
{
    fw_addr_t source = PmaAddress(0x2bb423ul, 1u);
    fw_addr_t output = RecordAddress(0u);

    ResetHarness();
    memory_image[0x88b6u] = 1u;
    memory_image[0x88b8u] = 6u;
    memory_image[0x1149u] = 1u;
    memory_image[0x114au] = 1u;
    memory_image[0x14ccu] = 0x20u;
    memory_image[PmaAddress(0x2bb03bul, 1u) + 2u] = 0xffu;
    FillRecord(source, 3u, 18u, 0x12u, 0x11u);

    assert(PopulateRawReadQRecordTableFromIndex(0u) == 1u);
    assert(memory_image[output + 2u] == 0x18u);
    assert(memory_image[output + 3u] == 12u);
    assert(memory_image[output + 4u] == 12u);
    assert(memory_image[output + 5u] == 12u);
    assert(memory_image[output + 7u] == 11u);
    assert(ReadWord(0x800eu) == 1u);
    assert(ReadWord(0x8010u) == 10u);
}

int main(void)
{
    TestTailClearAndEmptySelection();
    TestRejectedTrackSelection();
    TestSyntheticSeedAndWindowRecord();
    TestAdrMergeFilteringAndOverwrite();
    TestPackedPmaTupleConversion();
    puts("raw read Q-record table population tests passed");
    return 0;
}
