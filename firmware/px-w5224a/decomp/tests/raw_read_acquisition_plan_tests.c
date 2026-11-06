#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_acquisition_plan.h"
#include "platform/firmware_memory.h"

#define EVENT_CAPACITY 32u
#define READ_CAPACITY 256u

struct OperationEvent
{
    u8 operation;
    s32 argument0;
    s32 argument1;
    s32 argument2;
};

static u8 memory_bytes[0x20000ul];
static fw_addr_t read_addresses[READ_CAPACITY];
static unsigned read_count;
static struct OperationEvent events[EVENT_CAPACITY];
static unsigned event_count;
static u8 initialize_result;
static u8 eligibility_results[2];
static unsigned eligibility_call_count;
static u8 map_result;
static u8 execute_result;
static u8 sector_class_result;
static u8 acquire_result;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    assert(read_count < READ_CAPACITY);
    read_addresses[read_count++] = address;
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

s32 ObserveRawReadAcquisitionPlanOperation(u8 operation, s32 argument0, s32 argument1, s32 argument2)
{
    struct OperationEvent *event;

    assert(event_count < EVENT_CAPACITY);
    event = &events[event_count++];
    event->operation = operation;
    event->argument0 = argument0;
    event->argument1 = argument1;
    event->argument2 = argument2;

    switch(operation)
    {
    case RAW_READ_PLAN_INITIALIZE_RANGE:
        return initialize_result;
    case RAW_READ_PLAN_TEST_ELIGIBILITY:
        assert(eligibility_call_count < 2u);
        return eligibility_results[eligibility_call_count++];
    case RAW_READ_PLAN_MAP_STATUS:
        return map_result;
    case RAW_READ_PLAN_EXECUTE_PENDING_RANGE:
        return execute_result;
    case RAW_READ_PLAN_SELECT_SECTOR_CLASS:
        return sector_class_result;
    case RAW_READ_PLAN_ACQUIRE_SEGMENTS:
        return acquire_result;
    default:
        return 0l;
    }
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(read_addresses, 0, sizeof(read_addresses));
    memset(events, 0, sizeof(events));
    read_count = 0u;
    event_count = 0u;
    eligibility_call_count = 0u;
    initialize_result = 1u;
    eligibility_results[0] = 0u;
    eligibility_results[1] = 0u;
    map_result = 0u;
    execute_result = 1u;
    sector_class_result = 0u;
    acquire_result = 1u;
}

static void AssertEvent(unsigned index, u8 operation, s32 argument0, s32 argument1)
{
    assert(index < event_count);
    assert(events[index].operation == operation);
    assert(events[index].argument0 == argument0);
    assert(events[index].argument1 == argument1);
}

static void AssertReadSequence(fw_addr_t first, fw_addr_t second, fw_addr_t third)
{
    unsigned index;

    for(index = 0u; index + 2u < read_count; ++index)
        if(read_addresses[index] == first && read_addresses[index + 1u] == second && read_addresses[index + 2u] == third)
            return;
    assert(0 && "expected read sequence was not observed");
}

static void TestAlreadyInitializedPrimaryPlanBypass(void)
{
    ResetFixture();
    memory_bytes[0x1286u] = 1u;
    memory_bytes[0x0c44u] = 1u;

    assert(PrepareRawReadAcquisitionPlan(0u) == 1u);
    assert(event_count == 0u);
}

static void TestInitializationFalseIsAccepted(void)
{
    ResetFixture();
    initialize_result = 0u;

    assert(PrepareRawReadAcquisitionPlan(1u) == 1u);
    assert(event_count == 1u);
    AssertEvent(0u, RAW_READ_PLAN_INITIALIZE_RANGE, 1l, 0l);
}

static void TestZeroDataBlockEligibleStateSetsSequenceError(void)
{
    ResetFixture();
    eligibility_results[0] = 1u;

    assert(PrepareRawReadAcquisitionPlan(0u) == 0u);
    assert(event_count == 3u);
    AssertEvent(0u, RAW_READ_PLAN_INITIALIZE_RANGE, 0l, 0l);
    AssertEvent(1u, RAW_READ_PLAN_TEST_ELIGIBILITY, 1l, 0l);
    AssertEvent(2u, RAW_READ_PLAN_SET_SEQUENCE_ERROR, 0l, 0l);
}

static void TestMappedQTypeAndPendingRangeFailure(void)
{
    ResetFixture();
    memory_bytes[0x0c44u] = 1u;
    memory_bytes[0x0c48u] = 5u;
    memory_bytes[0x1149u] = 1u;
    memory_bytes[0xdd33u] = 0x16u;
    memory_bytes[0xdd32u] = 0x27u;
    memory_bytes[0xdd31u] = 0x40u;
    map_result = 12u;
    execute_result = 0u;

    assert(PrepareRawReadAcquisitionPlan(0u) == 0u);
    assert(memory_bytes[0x1285u] == 12u);
    assert(event_count == 4u);
    AssertEvent(2u, RAW_READ_PLAN_MAP_STATUS, 0x27l, 0x16l);
    assert(events[2].argument2 == 0x40l);
    AssertEvent(3u, RAW_READ_PLAN_EXECUTE_PENDING_RANGE, 0l, 0l);
    AssertReadSequence(0xdd33u, 0xdd32u, 0xdd31u);
}

static void TestSecondEligibilityAndAlternatePlanReachSegmentAcquisition(void)
{
    ResetFixture();
    memory_bytes[0x0c48u] = 4u;
    eligibility_results[1] = 7u;
    sector_class_result = 9u;
    acquire_result = 0u;

    assert(PrepareRawReadAcquisitionPlan(1u) == 0u);
    assert(memory_bytes[0x1285u] == 4u);
    assert(memory_bytes[0x142bu] == 9u);
    assert(event_count == 6u);
    AssertEvent(3u, RAW_READ_PLAN_TEST_ELIGIBILITY, 0l, 0l);
    AssertEvent(4u, RAW_READ_PLAN_SELECT_SECTOR_CLASS, 0l, 7l);
    AssertEvent(5u, RAW_READ_PLAN_ACQUIRE_SEGMENTS, 7l, 1l);
}

static void TestSuccessfulCurrentQPacketPlan(void)
{
    unsigned index;

    ResetFixture();
    memory_bytes[0x0c48u] = 4u;
    eligibility_results[1] = 1u;
    sector_class_result = 6u;
    memory_bytes[0x88b7u] = 1u;
    memory_bytes[0x14b4u] = 1u;
    WriteU32(0xdd3du, 900ul);
    WriteU32(0x1598u, 1000ul);
    for(index = 0u; index < 10u; ++index)
        memory_bytes[0x1272u + index] = (u8)(0x21u + index);
    memory_bytes[0x1272u] = 0x21u;

    assert(PrepareRawReadAcquisitionPlan(0u) == 1u);
    assert(event_count == 9u);
    AssertEvent(6u, RAW_READ_PLAN_SET_MSF_TUPLES, 100l, 1000l);
    AssertEvent(7u, RAW_READ_PLAN_SET_MSF_TUPLES, 0l, 1000l);
    AssertEvent(8u, RAW_READ_PLAN_CONFIGURE_LAYOUT, 0l, 0l);
    for(index = 0u; index < 10u; ++index)
    {
        u8 expected = (u8)(0x21u + index);
        if(index == 2u)
            expected = 1u;
        assert(memory_bytes[0x88aau + index] == expected);
    }
    assert(memory_bytes[0x1274u] == 1u);
    assert(memory_bytes[0x127cu] == 0u);
    assert(memory_bytes[0x14abu] == 1u);
    assert(memory_bytes[0x14b4u] == 1u);
    assert(memory_bytes[0x1286u] == 1u);
}

static void TestSuccessfulSyntheticQPacketPlan(void)
{
    unsigned index;

    ResetFixture();
    memory_bytes[0x0c44u] = 1u;
    memory_bytes[0x0c48u] = 4u;
    memory_bytes[0x1149u] = 1u;
    memory_bytes[0xdd33u] = 8u;
    memory_bytes[0xdd32u] = 2u;
    memory_bytes[0xdd31u] = 0x40u;
    map_result = 12u;
    sector_class_result = 9u;
    memory_bytes[0x14b4u] = 1u;
    WriteU32(0x1598u, 1234ul);
    for(index = 0u; index < 10u; ++index)
        memory_bytes[0x1491u + index] = (u8)(0x40u + index);

    assert(PrepareRawReadAcquisitionPlan(1u) == 1u);
    assert(event_count == 10u);
    AssertEvent(2u, RAW_READ_PLAN_MAP_STATUS, 2l, 8l);
    AssertEvent(5u, RAW_READ_PLAN_SELECT_SECTOR_CLASS, 0l, 0l);
    AssertEvent(6u, RAW_READ_PLAN_ACQUIRE_SEGMENTS, 0l, 1l);
    AssertEvent(7u, RAW_READ_PLAN_SET_MSF_TUPLES, 0l, 1234l);
    AssertEvent(8u, RAW_READ_PLAN_CONFIGURE_LAYOUT, 0l, 0l);
    AssertEvent(9u, RAW_READ_PLAN_DECREMENT_MSF, 0x88adl, 0l);
    assert(memory_bytes[0x1285u] == 4u);
    assert(memory_bytes[0x142bu] == 9u);
    assert(memory_bytes[0x88aau] == 0x40u);
    assert(memory_bytes[0x88abu] == 0x41u);
    assert(memory_bytes[0x88acu] == 1u);
    assert(memory_bytes[0x88adu] == 0u);
    assert(memory_bytes[0x88aeu] == 0u);
    assert(memory_bytes[0x88afu] == 0u);
    assert(memory_bytes[0x88b0u] == 0x46u);
    assert(memory_bytes[0x88b3u] == 0x49u);
    assert(memory_bytes[0x1274u] == 1u);
    assert(memory_bytes[0x14b4u] == 0u);
    assert(memory_bytes[0x1286u] == 1u);
}

int main(void)
{
    TestAlreadyInitializedPrimaryPlanBypass();
    TestInitializationFalseIsAccepted();
    TestZeroDataBlockEligibleStateSetsSequenceError();
    TestMappedQTypeAndPendingRangeFailure();
    TestSecondEligibilityAndAlternatePlanReachSegmentAcquisition();
    TestSuccessfulCurrentQPacketPlan();
    TestSuccessfulSyntheticQPacketPlan();
    puts("raw-read acquisition-plan tests passed");
    return 0;
}
