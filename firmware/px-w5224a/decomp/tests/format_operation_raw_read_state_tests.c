#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/format_operation_raw_read_state.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x10000];
static u8 prepare_result, poll_result, preroll_result, q_result;
static u8 queue_result, lead_result, mode2_result;
static u8 reset_calls, init_calls, clear_calls, prepare_calls, poll_calls;
static u8 report_calls, report_code, sense_calls, preroll_calls, q_calls;
static u8 queue_calls, lead_calls, mode2_calls;
static s32 preroll_start, mode2_start;
static u32 preroll_count, mode2_count;

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
    memset(memory_image, 0, sizeof(memory_image));
    prepare_result = poll_result = preroll_result = q_result = 1u;
    queue_result = lead_result = mode2_result = 1u;
    reset_calls = init_calls = clear_calls = prepare_calls = poll_calls = 0u;
    report_calls = report_code = sense_calls = preroll_calls = q_calls = 0u;
    queue_calls = lead_calls = mode2_calls = 0u;
    preroll_start = mode2_start = 0;
    preroll_count = mode2_count = 0u;
    WriteU32(0x156au, 16ul);
    WriteU32(0x1562u, 0x00123456ul);
    WriteU32(0x0c4bu, 10ul);
    WriteU32(0xe5fcu, 20ul);
    memory_image[0x0c44u] = 2u;
    memory_image[0x0c48u] = 11u;
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

void ObserveFormatRawResetPipeline(void)
{
    ++reset_calls;
}
void ObserveFormatRawInitializeSectorControl(void)
{
    ++init_calls;
}
void ObserveFormatRawClearPipelineStatus(void)
{
    ++clear_calls;
}
u8 ObserveFormatRawPreparePlan(u8 alternate)
{
    ++prepare_calls;
    assert(alternate == 0u);
    return prepare_result;
}
u8 ObserveFormatRawPoll(u8 count)
{
    ++poll_calls;
    assert(count == 1u);
    return poll_result;
}
void ObserveFormatRawReportFailure(u8 code)
{
    ++report_calls;
    report_code = code;
}
void ObserveFormatRawReportQueueSense(void)
{
    ++sense_calls;
}
u32 ObserveFormatRawDivide(u32 dividend, u32 divisor)
{
    assert(divisor != 0u);
    return dividend / divisor;
}
u32 ObserveFormatRawMultiply(u32 left, u32 right)
{
    return left * right;
}
u8 ObserveFormatRawAcquirePreroll(s32 start, u32 count)
{
    ++preroll_calls;
    preroll_start = start;
    preroll_count = count;
    return preroll_result;
}
u8 ObserveFormatRawExecuteQSequence(u8 first_index)
{
    ++q_calls;
    assert(first_index == 1u);
    return q_result;
}
u8 ObserveFormatRawInitializeQQueue(void)
{
    ++queue_calls;
    return queue_result;
}
u8 ObserveFormatRawAcquireLeadSequences(void)
{
    ++lead_calls;
    return lead_result;
}
u8 ObserveFormatRawAcquireMode2Range(s32 start, u32 count)
{
    ++mode2_calls;
    mode2_start = start;
    mode2_count = count;
    return mode2_result;
}

static void CheckCommonReset(void)
{
    assert(reset_calls == 1u && init_calls == 1u && clear_calls == 1u);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x126au) == 0x00123456ul);
    assert(memory_image[0x1283u] == 2u);
}

static void TestOrdinaryTenRecordPath(void)
{
    ResetHarness();
    assert(PrepareFormatOperationRawReadState() == 1u);
    CheckCommonReset();
    assert(prepare_calls == 1u && poll_calls == 1u);
    assert(ReadU32(0x1598u) == 150ul);
    assert(preroll_calls == 1u && preroll_start == 150l && preroll_count == 29ul);
    assert(q_calls == 1u && queue_calls == 0u && lead_calls == 1u);
    assert(ReadU16(0x8010u) == 10u);
    assert(memory_image[0x1285u] == 11u);
    assert(memory_image[0x88b7u] == 0u && memory_image[0x88b9u] == 0u);
}

static void TestQueueAndWorkingTrackMode2Paths(void)
{
    ResetHarness();
    memory_image[0xe5fau] = 1u;
    WriteU32(0xe616u, 1000ul);
    assert(PrepareFormatOperationRawReadState() == 1u);
    assert(preroll_start == 1151l && queue_calls == 1u && q_calls == 0u);

    ResetHarness();
    memory_image[0xe5fbu] = 1u;
    memory_image[0x0c45u] = 0xc0u;
    WriteU32(0xdd3du, 1000ul);
    WriteU32(0xdd41u, 1100ul);
    WriteU32(0xe61eu, 2000ul);
    WriteU32(0x1153u, 5000ul);
    assert(PrepareFormatOperationRawReadState() == 1u);
    assert(prepare_calls == 0u && poll_calls == 0u);
    assert(preroll_start == 1105l);
    assert(preroll_count == (u32)(29ul - 105ul));
    assert(q_calls == 1u && mode2_calls == 1u);
    assert(mode2_start == 2001l && mode2_count == 5251ul);
}

static void TestPreparationAndPollFailures(void)
{
    ResetHarness();
    prepare_result = 0u;
    assert(PrepareFormatOperationRawReadState() == 0u);
    assert(memory_image[0x88b7u] == 0u && preroll_calls == 0u);

    ResetHarness();
    poll_result = 0u;
    memory_image[0x0c84u] = 0x40u;
    assert(PrepareFormatOperationRawReadState() == 0u);
    assert(report_calls == 1u && report_code == 8u && sense_calls == 0u);

    ResetHarness();
    poll_result = 0u;
    WriteU16(0x1292u, 0u);
    memory_image[0xfba0u] = 0u;
    assert(PrepareFormatOperationRawReadState() == 0u);
    assert(sense_calls == 1u && report_calls == 0u);
}

static void TestDownstreamFailuresClearActiveFlags(void)
{
    ResetHarness();
    q_result = 0u;
    assert(PrepareFormatOperationRawReadState() == 0u);
    assert(memory_image[0x88b7u] == 0u && memory_image[0x88b9u] == 0u);

    ResetHarness();
    lead_result = 0u;
    assert(PrepareFormatOperationRawReadState() == 0u);
    assert(memory_image[0x88b7u] == 0u);

    ResetHarness();
    memory_image[0xe5fbu] = 1u;
    memory_image[0x0c45u] = 0xc0u;
    WriteU32(0xdd3du, 100ul);
    WriteU32(0xdd41u, 120ul);
    WriteU32(0xe61eu, 200ul);
    WriteU32(0x1153u, 300ul);
    mode2_result = 0u;
    assert(PrepareFormatOperationRawReadState() == 0u);
    assert(memory_image[0x88b7u] == 0u);
}

int main(void)
{
    TestOrdinaryTenRecordPath();
    TestQueueAndWorkingTrackMode2Paths();
    TestPreparationAndPollFailures();
    TestDownstreamFailuresClearActiveFlags();
    puts("format-operation raw-read state tests passed");
    return 0;
}
