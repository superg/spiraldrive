#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/write_controller_range_sequence.h"

enum SequenceEvent
{
    EVENT_BUILD,
    EVENT_PREPARE,
    EVENT_FAILURE,
    EVENT_YIELD,
    EVENT_SELECT,
    EVENT_INITIALIZE,
    EVENT_FILL,
    EVENT_ACQUIRE,
    EVENT_WAIT
};

typedef struct EventRecord
{
    u8 event;
    u32 argument0;
    u32 argument1;
} EventRecord;

static u8 memory[0x1000000ul];
static EventRecord events[40];
static unsigned event_count;
static u16 build_result;
static u8 prepare_status[4];
static unsigned prepare_count;
static u8 selected_end[4];
static u8 selected_split[4];
static u32 selected_range_start[4];
static unsigned select_count;
static u32 acquire_replacement_target;

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

static void RecordEvent(u8 event, u32 argument0, u32 argument1)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].event = event;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    ++event_count;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

u16 ObserveWriteRangeSequenceBuild(s32 start_frame_position, u32 block_count)
{
    RecordEvent(EVENT_BUILD, (u32)start_frame_position, block_count);
    return build_result;
}

u8 ObserveWriteRangeSequencePrepare(u8 omit_final_prefix_frame, u8 wait_for_completion)
{
    u8 status = prepare_status[prepare_count++];

    RecordEvent(EVENT_PREPARE, omit_final_prefix_frame, wait_for_completion);
    if(status != 0u)
    {
        memory[0xfb9au] = 1u;
        memory[0xfb9fu] = status;
    }
    return 0u;
}

void ObserveWriteRangeSequenceFailure(u8 failure_code)
{
    RecordEvent(EVENT_FAILURE, failure_code, 0u);
}

void ObserveWriteRangeSequenceYield(void)
{
    RecordEvent(EVENT_YIELD, 0u, 0u);
    memory[0xfb9au] = 0u;
}

u8 ObserveWriteRangeSequenceSelect(u8 start_index, u8 record_count)
{
    unsigned index = select_count++;

    RecordEvent(EVENT_SELECT, start_index, record_count);
    memory[0x2ba5cfu] = selected_split[index];
    WriteU32(0x2ba5bcu, selected_range_start[index]);
    return selected_end[index];
}

void ObserveWriteRangeSequenceInitializeTrack(void)
{
    RecordEvent(EVENT_INITIALIZE, 0u, 0u);
    WriteU32(0x0b02u, 0xdeadbeeful);
}

void ObserveWriteRangeSequenceFill(fw_addr_t buffer_address, u32 byte_count, u8 value)
{
    RecordEvent(EVENT_FILL, buffer_address, byte_count);
    assert(value == 0u);
}

u8 ObserveWriteRangeSequenceAcquire(s32 target_address, u8 mapping_mode)
{
    RecordEvent(EVENT_ACQUIRE, (u32)target_address, mapping_mode);
    if(acquire_replacement_target != 0ul)
        WriteU32(0x2ba5bcu, acquire_replacement_target);
    return 0u;
}

void ObserveWriteRangeSequenceWait(void)
{
    RecordEvent(EVENT_WAIT, 0u, 0u);
    assert(memory[0x2ba5c8u] == 1u);
    assert(memory[0x2ba5cau] == 1u);
    assert(memory[0x0ca1u] == 0u);
    assert(memory[0x0c2au] == 6u);
    assert(memory[0x15d7u] == 0u);
    assert(ReadU32(0x1598u) == ReadU32(0x2ba5bcu));
    assert(ReadU32(0x0b02u) == 0x12345678ul);
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(prepare_status, 0, sizeof(prepare_status));
    memset(selected_end, 0, sizeof(selected_end));
    memset(selected_split, 0, sizeof(selected_split));
    memset(selected_range_start, 0, sizeof(selected_range_start));
    event_count = 0u;
    build_result = 0u;
    prepare_count = 0u;
    select_count = 0u;
    acquire_replacement_target = 0ul;
    WriteU32(0x2ba5b4u, 0xffffff9cul);
    memory[0x2ba5c6u] = 7u;
    WriteU32(0x0b02u, 0x12345678ul);
}

static void AssertEventTypes(const u8 *expected, unsigned expected_count)
{
    unsigned index;

    assert(event_count == expected_count);
    for(index = 0u; index < expected_count; ++index)
        assert(events[index].event == expected[index]);
}

static void TestEmptyRangeReturnsSuccess(void)
{
    static const u8 expected[] = { EVENT_BUILD };

    ResetFixture();
    memory[0x2ba5ccu] = 0xaau;
    memory[0x2ba5cdu] = 0xbbu;
    memory[0x2ba5ceu] = 0xccu;
    memory[0x2ba5cfu] = 0xddu;

    assert(ExecuteWriteControllerBufferRangeSequence() == 1u);
    AssertEventTypes(expected, sizeof(expected));
    assert((s32)events[0].argument0 == -100L);
    assert(events[0].argument1 == 7u);
    assert(memory[0x2ba5ccu] == 0u);
    assert(memory[0x2ba5cdu] == 0u);
    assert(memory[0x2ba5ceu] == 0u);
    assert(memory[0x2ba5cfu] == 0u);
}

static void TestInitialPipelineFailure(void)
{
    static const u8 expected[] = { EVENT_BUILD, EVENT_PREPARE, EVENT_FAILURE };

    ResetFixture();
    build_result = 1u;
    prepare_status[0] = 2u;

    assert(ExecuteWriteControllerBufferRangeSequence() == 0u);
    AssertEventTypes(expected, sizeof(expected));
    assert(events[1].argument0 == 0u && events[1].argument1 == 1u);
    assert(events[2].argument0 == 1u);
    assert(memory[0x14b5u] == 1u);
    assert(select_count == 0u);
}

static void TestSingleRangeWithBothPipelineWaits(void)
{
    static const u8 expected[] = { EVENT_BUILD, EVENT_PREPARE, EVENT_YIELD, EVENT_SELECT, EVENT_INITIALIZE, EVENT_FILL, EVENT_ACQUIRE, EVENT_WAIT, EVENT_PREPARE, EVENT_YIELD };

    ResetFixture();
    build_result = 0x0101u;
    prepare_status[0] = 1u;
    prepare_status[1] = 1u;
    selected_range_start[0] = 0x10203040ul;
    acquire_replacement_target = 0x11223344ul;

    assert(ExecuteWriteControllerBufferRangeSequence() == 1u);
    AssertEventTypes(expected, sizeof(expected));
    assert(memory[0x2ba5ccu] == 1u);
    assert(events[3].argument0 == 0u && events[3].argument1 == 1u);
    assert(events[5].argument0 == 0xcca0u && events[5].argument1 == 0u);
    assert(events[6].argument0 == 0x10203040ul && events[6].argument1 == 1u);
    assert(ReadU32(0x1598u) == 0x11223344ul);
    assert(memory[0x2ba5cdu] == 1u && memory[0x2ba5ceu] == 1u);
    assert(memory[0x2ba5c8u] == 0u && memory[0x2ba5cau] == 0u);
    assert(memory[0x127du] == 0u && memory[0x127eu] == 0u);
    assert(memory[0x1554u] == 0u && memory[0x14b5u] == 1u);
    assert(memory[0x0ca0u] == 0u && memory[0x0ca1u] == 0u);
    assert((memory[0x0c8au] & 0x10u) != 0u);
    assert(ReadU32(0x0b02u) == 0x12345678ul);
}

static void TestPostRangePipelineFailure(void)
{
    static const u8 expected[] = { EVENT_BUILD, EVENT_SELECT, EVENT_INITIALIZE, EVENT_FILL, EVENT_ACQUIRE, EVENT_WAIT, EVENT_PREPARE, EVENT_FAILURE };

    ResetFixture();
    build_result = 1u;
    memory[0x2ba5cbu] = 1u;
    selected_range_start[0] = 0x87654321ul;
    prepare_status[0] = 2u;

    assert(ExecuteWriteControllerBufferRangeSequence() == 0u);
    AssertEventTypes(expected, sizeof(expected));
    assert(memory[0x2ba5cdu] == 0u && memory[0x2ba5ceu] == 0u);
    assert(memory[0x2ba5c8u] == 0u && memory[0x2ba5cau] == 0u);
    assert(events[7].argument0 == 1u);
}

static void TestSplitFlagForcesSecondRange(void)
{
    ResetFixture();
    build_result = 1u;
    memory[0x2ba5cbu] = 1u;
    selected_end[0] = 0u;
    selected_split[0] = 1u;
    selected_range_start[0] = 1000ul;
    selected_end[1] = 1u;
    selected_range_start[1] = 2000ul;

    assert(ExecuteWriteControllerBufferRangeSequence() == 1u);
    assert(select_count == 2u);
    assert(events[1].event == EVENT_SELECT);
    assert(events[1].argument0 == 0u && events[1].argument1 == 1u);
    assert(events[7].event == EVENT_SELECT);
    assert(events[7].argument0 == 1u && events[7].argument1 == 1u);
    assert(memory[0x2ba5cdu] == 2u && memory[0x2ba5ceu] == 2u);
    assert(memory[0x2ba5cfu] == 0u);
    assert(ReadU32(0x1598u) == 2000ul);
}

int main(void)
{
    TestEmptyRangeReturnsSuccess();
    TestInitialPipelineFailure();
    TestSingleRangeWithBothPipelineWaits();
    TestPostRangePipelineFailure();
    TestSplitFlagForcesSecondRange();
    puts("write controller range sequence tests passed");
    return 0;
}
