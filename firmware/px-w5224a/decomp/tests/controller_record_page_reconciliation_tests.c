#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_page_reconciliation.h"
#include "platform/firmware_memory.h"

struct RebuildCall
{
    u8 group;
    u8 page;
};

struct TransferCall
{
    u32 first;
    u32 second;
    u32 count;
};

static u8 memory[0x200000ul];
static struct RebuildCall rebuild_calls[8];
static struct TransferCall transfer_calls[8];
static unsigned rebuild_count;
static unsigned transfer_count;
static unsigned yield_count;

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

void ObserveBackwardRecordPageRebuild(u8 group_index, u8 page_index)
{
    assert(rebuild_count < sizeof(rebuild_calls) / sizeof(rebuild_calls[0]));
    rebuild_calls[rebuild_count].group = group_index;
    rebuild_calls[rebuild_count].page = page_index;
    ++rebuild_count;
}

void ObserveBackwardRecordPageTransfer(u32 first_controller_address, u32 second_controller_address, u32 controller_word_count)
{
    assert(transfer_count < sizeof(transfer_calls) / sizeof(transfer_calls[0]));
    transfer_calls[transfer_count].first = first_controller_address;
    transfer_calls[transfer_count].second = second_controller_address;
    transfer_calls[transfer_count].count = controller_word_count;
    ++transfer_count;
}

void ObserveBackwardRecordPageYield(void)
{
    ++yield_count;
}

static fw_addr_t Page(u8 group, u8 page)
{
    return 0x194000ul + (fw_addr_t)group * 0x4000ul + (u16)page * 0x0800u;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteU24(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 16);
    memory[address + 1u] = (u8)(value >> 8);
    memory[address + 2u] = (u8)value;
}

static u32 ReadU24(fw_addr_t address)
{
    return ((u32)memory[address] << 16) | ((u32)memory[address + 1u] << 8) | memory[address + 2u];
}

static void WriteRecord(fw_addr_t address, u8 byte0, u8 byte1, u8 byte2, u8 byte3, u8 byte4, u8 byte5)
{
    memory[address] = byte0;
    memory[address + 1u] = byte1;
    memory[address + 2u] = byte2;
    memory[address + 3u] = byte3;
    memory[address + 4u] = byte4;
    memory[address + 5u] = byte5;
}

static void ResetFixture(void)
{
    u8 group;
    u8 page;

    memset(memory, 0, sizeof(memory));
    rebuild_count = 0u;
    transfer_count = 0u;
    yield_count = 0u;
    for(group = 0u; group <= 8u; ++group)
        for(page = 0u; page < 8u; ++page)
            WriteU24(Page(group, page) + 0x15u, 0x00fffffful);
}

static void TestImmediateBoundaryExitRefreshesSelectedHeader(void)
{
    fw_addr_t selected = Page(0u, 1u);
    fw_addr_t previous = Page(0u, 0u);

    ResetFixture();
    WriteRecord(selected + 0x20u, 0xa5u, 0x01u, 0x02u, 0x13u, 0x14u, 0x15u);
    WriteU24(previous + 0x15u, 0x060000ul);

    ReconcileControllerBufferRecordsBackward(0u, 1u);

    assert(ReadU24(selected + 0x11u) == 0x050102ul);
    assert(rebuild_count == 0u);
    assert(transfer_count == 0u);
    assert(yield_count == 0u);
}

static void TestBoundarySwapRebuildsPreviousPage(void)
{
    static const u8 selected_record[6] = { 0xa5, 1, 2, 0x13, 0x14, 0x15 };
    static const u8 displaced_record[6] = { 0xb4, 2, 3, 0x23, 0x24, 0x25 };
    fw_addr_t selected = Page(0u, 1u);
    fw_addr_t previous = Page(0u, 0u);

    ResetFixture();
    memcpy(memory + selected + 0x20u, selected_record, 6u);
    memcpy(memory + previous + 0x20u, displaced_record, 6u);
    WriteU16(previous + 0x0au, 0x26u);
    WriteU24(previous + 0x15u, 0x040000ul);
    WriteU24(previous + 0x11u, 0x060000ul);

    ReconcileControllerBufferRecordsBackward(0u, 1u);

    assert(memcmp(memory + previous + 0x20u, selected_record, 6u) == 0);
    assert(rebuild_count == 1u);
    assert(rebuild_calls[0].group == 0u && rebuild_calls[0].page == 0u);
    assert(ReadU24(selected + 0x11u) == 0x050102ul);
    assert(transfer_count == 0u);
}

static void TestMultiPagePropagationShiftsPayloadAndUpdatesTriplets(void)
{
    static const u8 original_record[6] = { 0x05, 0, 0, 0x51, 0x52, 0x53 };
    static const u8 swap_displaced[6] = { 0x06, 0x10, 0, 0x61, 0x62, 0x63 };
    static const u8 middle_first[6] = { 0x03, 0x20, 0, 0x31, 0x32, 0x33 };
    static const u8 middle_last[6] = { 0x04, 0x30, 0, 0x41, 0x42, 0x43 };
    fw_addr_t original = Page(0u, 3u);
    fw_addr_t middle = Page(0u, 2u);
    fw_addr_t swap_page = Page(0u, 1u);

    ResetFixture();
    memcpy(memory + original + 0x20u, original_record, 6u);

    WriteU24(middle + 0x15u, 0x040000ul);
    WriteU24(middle + 0x11u, 0x030000ul);
    WriteU16(middle + 0x0au, 0x2cu);
    memcpy(memory + middle + 0x20u, middle_first, 6u);
    memcpy(memory + middle + 0x26u, middle_last, 6u);

    WriteU24(swap_page + 0x15u, 0x045000ul);
    WriteU24(swap_page + 0x11u, 0x060000ul);
    WriteU16(swap_page + 0x0au, 0x26u);
    memcpy(memory + swap_page + 0x20u, swap_displaced, 6u);

    ReconcileControllerBufferRecordsBackward(0u, 3u);

    assert(memcmp(memory + swap_page + 0x20u, original_record, 6u) == 0);
    assert(rebuild_count == 1u);
    assert(rebuild_calls[0].group == 0u && rebuild_calls[0].page == 1u);
    assert(transfer_count == 2u);
    assert(transfer_calls[0].first == 0x00095020ul);
    assert(transfer_calls[0].second == 0x000b4000ul);
    assert(transfer_calls[0].count == 3u);
    assert(transfer_calls[1].first == 0x000b4000ul);
    assert(transfer_calls[1].second == 0x00095026ul);
    assert(transfer_calls[1].count == 3u);
    assert(ReadU24(middle + 0x11u) == 0x032000ul);
    assert(ReadU24(middle + 0x15u) == 0x043000ul);
    assert(ReadU24(original + 0x11u) == 0x050000ul);
    assert(yield_count == 2u);
}

static void TestOutOfGridInputStopsAfterSixtyFiveEmptyPages(void)
{
    fw_addr_t selected = Page(8u, 7u);

    ResetFixture();
    WriteRecord(selected + 0x20u, 1u, 2u, 3u, 4u, 5u, 6u);
    memset(memory + selected + 0x11u, 0xa5, 3u);

    ReconcileControllerBufferRecordsBackward(8u, 7u);

    assert(memory[selected + 0x11u] == 0xa5u);
    assert(memory[selected + 0x12u] == 0xa5u);
    assert(memory[selected + 0x13u] == 0xa5u);
    assert(rebuild_count == 0u && transfer_count == 0u && yield_count == 0u);
}

static void TestForwardBoundarySwapRebuildsNextPage(void)
{
    static const u8 selected_record[6] = { 0x05, 1, 2, 0x13, 0x14, 0x15 };
    static const u8 displaced_record[6] = { 0x04, 2, 3, 0x23, 0x24, 0x25 };
    fw_addr_t selected = Page(0u, 0u);
    fw_addr_t next = Page(0u, 1u);

    ResetFixture();
    WriteU16(selected + 8u, 1u);
    WriteU16(selected + 0x0au, 0x26u);
    memcpy(memory + selected + 0x20u, selected_record, 6u);
    WriteU16(next + 8u, 1u);
    WriteU24(next + 0x11u, 0x040000ul);
    WriteU24(next + 0x15u, 0x030000ul);
    memcpy(memory + next + 0x20u, displaced_record, 6u);

    ReconcileControllerBufferRecordsForward(0u, 0u);

    assert(memcmp(memory + next + 0x20u, selected_record, 6u) == 0);
    assert(memcmp(memory + selected + 0x20u, displaced_record, 6u) == 0);
    assert(ReadU24(selected + 0x15u) == 0x040203ul);
    assert(rebuild_count == 1u);
    assert(rebuild_calls[0].group == 0u && rebuild_calls[0].page == 1u);
    assert(transfer_count == 0u && yield_count == 0u);
}

static void TestForwardMultiPagePropagationShiftsPayload(void)
{
    static const u8 original_record[6] = { 0x05, 0, 0, 0x51, 0x52, 0x53 };
    static const u8 middle_first[6] = { 0x03, 0x20, 0, 0x31, 0x32, 0x33 };
    static const u8 middle_last[6] = { 0x06, 0x30, 0, 0x61, 0x62, 0x63 };
    static const u8 swap_displaced[6] = { 0x04, 0x10, 0, 0x41, 0x42, 0x43 };
    fw_addr_t original = Page(0u, 0u);
    fw_addr_t middle = Page(0u, 1u);
    fw_addr_t swap_page = Page(0u, 2u);

    ResetFixture();
    WriteU16(original + 8u, 1u);
    WriteU16(original + 0x0au, 0x26u);
    memcpy(memory + original + 0x20u, original_record, 6u);

    WriteU16(middle + 8u, 2u);
    WriteU16(middle + 0x0au, 0x2cu);
    WriteU24(middle + 0x11u, 0x030000ul);
    WriteU24(middle + 0x15u, 0x060000ul);
    memcpy(memory + middle + 0x20u, middle_first, 6u);
    memcpy(memory + middle + 0x26u, middle_last, 6u);

    WriteU16(swap_page + 8u, 1u);
    WriteU24(swap_page + 0x11u, 0x040000ul);
    WriteU24(swap_page + 0x15u, 0x030000ul);
    memcpy(memory + swap_page + 0x20u, swap_displaced, 6u);

    ReconcileControllerBufferRecordsForward(0u, 0u);

    assert(memcmp(memory + swap_page + 0x20u, original_record, 6u) == 0);
    assert(memcmp(memory + middle + 0x26u, swap_displaced, 6u) == 0);
    assert(memcmp(memory + original + 0x20u, middle_first, 6u) == 0);
    assert(ReadU24(middle + 0x11u) == 0x032000ul);
    assert(ReadU24(middle + 0x15u) == 0x041000ul);
    assert(ReadU24(original + 0x15u) == 0x032000ul);
    assert(rebuild_count == 1u);
    assert(transfer_count == 1u);
    assert(transfer_calls[0].first == 0x00094826ul);
    assert(transfer_calls[0].second == 0x00094820ul);
    assert(transfer_calls[0].count == 3u);
    assert(yield_count == 2u);
}

int main(void)
{
    TestImmediateBoundaryExitRefreshesSelectedHeader();
    TestBoundarySwapRebuildsPreviousPage();
    TestMultiPagePropagationShiftsPayloadAndUpdatesTriplets();
    TestOutOfGridInputStopsAfterSixtyFiveEmptyPages();
    TestForwardBoundarySwapRebuildsNextPage();
    TestForwardMultiPagePropagationShiftsPayload();
    puts("backward controller-record page reconciliation tests passed");
    return 0;
}
