#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/write_command_transfer_pipeline.h"
#include "platform/firmware_memory.h"

typedef struct
{
    u8 operation;
    u32 argument0;
    u32 argument1;
    u32 argument2;
} Event;

static u8 memory_image[0x1000000];
static Event events[512];
static u16 event_count;
static u32 results[17][8];
static u8 result_count[17];
static u8 result_index[17];

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void SetResults(u8 operation, const u32 *values, u8 count)
{
    u8 index;

    result_count[operation] = count;
    for(index = 0u; index < count; ++index)
        results[operation][index] = values[index];
}

static u16 CountEvents(u8 operation)
{
    u16 index;
    u16 count = 0u;

    for(index = 0u; index < event_count; ++index)
        if(events[index].operation == operation)
            ++count;
    return count;
}

static Event *FindEvent(u8 operation, u16 occurrence)
{
    u16 index;

    for(index = 0u; index < event_count; ++index)
    {
        if(events[index].operation == operation)
        {
            if(occurrence == 0u)
                return &events[index];
            --occurrence;
        }
    }
    assert(0);
    return &events[0];
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(events, 0, sizeof(events));
    memset(results, 0, sizeof(results));
    memset(result_count, 0, sizeof(result_count));
    memset(result_index, 0, sizeof(result_index));
    event_count = 0u;
    WriteDword(0x156au, 32ul);
    WriteDword(0x1562u, 0x22000ul);
    WriteDword(0x1572u, 0x40000ul);
    WriteDword(0x2ba5c2u, 0x40000ul);
    WriteDword(0x1598u, 150ul);
    WriteDword(0xfb4u, 0x12345678ul);
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

u32 ObserveWriteCommandPipelineOperation(u8 operation, u32 argument0, u32 argument1, u32 argument2)
{
    u32 result = 0ul;

    assert(operation < 17u && event_count < 512u);
    events[event_count].operation = operation;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    events[event_count].argument2 = argument2;
    ++event_count;
    if(result_index[operation] < result_count[operation])
        result = results[operation][result_index[operation]++];

    if(operation == WRITE_PIPELINE_START_TRANSFER)
        WriteDword(0x2ba5d4u, ReadDword(0x2ba5d4u) + (u8)argument0);
    if(operation == WRITE_PIPELINE_YIELD)
    {
        memory_image[0x1462u] = 0u;
        memory_image[0xfb9au] = 0u;
    }
    return result;
}

static void TestInitializeAndCompleteEmptyTransfer(void)
{
    static const u32 target[] = { 777ul };
    static const u32 success[] = { 1ul };

    ResetHarness();
    SetResults(WRITE_PIPELINE_INITIALIZE_HOST_RANGE, target, 1u);
    SetResults(WRITE_PIPELINE_BEGIN, success, 1u);
    WriteDword(0x1455u, 0ul);
    WriteDword(0x2ba5abu, 700ul);

    assert(ExecuteWriteCommandTransferPipeline(1u) == 1u);
    assert(CountEvents(WRITE_PIPELINE_CLEAR_BIT30) == 1u);
    assert(CountEvents(WRITE_PIPELINE_CLEAR_CONTROLLER_REGIONS) == 1u);
    assert(CountEvents(WRITE_PIPELINE_RESET_RAW_READ) == 1u);
    assert(CountEvents(WRITE_PIPELINE_INITIALIZE_RAW_READ) == 1u);
    assert(CountEvents(WRITE_PIPELINE_FILL_CONTROLLER_RANGE) == 1u);
    assert(FindEvent(WRITE_PIPELINE_FILL_CONTROLLER_RANGE, 0u)->argument0 == 0x40000ul);
    assert(FindEvent(WRITE_PIPELINE_FILL_CONTROLLER_RANGE, 0u)->argument1 == 0x24c00ul);
    assert(ReadDword(0x129eu) == 0x88bbul);
    assert(ReadDword(0x126au) == 0x22000ul);
    assert(ReadDword(0x1598u) == 777ul);
    assert(ReadDword(0x8000u) == 777ul);
    assert(ReadDword(0x2ba5b4u) == 777ul);
    assert(ReadDword(0x2ba5b8u) == 777ul);
    assert(ReadDword(0x140au) == 777ul);
    assert(ReadDword(0x2ba5abu) == 777ul);
    assert(ReadDword(0xfb0u) == 0x12345678ul);
}

static void TestResumeEmptyTransferBeginFailure(void)
{
    static const u32 target[] = { 400ul };
    static const u32 failure[] = { 0ul };

    ResetHarness();
    memory_image[0x127du] = 1u;
    SetResults(WRITE_PIPELINE_INITIALIZE_HOST_RANGE, target, 1u);
    SetResults(WRITE_PIPELINE_BEGIN, failure, 1u);
    assert(ExecuteWriteCommandTransferPipeline(0u) == 0u);
    assert(memory_image[0x127du] == 0u);
    assert(memory_image[0x127eu] == 0u);
    assert(ReadDword(0x1598u) == 400ul);
    assert(CountEvents(WRITE_PIPELINE_CLEAR_CONTROLLER_REGIONS) == 0u);
}

static void TestOrdinaryPartialBatchCompletes(void)
{
    static const u32 target[] = { 150ul };
    static const u32 success[] = { 1ul };

    ResetHarness();
    WriteDword(0x1455u, 2ul);
    SetResults(WRITE_PIPELINE_INITIALIZE_HOST_RANGE, target, 1u);
    SetResults(WRITE_PIPELINE_BEGIN, success, 1u);

    assert(ExecuteWriteCommandTransferPipeline(0u) == 1u);
    assert(CountEvents(WRITE_PIPELINE_START_TRANSFER) == 1u);
    assert(FindEvent(WRITE_PIPELINE_START_TRANSFER, 0u)->argument0 == 2ul);
    assert(FindEvent(WRITE_PIPELINE_START_TRANSFER, 0u)->argument1 == 0x40000ul);
    assert(ReadDword(0x2ba5d4u) == 2ul);
    assert(ReadDword(0x1598u) == 152ul);
    assert(CountEvents(WRITE_PIPELINE_POLL_BATCH) == 0u);
}

static void TestPreparationFailureAfterSplitSetup(void)
{
    static const u32 target[] = { 177ul };
    static const u32 failure[] = { 1ul };

    ResetHarness();
    WriteDword(0x1455u, 40ul);
    memory_image[0x18050u] = 1u;
    memory_image[0x127du] = 1u;
    SetResults(WRITE_PIPELINE_INITIALIZE_HOST_RANGE, target, 1u);
    SetResults(WRITE_PIPELINE_FAIL_PREPARATION, failure, 1u);

    assert(ExecuteWriteCommandTransferPipeline(0u) == 0u);
    assert(memory_image[0x2ba5ddu] == 1u);
    assert(memory_image[0x2ba5deu] == 1u);
    assert(memory_image[0x2ba5d1u] == 5u);
    assert(memory_image[0x2ba5d2u] == 5u);
    assert(memory_image[0x2ba5c6u] == 5u);
    assert(ReadDword(0x2ba5d4u) == 5ul);
}

static void TestExactBatchPollAndRange(void)
{
    static const u32 target[] = { 150ul };
    static const u32 poll[] = { 0xfeul, 1ul };
    static const u32 success[] = { 1ul };

    ResetHarness();
    WriteDword(0x1455u, 32ul);
    memory_image[0x18050u] = 1u;
    SetResults(WRITE_PIPELINE_INITIALIZE_HOST_RANGE, target, 1u);
    SetResults(WRITE_PIPELINE_POLL_BATCH, poll, 2u);
    SetResults(WRITE_PIPELINE_EXECUTE_RANGE, success, 1u);
    SetResults(WRITE_PIPELINE_BEGIN, success, 1u);

    assert(ExecuteWriteCommandTransferPipeline(0u) == 1u);
    assert(CountEvents(WRITE_PIPELINE_POLL_BATCH) == 2u);
    assert(CountEvents(WRITE_PIPELINE_EXECUTE_RANGE) == 1u);
    assert(CountEvents(WRITE_PIPELINE_YIELD) >= 2u);
    assert(ReadDword(0x1598u) == 150ul);
}

static void TestPrefixTableRestart(void)
{
    static const u32 target[] = { 151ul };
    static const u32 one[] = { 1ul };

    ResetHarness();
    WriteDword(0x1455u, 40ul);
    WriteDword(0xf84490u + 12u, 156ul);
    WriteDword(0xf84690u + 12u, 900ul);
    SetResults(WRITE_PIPELINE_INITIALIZE_HOST_RANGE, target, 1u);
    SetResults(WRITE_PIPELINE_POLL_BATCH, one, 1u);
    SetResults(WRITE_PIPELINE_EXECUTE_RANGE, one, 1u);
    SetResults(WRITE_PIPELINE_PREPARE_PREFIX, one, 1u);

    assert(ExecuteWriteCommandTransferPipeline(0u) == 1u);
    assert(CountEvents(WRITE_PIPELINE_PREPARE_PREFIX) == 1u);
    assert(CountEvents(WRITE_PIPELINE_START_TRANSFER) == 2u);
    assert(FindEvent(WRITE_PIPELINE_START_TRANSFER, 0u)->argument0 == 31ul);
    assert(FindEvent(WRITE_PIPELINE_START_TRANSFER, 1u)->argument0 == 9ul);
    assert(ReadDword(0x1598u) == 900ul);
    assert(ReadDword(0x8000u) == 900ul);
    assert(ReadDword(0x2ba5b4u) == 900ul);
    assert(memory_image[0x2ba5c9u] == 1u);
    assert(CountEvents(WRITE_PIPELINE_SET_NO_SENSE) == 1u);
}

int main(void)
{
    TestInitializeAndCompleteEmptyTransfer();
    TestResumeEmptyTransferBeginFailure();
    TestOrdinaryPartialBatchCompletes();
    TestPreparationFailureAfterSplitSetup();
    TestExactBatchPollAndRange();
    TestPrefixTableRestart();
    puts("write-command transfer pipeline tests passed");
    return 0;
}
