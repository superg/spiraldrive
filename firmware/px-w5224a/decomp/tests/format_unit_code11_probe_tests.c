#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/format_unit_code11_probe.h"

static u8 memory[0x1000000ul];
static u8 poll_results[32];
static s32 map_results[32];
static unsigned poll_result_count;
static unsigned poll_count;
static unsigned map_result_count;
static unsigned map_count;
static unsigned reset_count;
static unsigned initialize_count;
static unsigned yield_count;
static unsigned receive_count;
static unsigned reset_target_count;
static s32 last_reset_target;
static unsigned update_count;
static s32 last_update_position;
static unsigned bit_clear_count;
static unsigned bit_set_count;

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

void FirmwareWriteBit0580(u8 value)
{
    value != 0u ? ++bit_set_count : ++bit_clear_count;
}

void FirmwareWriteBit0581(u8 value)
{
    value != 0u ? ++bit_set_count : ++bit_clear_count;
}

void ObserveFormat11ProbeResetRawRead(void)
{
    ++reset_count;
}

s32 ObserveFormat11ProbeMap(s32 logical_position)
{
    (void)logical_position;
    assert(map_count < map_result_count);
    return map_results[map_count++];
}

void ObserveFormat11ProbeInitializeCoroutine2(u32 entry)
{
    assert(entry == 0x00f90000ul);
    ++initialize_count;
}

void ObserveFormat11ProbeYield(void)
{
    ++yield_count;
    memory[0xe40u] = 0u;
}

void ObserveFormat11ProbeReceiveHostPacket(void)
{
    ++receive_count;
}

u8 ObserveFormat11ProbePoll(void)
{
    assert(poll_count < poll_result_count);
    return poll_results[poll_count++];
}

void ObserveFormat11ProbeResetForTarget(s32 target, u8 mode)
{
    assert(mode == 1u);
    ++reset_target_count;
    last_reset_target = target;
}

void ObserveFormat11ProbeUpdateRecordPage(s32 position, u8 count)
{
    assert(count == 1u);
    ++update_count;
    last_update_position = position;
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

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(poll_results, 0, sizeof(poll_results));
    memset(map_results, 0, sizeof(map_results));
    poll_result_count = poll_count = 0u;
    map_result_count = map_count = 0u;
    reset_count = initialize_count = yield_count = receive_count = 0u;
    reset_target_count = update_count = 0u;
    last_reset_target = last_update_position = 0;
    bit_clear_count = bit_set_count = 0u;
    WriteU32(0x2ba52eu, 100000u);
    WriteU32(0xf84590u, 1000u);
    WriteU32(0xfaffu, 0x00020000ul);
    WriteU32(0xfb4u, 0x12345678ul);
}

static void TestEmptyAndBoundedSegmentSuccess(void)
{
    ResetFixture();
    memory[0xfa07u] = 0xa5u;
    memory[0xc8bu] = 0x80u;
    assert(ProbeFormatUnitCode11MediaRanges() == 1u);
    assert(memory[0xfa07u] == 0u && memory[0xc8bu] == 0x84u);
    assert(poll_count == 0u);

    ResetFixture();
    memory[0x2ba588u] = 1u;
    WriteU32(0x2ba52eu, 0x4e8u);
    assert(ProbeFormatUnitCode11MediaRanges() == 1u);
    assert(poll_count == 0u && memory[0xc8bu] == 4u);
}

static void TestEightSlicesAndOptionalPortRestore(void)
{
    unsigned index;

    ResetFixture();
    memory[0x2ba588u] = 1u;
    memory[0xf7c01fu] = 1u;
    for(index = 0u; index < 8u; ++index)
    {
        poll_results[poll_result_count++] = 3u;
        map_results[map_result_count++] = 5000 + (s32)index;
    }
    assert(ProbeFormatUnitCode11MediaRanges() == 1u);
    assert(poll_count == 8u && receive_count == 8u);
    assert(map_count == 8u && reset_count == 9u && initialize_count == 8u);
    assert(bit_clear_count == 16u && bit_set_count == 16u);
    assert(memory[0x1226u] == 4u && memory[0x1037u] == 0u);
    assert(ReadU32(0x15dcu) == 1224u);
}

static void TestPendingSuccessBusyWaitAndRecordRefresh(void)
{
    ResetFixture();
    memory[0x2ba588u] = 1u;
    poll_results[poll_result_count++] = 0xfeu;
    poll_results[poll_result_count++] = 0u;
    map_results[map_result_count++] = 5000;
    map_results[map_result_count++] = 0x91 + 39 * 3;
    memory[0xe40u] = 1u;
    memory[0x20003u] = 1u;
    WriteU32(0x2bc6f7u, 0x100u);
    poll_results[poll_result_count++] = 0xffu;
    map_results[map_result_count++] = 5001;
    assert(ProbeFormatUnitCode11MediaRanges() == 0u);
    assert(receive_count == 3u && poll_count == 3u);
    assert(update_count == 1u && last_update_position == 1000);
    assert(ReadU32(0x2bc6f7u) == 0x120u);
    assert(reset_target_count == 2u && last_reset_target == 0);
    assert(memory[0xc88u] == 0x80u && memory[0xc8bu] == 0u);
}

static void TestFailureUsesLatestAlternateResetTarget(void)
{
    ResetFixture();
    memory[0x2ba588u] = 1u;
    poll_results[poll_result_count++] = 7u;
    poll_results[poll_result_count++] = 0xffu;
    map_results[map_result_count++] = 5000;
    map_results[map_result_count++] = 5001;
    assert(ProbeFormatUnitCode11MediaRanges() == 0u);
    assert(reset_target_count == 1u && last_reset_target == 1000);
    assert(memory[0xc88u] == 0x80u && memory[0xc8bu] == 0u);
}

int main(void)
{
    TestEmptyAndBoundedSegmentSuccess();
    TestEightSlicesAndOptionalPortRestore();
    TestPendingSuccessBusyWaitAndRecordRefresh();
    TestFailureUsesLatestAlternateResetTarget();
    puts("format unit code-11 probe tests passed");
    return 0;
}
