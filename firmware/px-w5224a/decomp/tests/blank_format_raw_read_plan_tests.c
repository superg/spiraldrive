#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/blank_format_raw_read_plan.h"
#include "platform/firmware_memory.h"

typedef struct
{
    s32 start;
    u32 count;
    u8 mode;
} RangeCall;

static u8 memory_image[0x10000];
static RangeCall ranges[4];
static u8 range_results[4];
static u8 q_result;
static u8 reset_calls;
static u8 range_calls;
static u8 q_calls;

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static void ResetHarness(u8 operation)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(ranges, 0, sizeof(ranges));
    memset(range_results, 1, sizeof(range_results));
    q_result = 1u;
    reset_calls = 0u;
    range_calls = 0u;
    q_calls = 0u;
    memory_image[0xe600u] = operation;
    WriteU32(0x114fu, 4000ul);
    WriteU32(0x1153u, 50000ul);
    WriteU32(0xdd39u, 12000ul);
    WriteU32(0xdd41u, 15000ul);
    WriteU32(0xe602u, 10000ul);
    WriteU32(0xe606u, 22000ul);
    WriteU32(0xe612u, 1000ul);
    WriteU32(0xe616u, 1999ul);
    WriteU32(0xe61au, 3000ul);
    WriteU32(0xe61eu, 3999ul);
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

void ObserveBlankFormatPlanReset(void)
{
    ++reset_calls;
}

u8 ObserveBlankFormatPlanAcquire(s32 start_frame_position, u32 frame_count, u8 q_metadata_mode)
{
    u8 index = range_calls++;

    assert(index < 4u);
    ranges[index].start = start_frame_position;
    ranges[index].count = frame_count;
    ranges[index].mode = q_metadata_mode;
    return range_results[index];
}

u8 ObserveBlankFormatPlanQSequence(u8 first_record_index)
{
    ++q_calls;
    assert(first_record_index == 0u);
    return q_result;
}

static void AssertInitialState(u8 operation)
{
    assert(reset_calls == 1u);
    assert(memory_image[0x88b6u] == 1u);
    assert(memory_image[0x88b8u] == operation);
    assert(memory_image[0x1283u] == 1u);
}

static void TestUnknownOperationSucceedsWithoutRanges(void)
{
    ResetHarness(7u);
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    AssertInitialState(7u);
    assert(range_calls == 0u && q_calls == 0u);
}

static void TestOperationZeroLeadInPlan(void)
{
    ResetHarness(0u);
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(range_calls == 3u);
    assert(ranges[0].start == 3000l && ranges[0].count == 1000ul && ranges[0].mode == 0u);
    assert(ranges[1].start == 4000l && ranges[1].count == 0xfffff060ul && ranges[1].mode == 1u);
    assert(ranges[2].start == 0l && ranges[2].count == 52250ul && ranges[2].mode == 2u);
    assert(memory_image[0x88b9u] == 0u && memory_image[0x1281u] == 2u && memory_image[0x1285u] == 8u);
}

static void TestOperationOneFailureRouting(void)
{
    ResetHarness(1u);
    range_results[1] = 0u;
    assert(ExecuteBlankOrFormatRawReadPlan() == 0u);
    assert(range_calls == 2u);
    assert(ranges[0].start == 3000l && ranges[0].count == 1000ul);
    assert(ranges[1].start == 4000l && ranges[1].count == 0xfffff060ul);
    assert(memory_image[0x88b9u] == 0u);

    ResetHarness(1u);
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(ranges[2].start == 0l && ranges[2].count == 22002ul && ranges[2].mode == 2u);
}

static void TestOperationTwoAdjustment(void)
{
    ResetHarness(2u);
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(range_calls == 1u && ranges[0].start == 12000l && ranges[0].count == 3000ul && ranges[0].mode == 2u);

    ResetHarness(2u);
    memory_image[0xdd36u] = 0x10u;
    range_results[0] = 0u;
    assert(ExecuteBlankOrFormatRawReadPlan() == 0u);
    assert(ranges[0].start == 12145l && ranges[0].count == 2855ul);
}

static void TestOperationThreeQAndOptionalRange(void)
{
    ResetHarness(3u);
    q_result = 0u;
    assert(ExecuteBlankOrFormatRawReadPlan() == 0u);
    assert(q_calls == 1u && range_calls == 0u && memory_image[0x88b9u] == 0u);

    ResetHarness(3u);
    memory_image[0xdd34u] = 2u;
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(q_calls == 1u && range_calls == 0u);

    ResetHarness(3u);
    memory_image[0xdd34u] = 2u;
    memory_image[0xdd36u] = 0x10u;
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(range_calls == 1u && ranges[0].start == 12000l && ranges[0].count == 3000ul && ranges[0].mode == 2u);
}

static void TestOperationFourRange(void)
{
    ResetHarness(4u);
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(range_calls == 1u && ranges[0].start == 10145l && ranges[0].count == 4855ul && ranges[0].mode == 2u);
}

static void TestOperationFiveStoredRanges(void)
{
    ResetHarness(5u);
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(range_calls == 2u);
    assert(ranges[0].start == 1000l && ranges[0].count == 1000ul && ranges[0].mode == 1u);
    assert(ranges[1].start == 3000l && ranges[1].count == 1000ul && ranges[1].mode == 2u);
    assert(memory_image[0x1281u] == 3u);
}

static void TestOperationSixSelectorRoutes(void)
{
    ResetHarness(6u);
    memory_image[0xdd63u] = 3u;
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(q_calls == 0u && range_calls == 0u);

    ResetHarness(6u);
    memory_image[0xdd63u] = 1u;
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(q_calls == 1u && range_calls == 1u);
    assert(ranges[0].start == 2000l && ranges[0].count == 1999ul && ranges[0].mode == 2u);

    ResetHarness(6u);
    memory_image[0xdd63u] = 2u;
    assert(ExecuteBlankOrFormatRawReadPlan() == 1u);
    assert(q_calls == 1u && range_calls == 2u);
    assert(ranges[0].start == 1000l && ranges[0].count == 1000ul && ranges[0].mode == 1u);
    assert(ranges[1].start == 2000l && ranges[1].count == 1999ul && ranges[1].mode == 2u);
}

int main(void)
{
    TestUnknownOperationSucceedsWithoutRanges();
    TestOperationZeroLeadInPlan();
    TestOperationOneFailureRouting();
    TestOperationTwoAdjustment();
    TestOperationThreeQAndOptionalRange();
    TestOperationFourRange();
    TestOperationFiveStoredRanges();
    TestOperationSixSelectorRoutes();
    puts("blank/format raw-read plan tests passed");
    return 0;
}
