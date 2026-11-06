#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/start_stop_unit.h"

enum event_kind
{
    EVENT_VALIDATE,
    EVENT_UNSUPPORTED,
    EVENT_SET_SENSE,
    EVENT_STATUS_SENSE,
    EVENT_NO_SENSE,
    EVENT_P3,
    EVENT_RESET_COROUTINES,
    EVENT_YIELD,
    EVENT_DISPATCH_WAIT,
    EVENT_DISPATCH_PENDING,
    EVENT_PREPARE_ATA,
    EVENT_PUBLISH_ATA,
    EVENT_PREPARE_WRITE,
    EVENT_ACTION2_WORKER,
    EVENT_ACTION3_WORKER
};

struct event_record
{
    enum event_kind kind;
    u32 first;
    u32 second;
};

enum yield_mode
{
    YIELD_NONE,
    YIELD_ACTION0,
    YIELD_ACTION2,
    YIELD_ACTION3_SYNC
};

static u8 memory[1u << 24];
static struct event_record events[96];
static size_t event_count;
static u8 validation_result;
static enum yield_mode active_yield_mode;
static size_t yield_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void RecordEvent(enum event_kind kind, u32 first, u32 second)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    event_count++;
}

u8 ObserveStartStopValidateCdb(fw_addr_t mask_address)
{
    RecordEvent(EVENT_VALIDATE, mask_address, validation_result);
    return validation_result;
}

void ObserveStartStopUnsupported(void)
{
    RecordEvent(EVENT_UNSUPPORTED, 0u, 0u);
}

void ObserveStartStopSetSense(u8 key, u16 code)
{
    RecordEvent(EVENT_SET_SENSE, key, code);
}

void ObserveStartStopSetSenseFromStatus(u8 status)
{
    RecordEvent(EVENT_STATUS_SENSE, status, 0u);
}

void ObserveStartStopSetNoSense(void)
{
    RecordEvent(EVENT_NO_SENSE, 0u, 0u);
}

void ObserveStartStopSetP3Bit(u8 bit_index, u8 value)
{
    RecordEvent(EVENT_P3, bit_index, value);
}

void ObserveStartStopResetMediaCoroutines(void)
{
    RecordEvent(EVENT_RESET_COROUTINES, 0u, 0u);
}

void ObserveStartStopYieldCoroutine0(void)
{
    yield_count++;
    RecordEvent(EVENT_YIELD, (u32)yield_count, active_yield_mode);
    if(active_yield_mode == YIELD_ACTION0)
    {
        memory[0x0015c3u] = 1u;
    }
    else if(active_yield_mode == YIELD_ACTION2)
    {
        if(yield_count == 1u)
            memory[0x001300u] = 0u;
        else if(yield_count == 2u)
            memory[0x0015c4u] = 1u;
        else if(yield_count == 3u)
            memory[0x001300u] = 0u;
    }
    else if(active_yield_mode == YIELD_ACTION3_SYNC)
    {
        if(yield_count == 1u)
            memory[0x001300u] = 0u;
        else if(yield_count == 2u)
            memory[0x000c84u] |= 2u;
        else if(yield_count == 3u)
            memory[0x001300u] = 0u;
        else if(yield_count == 5u)
            memory[0x000faeu] = 15u;
    }
}

void ObserveStartStopDispatchAndWait(void)
{
    RecordEvent(EVENT_DISPATCH_WAIT, 0u, 0u);
}

void ObserveStartStopDispatchPending(void)
{
    RecordEvent(EVENT_DISPATCH_PENDING, 0u, 0u);
}

void ObserveStartStopPrepareAtaCompletion(void)
{
    RecordEvent(EVENT_PREPARE_ATA, 0u, 0u);
}

void ObserveStartStopPublishAtaCompletion(void)
{
    RecordEvent(EVENT_PUBLISH_ATA, 0u, 0u);
}

void ObserveStartStopPrepareMediaWrite(void)
{
    RecordEvent(EVENT_PREPARE_WRITE, 0u, 0u);
}

void ObserveStartStopAction2Worker(void)
{
    RecordEvent(EVENT_ACTION2_WORKER, 0u, 0u);
    if(active_yield_mode == YIELD_ACTION2)
        memory[0x001300u] = 10u;
}

void ObserveStartStopAction3Worker(void)
{
    RecordEvent(EVENT_ACTION3_WORKER, 0u, 0u);
    if(active_yield_mode == YIELD_ACTION3_SYNC)
        memory[0x001300u] = 9u;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    validation_result = 1u;
    active_yield_mode = YIELD_NONE;
    yield_count = 0u;
}

static size_t CountEvents(enum event_kind kind)
{
    size_t index;
    size_t count = 0u;
    for(index = 0u; index < event_count; ++index)
        if(events[index].kind == kind)
            count++;
    return count;
}

static const struct event_record *FindEvent(enum event_kind kind, size_t occurrence)
{
    size_t index;
    for(index = 0u; index < event_count; ++index)
    {
        if(events[index].kind == kind)
        {
            if(occurrence == 0u)
                return &events[index];
            occurrence--;
        }
    }
    assert(0);
    return NULL;
}

static void AssertSense(u8 key, u16 code)
{
    assert(CountEvents(EVENT_SET_SENSE) == 1u);
    assert(FindEvent(EVENT_SET_SENSE, 0u)->first == key);
    assert(FindEvent(EVENT_SET_SENSE, 0u)->second == code);
}

static void TestValidationAndActionRejection(void)
{
    u16 action;

    ResetState();
    validation_result = 0u;
    HandleStartStopUnitCommand();
    assert(FindEvent(EVENT_VALIDATE, 0u)->first == 0xff8e0cul);
    assert(CountEvents(EVENT_UNSUPPORTED) == 1u);

    for(action = 4u; action < 256u; ++action)
    {
        event_count = 0u;
        validation_result = 1u;
        memory[0x000b0eu] = (u8)action;
        HandleStartStopUnitCommand();
        assert(CountEvents(EVENT_UNSUPPORTED) == 1u);
        assert(event_count == 2u);
    }
}

static void TestAction0SynchronousStop(void)
{
    ResetState();
    active_yield_mode = YIELD_ACTION0;
    memory[0x000b0eu] = 0u;
    memory[0x000b0bu] = 0u;
    memory[0x000c85u] = 2u;
    memory[0x000c9eu] = 3u;
    memory[0x000c89u] = 0x80u;
    memory[0x000c8au] = 0x20u;
    memory[0x000c8bu] = 8u;

    HandleStartStopUnitCommand();

    assert(yield_count == 1u);
    assert(CountEvents(EVENT_RESET_COROUTINES) == 1u);
    assert(CountEvents(EVENT_NO_SENSE) == 1u);
    assert(CountEvents(EVENT_P3) == 4u);
    assert(FindEvent(EVENT_P3, 0u)->first == 0u && FindEvent(EVENT_P3, 0u)->second == 0u);
    assert(FindEvent(EVENT_P3, 1u)->first == 1u && FindEvent(EVENT_P3, 1u)->second == 0u);
    assert(FindEvent(EVENT_P3, 2u)->first == 0u && FindEvent(EVENT_P3, 2u)->second == 1u);
    assert(FindEvent(EVENT_P3, 3u)->first == 1u && FindEvent(EVENT_P3, 3u)->second == 0u);
    assert(memory[0x000c8au] == 0x21u);
    assert(memory[0x00122cu] == 60u);
}

static void TestAction1ImmediateLoad(void)
{
    ResetState();
    memory[0x000b0eu] = 1u;
    memory[0x000b0bu] = 1u;
    memory[0x000c85u] = 2u;
    memory[0x000c9eu] = 2u;
    memory[0x2ba5afu] = 0u;
    memory[0x00e750u] = 0u;
    memory[0x000c89u] = 1u;
    memory[0x000e3bu] = 0x12u;
    memory[0x000e3cu] = 0x34u;
    memory[0x000e3du] = 0x56u;
    memory[0x000e3eu] = 0x78u;

    HandleStartStopUnitCommand();

    assert(CountEvents(EVENT_DISPATCH_PENDING) == 1u);
    assert(CountEvents(EVENT_DISPATCH_WAIT) == 0u);
    assert(CountEvents(EVENT_NO_SENSE) == 1u);
    assert(memory[0x000c8cu] == 0u);
    assert(memory[0x000c8eu] == 0x12u);
    assert(memory[0x000c8fu] == 0x34u);
    assert(memory[0x000c90u] == 0x56u);
    assert(memory[0x000c91u] == 0x78u);
    assert(memory[0x000c89u] == 9u);
    assert(memory[0x00122bu] == 20u);
}

static void TestAction2WriteFinalization(void)
{
    ResetState();
    active_yield_mode = YIELD_ACTION2;
    memory[0x000b0eu] = 2u;
    memory[0x000b0bu] = 0u;
    memory[0x000c9eu] = 3u;
    memory[0x000c89u] = 0x88u;
    memory[0x000c8au] = 0x55u;
    memory[0x000c8bu] = 1u;
    memory[0x000c83u] = 0x10u;
    memory[0x001300u] = 9u;

    HandleStartStopUnitCommand();

    assert(yield_count == 3u);
    assert(CountEvents(EVENT_PREPARE_ATA) == 1u);
    assert(CountEvents(EVENT_PUBLISH_ATA) == 1u);
    assert(CountEvents(EVENT_PREPARE_WRITE) == 1u);
    assert(CountEvents(EVENT_ACTION2_WORKER) == 1u);
    assert(CountEvents(EVENT_NO_SENSE) == 2u);
    assert(memory[0x000c89u] == 0x80u);
    assert(memory[0x000c8au] == 0x54u);
    assert(memory[0x000c8bu] == 1u);
    assert(memory[0x000c83u] == 0x12u);
    assert(memory[0x00122cu] == 10u);
    assert(FindEvent(EVENT_P3, 2u)->second == 1u);
    assert(FindEvent(EVENT_P3, 3u)->second == 1u);
}

static void TestAction3ImmediateStatus(void)
{
    ResetState();
    memory[0x000b0eu] = 3u;
    memory[0x000b0bu] = 1u;
    memory[0x000c84u] = 4u;
    memory[0x000faeu] = 3u;

    HandleStartStopUnitCommand();

    assert(memory[0x000c84u] == 4u);
    assert(CountEvents(EVENT_ACTION3_WORKER) == 1u);
    assert(yield_count == 1u);
    AssertSense(2u, 0x0401u);
    assert(CountEvents(EVENT_NO_SENSE) == 0u);
}

static void TestAction3SynchronousFailure(void)
{
    ResetState();
    active_yield_mode = YIELD_ACTION3_SYNC;
    memory[0x000b0eu] = 3u;
    memory[0x000b0bu] = 0u;
    memory[0x000c84u] = 4u;
    memory[0x001300u] = 10u;
    memory[0x000faeu] = 3u;

    HandleStartStopUnitCommand();

    assert(yield_count == 5u);
    assert((memory[0x000c84u] & 2u) != 0u);
    AssertSense(2u, 0x5300u);
    assert(CountEvents(EVENT_NO_SENSE) == 0u);
}

static void TestFixedSenseRoutes(void)
{
    ResetState();
    memory[0x000b0eu] = 0u;
    memory[0x000faeu] = 7u;
    HandleStartStopUnitCommand();
    assert(CountEvents(EVENT_STATUS_SENSE) == 1u);
    assert(FindEvent(EVENT_STATUS_SENSE, 0u)->first == 7u);

    ResetState();
    memory[0x000b0eu] = 2u;
    memory[0x000c35u] = 1u;
    memory[0x000c36u] = 0u;
    HandleStartStopUnitCommand();
    AssertSense(5u, 0x5302u);

    ResetState();
    memory[0x000b0eu] = 3u;
    memory[0x000c84u] = 0u;
    HandleStartStopUnitCommand();
    assert(CountEvents(EVENT_UNSUPPORTED) == 1u);
}

int main(void)
{
    TestValidationAndActionRejection();
    TestAction0SynchronousStop();
    TestAction1ImmediateLoad();
    TestAction2WriteFinalization();
    TestAction3ImmediateStatus();
    TestAction3SynchronousFailure();
    TestFixedSenseRoutes();
    return 0;
}
