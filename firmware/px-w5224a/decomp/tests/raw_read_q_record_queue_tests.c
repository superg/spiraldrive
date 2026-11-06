#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_q_record_queue.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x10000];
static u8 build_results[4];
static u8 wait_statuses[4];
static u16 wait_queued_counts[4];
static u8 build_result_count;
static u8 wait_result_count;
static u8 build_calls;
static u8 wait_calls;
static u8 restore_calls;
static u8 discard_calls;
static u8 restore_result;
static u8 discard_result;
static u8 report_calls;
static u8 report_code;
static u8 fill_calls;

static void WriteU16(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory_image[address] << 8) | memory_image[address + 1u]);
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void ResetHarness(void)
{
    memset(memory_image, 0xa5, sizeof(memory_image));
    memset(build_results, 0, sizeof(build_results));
    memset(wait_statuses, 0, sizeof(wait_statuses));
    memset(wait_queued_counts, 0, sizeof(wait_queued_counts));
    build_result_count = 0u;
    wait_result_count = 0u;
    build_calls = 0u;
    wait_calls = 0u;
    restore_calls = 0u;
    discard_calls = 0u;
    restore_result = 1u;
    discard_result = 1u;
    report_calls = 0u;
    report_code = 0u;
    fill_calls = 0u;
    WriteU32(0x156au, 16ul);
    WriteU32(0x1562u, 0x00123456ul);
    WriteU16(0x8010u, 5u);
    memory_image[0x0c44u] = 0u;
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

void ObserveQRecordQueueFill(fw_addr_t address, u32 count, u8 value)
{
    ++fill_calls;
    assert(address == 0ul);
    assert(count == 0xb7c0ul);
    assert(value == 0u);
}

u8 ObserveQRecordQueueBuild(void)
{
    u8 index = build_calls++;

    return index < build_result_count ? build_results[index] : 1u;
}

void ObserveQRecordQueueWait(void)
{
    u8 index = wait_calls++;

    if(index < wait_result_count)
    {
        memory_image[0xfba0u] = wait_statuses[index];
        WriteU16(0x1292u, wait_queued_counts[index]);
    }
}

u8 ObserveQRecordQueueRestore(u16 count)
{
    ++restore_calls;
    assert(count == 1u);
    return restore_result;
}

u8 ObserveQRecordQueueDiscard(u16 count)
{
    ++discard_calls;
    assert(count == 1u);
    return discard_result;
}

void ObserveQRecordQueueReportFailure(u8 code)
{
    ++report_calls;
    report_code = code;
}

static void CheckResetState(void)
{
    assert(ReadU16(0x1298u) == 0u);
    assert(ReadU16(0x1294u) == 0u);
    assert(ReadU16(0x1296u) == 0u);
    assert(ReadU16(0x1292u) == 0u);
    assert(ReadU16(0x129au) == 15u);
    assert(ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x1262u) == 0ul);
    assert(ReadU32(0x1266u) == 0ul);
    assert(ReadU32(0x126au) == 0x00123456ul);
    assert(ReadU32(0x126eu) == 0ul);
    assert(memory_image[0x127eu] == 0u);
    assert(memory_image[0x1490u] == 0u);
    assert(memory_image[0x128cu] == 0u);
    assert(memory_image[0x1285u] == 0u);
    assert(memory_image[0x1283u] == memory_image[0x0c44u]);
    assert(memory_image[0x127cu] == 0u);
    assert(memory_image[0x1281u] == 0u);
    assert(memory_image[0x1282u] == 0u);
    assert(fill_calls == 1u);
}

static void TestBuildFailurePolicyAndReset(void)
{
    ResetHarness();
    build_results[0] = 0u;
    build_result_count = 1u;
    assert(InitializeAndBuildRawReadQRecordQueue() == 0u);
    CheckResetState();
    assert(wait_calls == 0u);
    assert(report_calls == 0u);

    ResetHarness();
    memory_image[0x0c44u] = 2u;
    build_results[0] = 0u;
    build_result_count = 1u;
    assert(InitializeAndBuildRawReadQRecordQueue() == 1u);
    CheckResetState();
}

static void TestCompletionAndTerminalFailure(void)
{
    ResetHarness();
    wait_statuses[0] = 0u;
    wait_queued_counts[0] = 5u;
    wait_result_count = 1u;
    assert(InitializeAndBuildRawReadQRecordQueue() == 1u);
    assert(build_calls == 1u && wait_calls == 1u);
    assert(report_calls == 0u);

    ResetHarness();
    wait_statuses[0] = 2u;
    wait_result_count = 1u;
    assert(InitializeAndBuildRawReadQRecordQueue() == 0u);
    assert(report_calls == 1u && report_code == 3u);
}

static void TestQueueRepairRoutes(void)
{
    ResetHarness();
    wait_statuses[0] = 4u;
    wait_queued_counts[0] = 4u;
    wait_statuses[1] = 0u;
    wait_queued_counts[1] = 5u;
    wait_result_count = 2u;
    assert(InitializeAndBuildRawReadQRecordQueue() == 1u);
    assert(restore_calls == 1u);
    assert(discard_calls == 0u);

    ResetHarness();
    wait_statuses[0] = 4u;
    wait_queued_counts[0] = 3u;
    wait_statuses[1] = 0u;
    wait_queued_counts[1] = 5u;
    wait_result_count = 2u;
    assert(InitializeAndBuildRawReadQRecordQueue() == 1u);
    assert(discard_calls == 1u);
    assert(restore_calls == 1u);
}

static void TestTypeTwoShortfallAndRepairTolerance(void)
{
    ResetHarness();
    memory_image[0x0c44u] = 2u;
    wait_statuses[0] = 4u;
    wait_queued_counts[0] = 3u;
    wait_result_count = 1u;
    assert(InitializeAndBuildRawReadQRecordQueue() == 1u);
    assert(discard_calls == 0u && restore_calls == 0u);

    ResetHarness();
    memory_image[0x0c44u] = 2u;
    WriteU16(0x8010u, 5u);
    wait_statuses[0] = 4u;
    wait_queued_counts[0] = 4u;
    wait_result_count = 1u;
    restore_result = 0u;
    assert(InitializeAndBuildRawReadQRecordQueue() == 1u);
    assert(restore_calls == 1u && report_calls == 0u);
}

int main(void)
{
    TestBuildFailurePolicyAndReset();
    TestCompletionAndTerminalFailure();
    TestQueueRepairRoutes();
    TestTypeTwoShortfallAndRepairTolerance();
    puts("raw-read Q-record queue tests passed");
    return 0;
}
