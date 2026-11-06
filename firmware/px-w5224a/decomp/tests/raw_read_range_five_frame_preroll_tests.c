#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_range_five_frame_preroll.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x2000];
static u8 plan_results[32];
static u8 wait_results[32];
static u8 capture_results[32];
static u8 prefix_results[32];
static u8 plan_result_count;
static u8 wait_result_count;
static u8 capture_result_count;
static u8 prefix_result_count;
static u8 plan_call_count;
static u8 wait_call_count;
static u8 capture_call_count;
static u8 prefix_call_count;
static u8 resume_call_count;
static u8 populate_call_count;
static u8 update_q_call_count;
static u8 enqueue_call_count;
static u8 advance_msf_call_count;
static u8 advance_ring_call_count;
static u8 report_call_count;
static u8 reported_failure;
static fw_addr_t capture_addresses[32];
static fw_addr_t populate_addresses[32];
static fw_addr_t advance_ring_arguments[32];
static u8 prefix_omit[32];
static u8 prefix_wait[32];

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

static u8 NextResult(const u8 *results, u8 count, u8 index)
{
    return index < count ? results[index] : 1u;
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(plan_results, 0, sizeof(plan_results));
    memset(wait_results, 0, sizeof(wait_results));
    memset(capture_results, 0, sizeof(capture_results));
    memset(prefix_results, 0, sizeof(prefix_results));
    memset(capture_addresses, 0, sizeof(capture_addresses));
    memset(populate_addresses, 0, sizeof(populate_addresses));
    memset(advance_ring_arguments, 0, sizeof(advance_ring_arguments));
    memset(prefix_omit, 0, sizeof(prefix_omit));
    memset(prefix_wait, 0, sizeof(prefix_wait));
    plan_result_count = 0u;
    wait_result_count = 0u;
    capture_result_count = 0u;
    prefix_result_count = 0u;
    plan_call_count = 0u;
    wait_call_count = 0u;
    capture_call_count = 0u;
    prefix_call_count = 0u;
    resume_call_count = 0u;
    populate_call_count = 0u;
    update_q_call_count = 0u;
    enqueue_call_count = 0u;
    advance_msf_call_count = 0u;
    advance_ring_call_count = 0u;
    report_call_count = 0u;
    reported_failure = 0u;
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

u8 ObserveFiveFramePreparePlan(u8 alternate_plan)
{
    u8 index = plan_call_count++;

    assert(alternate_plan == 0u);
    return NextResult(plan_results, plan_result_count, index);
}

fw_addr_t ObserveFiveFrameResumeRing(void)
{
    ++resume_call_count;
    return 0x002000ul;
}

u8 ObserveFiveFrameWaitForCapacity(void)
{
    u8 index = wait_call_count++;

    return NextResult(wait_results, wait_result_count, index);
}

u8 ObserveFiveFrameCapture(fw_addr_t address, u8 preserve_main, u8 clear_subchannel)
{
    u8 index = capture_call_count++;

    assert(preserve_main == 1u);
    assert(clear_subchannel == 1u);
    capture_addresses[index] = address;
    return NextResult(capture_results, capture_result_count, index);
}

void ObserveFiveFramePopulateSubheader(fw_addr_t address, u8 source_kind)
{
    assert(source_kind == 4u);
    populate_addresses[populate_call_count++] = address;
}

void ObserveFiveFrameUpdateQ(u8 skip_identification, u8 decrement_relative)
{
    assert(skip_identification == 0u);
    assert(decrement_relative == 0u);
    ++update_q_call_count;
}

void ObserveFiveFrameEnqueue(void)
{
    ++enqueue_call_count;
}

void ObserveFiveFrameAdvanceMsf(u8 reverse_relative)
{
    assert(reverse_relative == 0u);
    ++advance_msf_call_count;
}

fw_addr_t ObserveFiveFrameAdvanceRing(fw_addr_t cursor)
{
    advance_ring_arguments[advance_ring_call_count++] = cursor;
    return cursor + 0x930ul;
}

u8 ObserveFiveFramePreparePrefix(u8 omit_final_prefix, u8 wait_for_completion)
{
    u8 index = prefix_call_count++;

    prefix_omit[index] = omit_final_prefix;
    prefix_wait[index] = wait_for_completion;
    return NextResult(prefix_results, prefix_result_count, index);
}

void ObserveFiveFrameReportFailure(u8 failure_code)
{
    ++report_call_count;
    reported_failure = failure_code;
}

static void TestOnePacketHappyPath(void)
{
    u8 index;

    ResetHarness();
    WriteU32(0x0c4bu, 5ul);
    memory_image[0x0c46u] = 2u;
    memory_image[0x14b2u] = 0xa5u;
    WriteU32(0x14b8u, 0x12345678ul);

    assert(AcquireRawReadRangeWithFiveFramePreroll(100l, 0ul) == 1u);
    assert(ReadU32(0x1598u) == 100ul);
    assert(plan_call_count == 1u);
    assert(resume_call_count == 1u);
    assert(wait_call_count == 5u);
    assert(capture_call_count == 5u);
    assert(populate_call_count == 5u);
    assert(update_q_call_count == 5u);
    assert(enqueue_call_count == 5u);
    assert(advance_msf_call_count == 5u);
    assert(advance_ring_call_count == 5u);
    for(index = 0u; index != 5u; ++index)
    {
        fw_addr_t expected = 0x2000ul + (u32)index * 0x930ul;

        assert(capture_addresses[index] == expected);
        assert(populate_addresses[index] == expected);
        assert(advance_ring_arguments[index] == expected);
    }
    assert(prefix_call_count == 3u);
    assert(prefix_omit[0] == 1u && prefix_wait[0] == 0u);
    assert(prefix_omit[1] == 0u && prefix_wait[1] == 0u);
    assert(prefix_omit[2] == 1u && prefix_wait[2] == 1u);
    assert(memory_image[0x14b2u] == 0u);
    assert(ReadU32(0x14b8u) == 0ul);
    assert(memory_image[0x14b4u] == 0u);
    assert(memory_image[0x14b5u] == 1u);
    assert(memory_image[0x14b6u] == 0u);
    assert(memory_image[0x14b7u] == 0u);
    assert(report_call_count == 0u);
}

static void TestTenPlanFailuresStopWithoutPipelineReport(void)
{
    ResetHarness();
    memset(plan_results, 0, 10u);
    plan_result_count = 10u;
    WriteU32(0x0c4bu, 1ul);

    assert(AcquireRawReadRangeWithFiveFramePreroll(100l, 0ul) == 0u);
    assert(plan_call_count == 10u);
    assert(ReadU32(0x1598u) == 95ul);
    assert(memory_image[0x14b6u] == 1u);
    assert(memory_image[0x14b7u] == 1u);
    assert(resume_call_count == 0u);
    assert(report_call_count == 0u);
}

static void TestCapacityFailureRestartsFromSavedTarget(void)
{
    ResetHarness();
    WriteU32(0x0c4bu, 5ul);
    wait_results[0] = 0u;
    wait_result_count = 1u;

    assert(AcquireRawReadRangeWithFiveFramePreroll(100l, 0ul) == 1u);
    assert(plan_call_count == 2u);
    assert(resume_call_count == 2u);
    assert(wait_call_count == 6u);
    assert(capture_call_count == 5u);
    assert(ReadU32(0x1598u) == 100ul);
    assert(report_call_count == 0u);
}

static void TestPacketPrefixFailureRepeatsPacket(void)
{
    ResetHarness();
    WriteU32(0x0c4bu, 5ul);
    prefix_results[0] = 0u;
    prefix_result_count = 1u;

    assert(AcquireRawReadRangeWithFiveFramePreroll(100l, 0ul) == 1u);
    assert(plan_call_count == 2u);
    assert(capture_call_count == 10u);
    assert(prefix_call_count == 4u);
    assert(prefix_omit[0] == 1u && prefix_wait[0] == 0u);
    assert(prefix_omit[1] == 1u && prefix_wait[1] == 0u);
    assert(prefix_omit[2] == 0u && prefix_wait[2] == 0u);
    assert(prefix_omit[3] == 1u && prefix_wait[3] == 1u);
    assert(ReadU32(0x1598u) == 100ul);
}

static void TestTerminalPrefixFailures(void)
{
    ResetHarness();
    prefix_results[0] = 0u;
    prefix_result_count = 1u;
    assert(AcquireRawReadRangeWithFiveFramePreroll(3l, 0ul) == 0u);
    assert(prefix_call_count == 1u);
    assert(prefix_omit[0] == 0u && prefix_wait[0] == 0u);
    assert(memory_image[0x14b5u] == 0u);

    ResetHarness();
    prefix_results[0] = 1u;
    prefix_results[1] = 0u;
    prefix_result_count = 2u;
    assert(AcquireRawReadRangeWithFiveFramePreroll(3l, 0ul) == 0u);
    assert(prefix_call_count == 2u);
    assert(prefix_omit[1] == 1u && prefix_wait[1] == 1u);
    assert(memory_image[0x14b5u] == 1u);
}

int main(void)
{
    TestOnePacketHappyPath();
    TestTenPlanFailuresStopWithoutPipelineReport();
    TestCapacityFailureRestartsFromSavedTarget();
    TestPacketPrefixFailureRepeatsPacket();
    TestTerminalPrefixFailures();
    puts("raw-read five-frame-preroll range tests passed");
    return 0;
}
