#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_toc_summary.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x20000];
static u8 convert_calls;

static u16 RecordAddress(u16 index)
{
    return (u16)(0xb4bdu + (u16)(index << 3));
}

static void SetRecord(u16 index, u8 flags, u8 track, u8 type, u8 minute, u8 second, u8 frame)
{
    u16 address = RecordAddress(index);
    memory_image[address] = flags;
    memory_image[address + 1u] = track;
    memory_image[address + 3u] = type;
    memory_image[address + 4u] = minute;
    memory_image[address + 6u] = second;
    memory_image[address + 7u] = frame;
}

static u8 PackedBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

static void ResetHarness(void)
{
    memset(memory_image, 0xa5, sizeof(memory_image));
    convert_calls = 0u;
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

void ObserveRawReadTocConvertTuple(u16 tuple_address)
{
    ++convert_calls;
    assert(tuple_address == 0x7000u);
    memory_image[tuple_address] = PackedBcd(memory_image[tuple_address]);
    memory_image[tuple_address + 1u] = PackedBcd(memory_image[tuple_address + 1u]);
    memory_image[tuple_address + 2u] = PackedBcd(memory_image[tuple_address + 2u]);
}

static void TestIneligibleWriteTypes(void)
{
    u8 type;
    for(type = 0u; type < 4u; ++type)
    {
        if(type == 2u)
            continue;
        ResetHarness();
        memory_image[0x0c44u] = type;
        memory_image[0x1400u] = 0x55u;
        InitializeRawReadTocSummaryFromTrackTable();
        assert(memory_image[0x1400u] == 0x55u && convert_calls == 0u);
    }
}

static void TestOrdinarySummary(void)
{
    ResetHarness();
    memory_image[0x0c44u] = 2u;
    memory_image[0x0c4au] = 0u;
    SetRecord(0u, 0u, 0u, 0u, 0u, 0u, 0u);
    SetRecord(1u, 2u, 4u, 0u, 0u, 0u, 0u);
    SetRecord(2u, 1u, 5u, 0x20u, 0u, 0u, 0u);
    SetRecord(3u, 1u, 6u, 0u, 0u, 0u, 0u);
    SetRecord(4u, 1u, 0xaau, 0u, 12u, 34u, 56u);
    InitializeRawReadTocSummaryFromTrackTable();
    assert(memory_image[0x1400u] == 5u);
    assert(memory_image[0x1401u] == 6u);
    assert(memory_image[0x1402u] == 0x20u);
    assert(memory_image[0x1406u] == 0x12u && memory_image[0x1407u] == 0x34u && memory_image[0x1408u] == 0x56u);
    assert(convert_calls == 1u);

    ResetHarness();
    memory_image[0x0c44u] = 2u;
    memory_image[0x0c4au] = 1u;
    SetRecord(0u, 1u, 1u, 0x10u, 0u, 0u, 0u);
    SetRecord(1u, 1u, 0xaau, 0u, 0u, 0u, 0u);
    InitializeRawReadTocSummaryFromTrackTable();
    assert(memory_image[0x1402u] == 0u);
}

static void TestSpecialSummary(void)
{
    ResetHarness();
    memory_image[0x0c44u] = 2u;
    memory_image[0x0c4au] = 0x10u;
    SetRecord(0u, 1u, 2u, 0u, 0u, 0u, 0u);
    SetRecord(1u, 0x41u, 3u, 0u, 0u, 0u, 0u);
    SetRecord(2u, 1u, 4u, 0u, 0u, 0u, 0u);
    SetRecord(3u, 1u, 0xaau, 0u, 1u, 2u, 3u);
    InitializeRawReadTocSummaryFromTrackTable();
    assert(memory_image[0x1400u] == 4u);
    assert(memory_image[0x1401u] == 4u);
    assert(memory_image[0x1402u] == 0x10u);
    assert(memory_image[0x1406u] == 1u && memory_image[0x1407u] == 2u && memory_image[0x1408u] == 3u);
    assert(convert_calls == 1u);
}

int main(void)
{
    TestIneligibleWriteTypes();
    TestOrdinarySummary();
    TestSpecialSummary();
    puts("raw-read TOC summary tests passed");
    return 0;
}
