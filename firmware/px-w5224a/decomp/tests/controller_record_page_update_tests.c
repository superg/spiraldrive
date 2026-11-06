#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_page_update.h"
#include "platform/firmware_memory.h"

struct FindResult
{
    u8 mode;
    u32 value;
};

struct PageCall
{
    u32 first;
    u32 second;
    u16 count;
};

static u8 memory[0x1000000ul];
static struct FindResult find_results[80];
static struct PageCall copy_six_calls[512];
static struct PageCall dword_calls[8];
static unsigned find_result_count;
static unsigned find_count;
static unsigned copy_six_count;
static unsigned dword_count;
static unsigned rebuild_count;
static unsigned clear_count;
static unsigned backward_count;
static unsigned forward_count;
static unsigned compact_count;
static u8 last_group;
static u8 last_page;

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

u32 ObserveRecordPageUpdateFind(s32 key, u8 mode)
{
    (void)key;
    assert(find_count < find_result_count);
    assert(find_results[find_count].mode == mode);
    return find_results[find_count++].value;
}

void ObserveRecordPageUpdateCopyFrom(u32 source, fw_addr_t destination, u8 count)
{
    u8 index;

    for(index = 0u; index < count; ++index)
        memory[destination + index] = memory[source + 0x100000ul + index];
}

void ObserveRecordPageUpdateCopyTo(u32 destination, fw_addr_t source, u8 count)
{
    u8 index;

    for(index = 0u; index < count; ++index)
        memory[destination + 0x100000ul + index] = memory[source + index];
}

void ObserveRecordPageUpdateCopySix(u32 destination, u32 source)
{
    assert(copy_six_count < sizeof(copy_six_calls) / sizeof(copy_six_calls[0]));
    copy_six_calls[copy_six_count].first = destination;
    copy_six_calls[copy_six_count].second = source;
    copy_six_calls[copy_six_count].count = 6u;
    ++copy_six_count;
    memmove(memory + destination, memory + source, 6u);
}

void ObserveRecordPageUpdateCopyDwords(u32 destination, fw_addr_t source, u16 count)
{
    assert(dword_count < sizeof(dword_calls) / sizeof(dword_calls[0]));
    dword_calls[dword_count].first = destination;
    dword_calls[dword_count].second = source;
    dword_calls[dword_count].count = count;
    ++dword_count;
    memcpy(memory + destination + 0x100000ul, memory + source, (size_t)count * 4u);
}

void ObserveRecordPageUpdateCompact(u8 include_type_zero_and_one)
{
    assert(include_type_zero_and_one <= 1u);
    ++compact_count;
}

void ObserveRecordPageUpdateClear(u16 group, u16 page)
{
    ++clear_count;
    last_group = (u8)group;
    last_page = (u8)page;
}

void ObserveRecordPageUpdateRebuild(u8 group, u8 page)
{
    ++rebuild_count;
    last_group = group;
    last_page = page;
}

void ObserveRecordPageUpdateReconcileBackward(u8 group, u8 page)
{
    ++backward_count;
    last_group = group;
    last_page = page;
}

void ObserveRecordPageUpdateReconcileForward(u8 group, u8 page)
{
    ++forward_count;
    last_group = group;
    last_page = page;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void SetZoneTables(u8 selected_zone, s32 direct_start, s32 gap_start, s32 next_gap_start)
{
    u8 index;

    for(index = 0u; index <= 0x40u; ++index)
    {
        WriteU32(0xf84590ul + (u32)index * 4ul, 0x70000000ul);
        WriteU32(0xf84790ul + (u32)index * 4ul, 0x70000000ul);
    }
    WriteU32(0xf84590ul + (u32)selected_zone * 4ul, (u32)direct_start);
    WriteU32(0xf84790ul + (u32)selected_zone * 4ul, (u32)gap_start);
    WriteU32(0xf84790ul + (u32)(selected_zone + 1u) * 4ul, (u32)next_gap_start);
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(find_results, 0, sizeof(find_results));
    find_result_count = 0u;
    find_count = 0u;
    copy_six_count = 0u;
    dword_count = 0u;
    rebuild_count = 0u;
    clear_count = 0u;
    backward_count = 0u;
    forward_count = 0u;
    compact_count = 0u;
    last_group = 0xffu;
    last_page = 0xffu;
}

static void QueueFind(u8 mode, u32 value)
{
    find_results[find_result_count].mode = mode;
    find_results[find_result_count].value = value;
    ++find_result_count;
}

static void TestInvalidPreservationCountDoesNothing(void)
{
    ResetFixture();
    memory[0xc8bu] = 0x55u;
    UpdateControllerBufferRecordPageForFramePosition(100, 9u);
    assert(find_count == 0u && memory[0xc8bu] == 0x55u);
}

static void TestDirectZonePublishesWorkspaceWindow(void)
{
    ResetFixture();
    SetZoneTables(10u, 1000, 2000, 2100);
    QueueFind(0x30u, 0u);
    UpdateControllerBufferRecordPageForFramePosition(1000, 0u);
    assert(ReadU32(0x2ba51cu) == 0x305020ul);
    assert(ReadU32(0x2ba520u) == 0x305620ul);
    assert(find_count == 1u);
}

static void TestGapSearchInsertsOnNextPageAndReconcilesBackward(void)
{
    u32 next_page = 0x099000ul;
    u32 insertion = next_page + 0x20ul + 0x12ul;

    ResetFixture();
    SetZoneTables(9u, 4000, 500, 1000);
    QueueFind(0x20u, 0u);
    QueueFind(0x20u, insertion);
    UpdateControllerBufferRecordPageForFramePosition(600, 0u);
    assert(find_count == 2u);
    assert(memory[insertion + 0x100000ul] == 0u);
    assert(memory[insertion + 0x100001ul] == 2u);
    assert(memory[insertion + 0x100002ul] == 0x58u);
    assert(rebuild_count == 1u && clear_count == 1u);
    assert(backward_count == 1u && forward_count == 0u);
    assert(last_group == 1u && last_page == 2u);
}

static void TestGapSearchExhaustionSetsFailureBit(void)
{
    unsigned index;

    ResetFixture();
    SetZoneTables(9u, 4000, 500, 1000);
    for(index = 0u; index < 0x40u; ++index)
        QueueFind(0x20u, 0u);
    memory[0xc8bu] = 0x80u;
    UpdateControllerBufferRecordPageForFramePosition(600, 0u);
    assert(find_count == 0x40u);
    assert(memory[0xc8bu] == 0x82u);
    assert(rebuild_count == 0u && backward_count == 0u && forward_count == 0u);
}

static void QueueHeaderRebuildFinds(u32 page)
{
    QueueFind(0x31u, page + 0x80u);
    QueueFind(0x20u, page + 0x40u);
    QueueFind(0x31u, page + 0x90u);
    QueueFind(0x31u, page + 0x90u);
}

static void TestDirectZeroPreservationShiftsAndReinserts(void)
{
    u32 page = 0x300000ul;
    u32 found = page + 0x20ul + 12ul;
    u32 cpu_found = found + 0x100000ul;

    ResetFixture();
    SetZoneTables(0u, 1000, 2000, 2100);
    memory[cpu_found] = 0x20u;
    memory[cpu_found + 1u] = 1u;
    memory[cpu_found + 2u] = 2u;
    memory[cpu_found + 3u] = 3u;
    memory[cpu_found + 4u] = 4u;
    memory[cpu_found + 5u] = 5u;
    QueueFind(0x30u, found);
    QueueHeaderRebuildFinds(page);
    UpdateControllerBufferRecordPageForFramePosition(1000, 0u);
    assert(memory[0x2b971cu] == 0x30u);
    assert(copy_six_count == 254u);
    assert(compact_count == 1u);
    assert(find_count == 5u);
    assert(memory[0xfa12u] == 0u && memory[0xfa13u] == 0u);
}

static void TestEightPreservedGroupsUseWholeWorkspaceCopy(void)
{
    u32 page = 0x300000ul;
    u32 found = page + 0x20ul;
    u32 cpu_found = found + 0x100000ul;
    unsigned index;

    ResetFixture();
    SetZoneTables(0u, 1000, 2000, 2100);
    for(index = 0u; index < 256u; ++index)
        memory[cpu_found + index * 6u] = 0x20u;
    QueueFind(0x30u, found);
    QueueHeaderRebuildFinds(page);
    UpdateControllerBufferRecordPageForFramePosition(1000, 8u);
    assert(copy_six_count == 257u);
    assert(dword_count == 1u);
    assert(dword_calls[0].first == found);
    assert(dword_calls[0].second == 0x2b971cul);
    assert(dword_calls[0].count == 384u);
    assert(memory[0x2b971cu] == 0x30u);
}

static void TestPreservedGroupSkipsNonRecordAndCopiesRemainingBytes(void)
{
    u32 page = 0x300000ul;
    u32 found = page + 0x20ul;
    u32 cpu_found = found + 0x100000ul;
    unsigned index;

    ResetFixture();
    SetZoneTables(0u, 1000, 2000, 2100);
    for(index = 0u; index < 32u; ++index)
        memory[cpu_found + index * 6u] = 0x20u;
    memory[cpu_found + 5u * 6u] = 0x40u;
    QueueFind(0x30u, found);
    QueueHeaderRebuildFinds(page);
    UpdateControllerBufferRecordPageForFramePosition(1000, 1u);
    assert(memory[0x2b971cu] == 0x30u);
    assert(memory[0x2b971cu + 5u * 6u] == 0x40u);
    assert(copy_six_count == 257u);
    assert(dword_count == 1u);
    assert(dword_calls[0].first == page + 0x566u);
    assert(dword_calls[0].second == 0x2b9722ul);
    assert(dword_calls[0].count == 46u);
}

int main(void)
{
    TestInvalidPreservationCountDoesNothing();
    TestDirectZonePublishesWorkspaceWindow();
    TestGapSearchInsertsOnNextPageAndReconcilesBackward();
    TestGapSearchExhaustionSetsFailureBit();
    TestDirectZeroPreservationShiftsAndReinserts();
    TestEightPreservedGroupsUseWholeWorkspaceCopy();
    TestPreservedGroupSkipsNonRecordAndCopiesRemainingBytes();
    puts("controller record page update tests passed");
    return 0;
}
