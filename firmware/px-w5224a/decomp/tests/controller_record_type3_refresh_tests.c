#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_type3_refresh.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static s16 updated_indices[64];
static unsigned update_count;
static unsigned scan_count;
static u16 scan_result;

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

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void WriteBoundary(fw_addr_t table, u16 segment, u16 index)
{
    WriteU32(table + (u16)(segment * 4u), (u32)index * 0x27ul + 0x91ul);
}

void ObserveType3RecordBlockUpdate(s16 position_block_index)
{
    assert(update_count < 64u);
    updated_indices[update_count++] = position_block_index;
}

u16 ObserveType3RecordBankScan(u8 force_bank_scan, s32 start_frame_position, u32 frame_count)
{
    assert(force_bank_scan == 1u);
    assert(start_frame_position == (s32)ReadU32(0x001153u));
    assert(frame_count == 0ul);
    ++scan_count;
    return scan_result;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(updated_indices, 0, sizeof(updated_indices));
    update_count = scan_count = 0u;
    scan_result = 0u;
    WriteU32(0x00faffu, 0x00020000ul);
    WriteU32(0x001153u, 0x00123456ul);
}

static void TestRangeBelowThirtyTwoIsIgnored(void)
{
    ResetFixture();
    RefreshType3ControllerRecordBlocksForIndexRange(0u, 31u);
    assert(update_count == 0u && scan_count == 0u);
}

static void TestClampedOverlappingPartialRanges(void)
{
    static const s16 expected[] = { 33, 35, 45, 33, 35 };

    ResetFixture();
    WriteBoundary(0xf84490ul, 0u, 30u);
    WriteBoundary(0xf84690ul, 0u, 50u);
    memory[0x00020000u + 33u] = 3u;
    memory[0x00020000u + 35u] = 3u;
    memory[0x00020000u + 45u] = 3u;
    RefreshType3ControllerRecordBlocksForIndexRange(10u, 40u);
    assert(update_count == sizeof(expected) / sizeof(expected[0]));
    assert(memcmp(updated_indices, expected, sizeof(expected)) == 0);
    assert(scan_count == 1u);
    assert(ReadU32(0x000ce1u) == 0x00123456ul && ReadU32(0x000ce5u) == 0x00123456ul);
}

static void TestCompleteMiddleSegmentAndMatchedSummary(void)
{
    static const s16 expected[] = { 201, 204 };

    ResetFixture();
    WriteBoundary(0xf84690ul, 1u, 140u);
    WriteBoundary(0xf84490ul, 2u, 200u);
    WriteBoundary(0xf84690ul, 2u, 290u);
    memory[0x00020000u + 201u] = 3u;
    memory[0x00020000u + 204u] = 3u;
    scan_result = 2u;
    WriteU32(0x2b9704u, 0x01020304ul);
    WriteU32(0x2b970cu, 0xaabbccddu);
    RefreshType3ControllerRecordBlocksForIndexRange(150u, 300u);
    assert(update_count == sizeof(expected) / sizeof(expected[0]));
    assert(memcmp(updated_indices, expected, sizeof(expected)) == 0);
    assert(scan_count == 1u);
    assert(ReadU32(0x000ce1u) == 0x01020304ul && ReadU32(0x000ce5u) == 0x01020304ul);
}

static void TestNonTypeThreeStateDoesNotPublish(void)
{
    ResetFixture();
    WriteBoundary(0xf84490ul, 0u, 32u);
    WriteBoundary(0xf84690ul, 0u, 40u);
    memory[0x00020021u] = 2u;
    RefreshType3ControllerRecordBlocksForIndexRange(32u, 35u);
    assert(update_count == 0u && scan_count == 0u);
}

int main(void)
{
    TestRangeBelowThirtyTwoIsIgnored();
    TestClampedOverlappingPartialRanges();
    TestCompleteMiddleSegmentAndMatchedSummary();
    TestNonTypeThreeStateDoesNotPublish();
    puts("controller record type-3 refresh tests passed");
    return 0;
}
