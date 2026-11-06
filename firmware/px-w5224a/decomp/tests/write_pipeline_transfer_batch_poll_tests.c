#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/write_pipeline_transfer_batch_poll.h"
#include "platform/firmware_memory.h"

typedef struct
{
    u8 operation;
    u32 argument0;
    u32 argument1;
    u32 argument2;
} Event;

static u8 memory_image[0x1000000];
static Event events[1024];
static u16 event_count;
static u32 operation_results[24][8];
static u8 operation_result_count[24];
static u8 operation_result_index[24];
static fw_addr_t resumed_ring_address;

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

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

static void SetResults(u8 operation, const u32 *results, u8 count)
{
    u8 index;

    assert(operation < 24u && count <= 8u);
    operation_result_count[operation] = count;
    for(index = 0u; index < count; ++index)
        operation_results[operation][index] = results[index];
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
    memset(operation_results, 0, sizeof(operation_results));
    memset(operation_result_count, 0, sizeof(operation_result_count));
    memset(operation_result_index, 0, sizeof(operation_result_index));
    event_count = 0u;
    resumed_ring_address = 0x2000ul;
    WriteDword(0x1598u, 1000ul);
    WriteDword(0x156au, 64ul);
    WriteDword(0x1572u, 0x10000ul);
    memory_image[0x2ba5c6u] = 4u;
    memory_image[0x2ba5d2u] = 5u;
    memory_image[0x1279u] = 1u;
    memory_image[0x127au] = 2u;
    memory_image[0x127bu] = 3u;
    WriteDword(0xc71u, 0x11223344ul);
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

u32 ObserveWritePipelineBatchOperation(u8 operation, u32 argument0, u32 argument1, u32 argument2)
{
    u32 result = 0ul;

    assert(operation < 24u && event_count < 1024u);
    events[event_count].operation = operation;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    events[event_count].argument2 = argument2;
    ++event_count;

    if(operation_result_index[operation] < operation_result_count[operation])
        result = operation_results[operation][operation_result_index[operation]++];

    if(operation == WRITE_BATCH_YIELD)
        memory_image[0xfb9au] = 0u;
    if(operation == WRITE_BATCH_RESUME_RING)
    {
        WriteDword(0x126eu, resumed_ring_address);
        result = resumed_ring_address;
    }
    if(operation == WRITE_BATCH_ADVANCE_RING)
        result = argument0 + 0x930ul;
    return result;
}

static void TestInactivePipelineRejectsWithoutHelpers(void)
{
    ResetHarness();
    memory_image[0x1596u] = 1u;
    assert(PollWritePipelineTransferBatchStatus() == 0u);
    assert(event_count == 0u);
}

static void TestCapacityStatusRoutes(void)
{
    static const u32 one[] = { 1ul };
    static const u32 zero[] = { 0ul };

    ResetHarness();
    memory_image[0x127du] = 1u;
    SetResults(WRITE_BATCH_SHOULD_WAIT, one, 1u);
    SetResults(WRITE_BATCH_RECOVER_FAILURE, zero, 1u);
    assert(PollWritePipelineTransferBatchStatus() == 0u);
    assert(CountEvents(WRITE_BATCH_RECOVER_FAILURE) == 1u);

    ResetHarness();
    SetResults(WRITE_BATCH_SHOULD_WAIT, one, 1u);
    SetResults(WRITE_BATCH_RESTART_ACQUISITION, one, 1u);
    assert(PollWritePipelineTransferBatchStatus() == 0xfeu);
    assert(CountEvents(WRITE_BATCH_RESTART_ACQUISITION) == 1u);

    ResetHarness();
    memory_image[0x127du] = 1u;
    SetResults(WRITE_BATCH_SHOULD_WAIT, zero, 1u);
    SetResults(WRITE_BATCH_FAIL_PREPARATION, one, 1u);
    assert(PollWritePipelineTransferBatchStatus() == 0u);
    assert(CountEvents(WRITE_BATCH_RELEASE_WAIT) == 0u);

    ResetHarness();
    SetResults(WRITE_BATCH_SHOULD_WAIT, zero, 1u);
    SetResults(WRITE_BATCH_RELEASE_WAIT, zero, 1u);
    assert(PollWritePipelineTransferBatchStatus() == 0xfeu);
}

static void TestActivePipelineReconciliation(void)
{
    static const u32 zero[] = { 0ul };
    static const u32 descriptor[] = { 0x0200ul };

    ResetHarness();
    memory_image[0xfb9au] = 1u;
    WriteWord(0xfbb4u, 0x4fu);
    WriteWord(0x1292u, 6u);
    WriteWord(0x202u, 0x300u);
    WriteWord(0x300u, 0x1234u);
    WriteDword(0x1598u, 0x56789abcul);
    SetResults(WRITE_BATCH_LATEST_DESCRIPTOR, descriptor, 1u);
    SetResults(WRITE_BATCH_SHOULD_WAIT, zero, 1u);
    SetResults(WRITE_BATCH_RELEASE_WAIT, zero, 1u);

    assert(PollWritePipelineTransferBatchStatus() == 0xfeu);
    assert(CountEvents(WRITE_BATCH_ACQUIRE_PREFIX) == 1u);
    assert(CountEvents(WRITE_BATCH_YIELD) == 1u);
    assert(ReadDword(0x1598u) == 0x56789abcul);
    assert(ReadDword(0x8000u) == 0x56789abcul);
    assert(ReadDword(0x14bcu) == 0x80000000ul);
}

static void TestCompleteSeededBatch(void)
{
    static const u32 zero[] = { 0ul };
    static const u32 one[] = { 1ul };
    static const u32 waits[] = { 0ul, 1ul };
    u16 index;

    ResetHarness();
    memory_image[0x2ba5cau] = 1u;
    SetResults(WRITE_BATCH_SHOULD_WAIT, zero, 1u);
    SetResults(WRITE_BATCH_RELEASE_WAIT, one, 1u);
    SetResults(WRITE_BATCH_WAIT_CAPACITY, waits, 2u);
    for(index = 0u; index < 7u; ++index)
        memory_image[0xf84890u + index] = (u8)index;

    assert(PollWritePipelineTransferBatchStatus() == 1u);
    assert(CountEvents(WRITE_BATCH_BUILD_Q) == 3u);
    assert(CountEvents(WRITE_BATCH_COPY_Q_MSF) == 2u);
    assert(CountEvents(WRITE_BATCH_ENQUEUE_DESCRIPTOR) == 39u);
    assert(CountEvents(WRITE_BATCH_WRITE_REGISTER_BLOCK) == 7u);
    assert(CountEvents(WRITE_BATCH_SUBMIT_COMMAND) == 7u);
    assert(CountEvents(WRITE_BATCH_ADVANCE_RING) == 39u);
    assert(CountEvents(WRITE_BATCH_ADVANCE_Q_MSF) == 39u);
    assert(CountEvents(WRITE_BATCH_COPY_AUXILIARY_PREFIX) == 32u);
    assert(CountEvents(WRITE_BATCH_SEED_RANGE) == 1u);
    assert(CountEvents(WRITE_BATCH_LATCH_FAILURE) == 1u);
    assert(ReadDword(0x1598u) == 1007ul);
    assert(ReadDword(0x14bcu) == 1007ul);
    assert(memory_image[0x18028u] == (u8)((6u << 5) | 2u));
    assert(ReadDword(0x10200cu) == 0x01020302ul);
    assert(ReadDword(0x102010u) == 0x11223344ul);
    assert(ReadDword(0x102014u) == 0x11223344ul);
}

static void TestRelocationSplitsAtRingEnd(void)
{
    static const u32 zero[] = { 0ul };
    static const u32 one[] = { 1ul };
    Event *first;
    Event *second;

    ResetHarness();
    WriteDword(0x156au, 34ul);
    memory_image[0x2ba5c6u] = 4u;
    memory_image[0x2ba5cau] = 0u;
    memory_image[0x2ba5c8u] = 0u;
    SetResults(WRITE_BATCH_SHOULD_WAIT, zero, 1u);
    SetResults(WRITE_BATCH_RELEASE_WAIT, one, 1u);

    assert(PollWritePipelineTransferBatchStatus() == 1u);
    assert(CountEvents(WRITE_BATCH_TRANSFER_SECTORS) == 2u);
    first = FindEvent(WRITE_BATCH_TRANSFER_SECTORS, 0u);
    second = FindEvent(WRITE_BATCH_TRANSFER_SECTORS, 1u);
    assert(first->argument0 == 0x10000ul);
    assert(first->argument1 == 33ul * 0x930ul);
    assert(first->argument2 == 1ul);
    assert(second->argument0 == 0x10000ul + 0x930ul);
    assert(second->argument1 == 0ul);
    assert(second->argument2 == 3ul);
}

int main(void)
{
    TestInactivePipelineRejectsWithoutHelpers();
    TestCapacityStatusRoutes();
    TestActivePipelineReconciliation();
    TestCompleteSeededBatch();
    TestRelocationSplitsAtRingEnd();
    puts("write-pipeline transfer batch poll tests passed");
    return 0;
}
