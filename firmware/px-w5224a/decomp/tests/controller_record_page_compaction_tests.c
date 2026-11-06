#include <assert.h>
#include <string.h>
#include "platform/controller_record_page_compaction.h"
#include "platform/firmware_memory.h"

typedef struct
{
    u32 destination;
    fw_addr_t source;
    u8 count;
} CopyCall;

static u8 memory[0x1000000ul];
static unsigned page_copy_count;
static CopyCall copy_calls[16];
static unsigned copy_call_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
void CopyControllerBufferRecordPageToScratch(void)
{
    ++page_copy_count;
}
void CopyBytesToAddressPlus1MiB(fw_addr_t destination, fw_addr_t source, u8 count)
{
    copy_calls[copy_call_count].destination = destination;
    copy_calls[copy_call_count].source = source;
    copy_calls[copy_call_count].count = count;
    ++copy_call_count;
}

static void WriteAddress(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void WriteRecord(u16 index, u8 type, u32 first_key, u32 second_key)
{
    fw_addr_t address = 0x2b971cul + (u16)(index * 6u);

    memory[address] = (u8)((type << 4) | ((first_key >> 16) & 0x0fu));
    memory[address + 1u] = (u8)(first_key >> 8);
    memory[address + 2u] = (u8)first_key;
    memory[address + 3u] = (u8)(0xa0u | ((second_key >> 16) & 0x0fu));
    memory[address + 4u] = (u8)(second_key >> 8);
    memory[address + 5u] = (u8)second_key;
}

static u32 ReadTriplet(u16 index, u8 second)
{
    fw_addr_t address = 0x2b971cul + (u16)(index * 6u) + (second != 0u ? 3u : 0u);

    return ((u32)memory[address] << 16) | ((u32)memory[address + 1u] << 8) | memory[address + 2u];
}

static void Reset(void)
{
    memset(memory, 0x20, sizeof(memory));
    memset(copy_calls, 0, sizeof(copy_calls));
    page_copy_count = copy_call_count = 0u;
    WriteAddress(0x2ba51cul, 0x00094020ul);
}

static void TestLeadingTypeThreeRecordsSortBySecondKey(void)
{
    Reset();
    WriteRecord(0u, 3u, 0x30002ul, 0x300ul);
    WriteRecord(1u, 3u, 0x30001ul, 0x100ul);
    WriteRecord(2u, 2u, 0x20001ul, 0x200ul);
    CompactAndWriteControllerBufferRecordPage(0u);
    assert(page_copy_count == 1u);
    assert(ReadTriplet(0u, 0u) == 0x330001ul);
    assert(ReadTriplet(0u, 1u) == 0xa00100ul);
    assert(ReadTriplet(1u, 0u) == 0x330002ul);
    assert(ReadTriplet(1u, 1u) == 0xa00300ul);
    assert(copy_call_count == 1u);
    assert(copy_calls[0].destination == 0x00094020ul && copy_calls[0].source == 0x2b971cul && copy_calls[0].count == 12u);
}

static void TestTypeZeroAndOneRecordsSortByFirstKey(void)
{
    Reset();
    WriteRecord(0u, 2u, 0x20000ul, 0x400ul);
    WriteRecord(1u, 1u, 0x10030ul, 0x300ul);
    WriteRecord(2u, 0u, 0x00010ul, 0x200ul);
    WriteRecord(3u, 1u, 0x10020ul, 0x100ul);
    CompactAndWriteControllerBufferRecordPage(1u);
    assert(ReadTriplet(0u, 0u) == 0x000010ul);
    assert(ReadTriplet(1u, 0u) == 0x110020ul);
    assert(ReadTriplet(2u, 0u) == 0x110030ul);
    assert(copy_calls[0].count == 18u);
}

static void TestCopyIsSplitInto255ByteChunks(void)
{
    u16 index;

    Reset();
    for(index = 0u; index < 43u; ++index)
        WriteRecord(index, 1u, (u32)index + 1ul, (u32)index + 0x100ul);
    CompactAndWriteControllerBufferRecordPage(1u);
    assert(copy_call_count == 2u);
    assert(copy_calls[0].destination == 0x00094020ul && copy_calls[0].source == 0x2b971cul && copy_calls[0].count == 255u);
    assert(copy_calls[1].destination == 0x0009411ful && copy_calls[1].source == 0x2b981bul && copy_calls[1].count == 3u);
}

static void TestEmptySelectionStillIssuesZeroLengthCopy(void)
{
    Reset();
    CompactAndWriteControllerBufferRecordPage(0u);
    assert(copy_call_count == 1u && copy_calls[0].count == 0u);
}

int main(void)
{
    TestLeadingTypeThreeRecordsSortBySecondKey();
    TestTypeZeroAndOneRecordsSortByFirstKey();
    TestCopyIsSplitInto255ByteChunks();
    TestEmptySelectionStillIssuesZeroLengthCopy();
    return 0;
}
