#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_index_range_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static fw_addr_t submitted_cursor[128];
static u8 submitted_parameter[128];
static u8 controller_mode[128];
static u8 reverse_values[128];
static unsigned submit_count;
static unsigned block_count;
static unsigned update_q_count;
static unsigned enqueue_count;
static unsigned advance_q_count;
static unsigned capacity_count;
static unsigned service_count;
static unsigned yield_count;
static unsigned clear_count;
static unsigned drain_count;
static unsigned refresh_count;
static u16 refreshed_first;
static u16 refreshed_last;
static fw_addr_t resume_cursor;
static u8 service_sets_abort;

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

static u16 ReadU16(fw_addr_t address)
{
    return ((u16)memory[address] << 8) | memory[address + 1u];
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u8 ToBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

static u8 FromBcd(u8 value)
{
    return (u8)((value >> 4) * 10u + (value & 0x0fu));
}

fw_addr_t ObserveRecordRangeResumeRing(void)
{
    return resume_cursor;
}

void ObserveRecordRangeConvert(s32 frame_position, fw_addr_t destination)
{
    u32 position = (u32)frame_position;

    memory[destination] = (u8)(position / (75ul * 60ul));
    memory[destination + 1u] = (u8)((position / 75ul) % 60ul);
    memory[destination + 2u] = (u8)(position % 75ul);
}

void ObserveRecordRangePackBcd(u16 tuple_address)
{
    memory[tuple_address] = ToBcd(memory[tuple_address]);
    memory[tuple_address + 1u] = ToBcd(memory[tuple_address + 1u]);
    memory[tuple_address + 2u] = ToBcd(memory[tuple_address + 2u]);
}

void ObserveRecordRangeCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    while(byte_count-- != 0u)
        memory[destination++] = memory[source++];
}

void ObserveRecordRangeWriteControllerBlock(fw_addr_t source)
{
    assert(source == 0x1428ul);
    ++block_count;
}

void ObserveRecordRangeSubmit(fw_addr_t cursor, u8 parameter)
{
    assert(submit_count < 128u);
    submitted_cursor[submit_count] = cursor;
    submitted_parameter[submit_count] = parameter;
    controller_mode[submit_count] = memory[0x018028u];
    ++submit_count;
}

void ObserveRecordRangeUpdateQ(void)
{
    ++update_q_count;
}

void ObserveRecordRangeEnqueue(void)
{
    ++enqueue_count;
}

void ObserveRecordRangeIncrement(u16 tuple_address)
{
    u32 position = ((u32)FromBcd(memory[tuple_address]) * 60ul + FromBcd(memory[tuple_address + 1u])) * 75ul + FromBcd(memory[tuple_address + 2u]) + 1ul;

    memory[tuple_address] = ToBcd((u8)(position / (75ul * 60ul)));
    memory[tuple_address + 1u] = ToBcd((u8)((position / 75ul) % 60ul));
    memory[tuple_address + 2u] = ToBcd((u8)(position % 75ul));
}

fw_addr_t ObserveRecordRangeAdvanceRing(fw_addr_t cursor)
{
    return cursor + 0x930ul;
}

void ObserveRecordRangeAdvanceQ(u8 reverse_relative)
{
    assert(advance_q_count < 128u);
    reverse_values[advance_q_count++] = reverse_relative;
}

void ObserveRecordRangeWaitForCapacity(void)
{
    ++capacity_count;
}

void ObserveRecordRangeServiceHost(void)
{
    ++service_count;
    if(service_sets_abort != 0u)
        memory[0x0c89u] |= 4u;
}

void ObserveRecordRangeYield(void)
{
    ++yield_count;
}

void ObserveRecordRangeClearCompletion(void)
{
    ++clear_count;
}

void ObserveRecordRangeWaitForRead(void)
{
    ++drain_count;
    WriteU16(0x1292u, (u16)(ReadU16(0x1292u) - 1u));
}

void ObserveRecordRangeRefresh(u16 first_index, u16 last_index)
{
    ++refresh_count;
    refreshed_first = first_index;
    refreshed_last = last_index;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(submitted_cursor, 0, sizeof(submitted_cursor));
    memset(submitted_parameter, 0, sizeof(submitted_parameter));
    memset(controller_mode, 0, sizeof(controller_mode));
    memset(reverse_values, 0, sizeof(reverse_values));
    submit_count = block_count = update_q_count = enqueue_count = 0u;
    advance_q_count = capacity_count = service_count = yield_count = 0u;
    clear_count = drain_count = refresh_count = 0u;
    refreshed_first = refreshed_last = 0u;
    resume_cursor = 0x2000ul;
    service_sets_abort = 0u;
    WriteU32(0x156aul, 16ul);
    WriteU32(0x1562ul, 0x00123456ul);
    memory[0xf84890u] = 1u;
    memory[0xf84891u] = 2u;
    memory[0xf84892u] = 3u;
    memory[0xf84893u] = 4u;
    memory[0xf84894u] = 5u;
    memory[0xf84895u] = 6u;
    memory[0xf84896u] = 7u;
}

static void TestOneCompleteIndexAndFrame150Transition(void)
{
    unsigned index;

    ResetFixture();
    WriteU16(0x1292u, 3u);
    assert(AcquireThirtyNineFrameControllerRecordIndexRange(0u, 0u, 3u) == 0u);
    assert(submit_count == 40u && block_count == 40u);
    assert(update_q_count == 40u && enqueue_count == 40u);
    assert(capacity_count == 1u && service_count == 1u && yield_count == 1u);
    assert(drain_count == 2u && refresh_count == 1u);
    assert(refreshed_first == 0u && refreshed_last == 0u);
    assert(ReadU32(0x1598u) == 185ul && ReadU32(0x8000u) == 145ul);
    assert(ReadU32(0x2ba59fu) == 184ul && memory[0x2ba5d8u] == 0u);
    assert(ReadU32(0x1455u) == 39ul);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x126au) == 0x00123456ul);
    for(index = 0u; index != 4u; ++index)
        assert(reverse_values[index] == 1u);
    assert(reverse_values[4] == 0u && reverse_values[39] == 1u);
    assert(memory[0x1275u] == 0u && memory[0x1276u] == 0u && memory[0x1277u] == 0u);
    assert(controller_mode[0] == 0x22u);
    assert(controller_mode[4] == 0xa2u);
    assert(controller_mode[37] == 0xc2u);
    assert(controller_mode[38] == 0xe2u);
    assert(controller_mode[39] == 0xa2u);
    for(index = 0u; index != submit_count; ++index)
    {
        assert(submitted_parameter[index] == 0xaau);
        assert(submitted_cursor[index] == resume_cursor + index * 0x930ul);
    }
}

static void TestServiceAbortShortensRefreshRange(void)
{
    ResetFixture();
    service_sets_abort = 1u;
    assert(AcquireThirtyNineFrameControllerRecordIndexRange(4u, 8u, 2u) == 1u);
    assert(submit_count == 40u && capacity_count == 1u);
    assert(refreshed_first == 4u && refreshed_last == 4u);
    assert((memory[0x0c89u] & 4u) != 0u);
    assert(ReadU32(0x1598u) == 341ul);
}

static void TestBitFourModeRouting(void)
{
    ResetFixture();
    memory[0x0c83u] = 0x10u;
    assert(AcquireThirtyNineFrameControllerRecordIndexRange(2u, 2u, 3u) == 0u);
    assert(clear_count == 1u && (memory[0x0c89u] & 4u) == 0u);

    ResetFixture();
    memory[0x0c83u] = 0x10u;
    assert(AcquireThirtyNineFrameControllerRecordIndexRange(2u, 5u, 1u) == 1u);
    assert(clear_count == 0u && refreshed_last == 2u);
}

static void TestSharedAbortLatchAndEmptyRange(void)
{
    ResetFixture();
    memory[0x2ba5b0u] = 1u;
    assert(AcquireThirtyNineFrameControllerRecordIndexRange(3u, 6u, 3u) == 1u);
    assert(memory[0x2ba5b1u] == 1u && refreshed_last == 3u);

    ResetFixture();
    assert(AcquireThirtyNineFrameControllerRecordIndexRange(7u, 6u, 3u) == 0u);
    assert(submit_count == 1u && capacity_count == 0u);
    assert(refreshed_first == 7u && refreshed_last == 6u);
}

int main(void)
{
    TestOneCompleteIndexAndFrame150Transition();
    TestServiceAbortShortensRefreshRange();
    TestBitFourModeRouting();
    TestSharedAbortLatchAndEmptyRange();
    puts("controller record index-range acquisition tests passed");
    return 0;
}
