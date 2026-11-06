#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_port80f8_calibration_attempt.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_SET_FLAG,
    EVENT_WRITE_PARAMETERS,
    EVENT_WRITE_PORT,
    EVENT_READ_PORT,
    EVENT_WAIT_CLEAR,
    EVENT_WAIT_TIMER,
    EVENT_SET_CHANNEL,
    EVENT_RESET_CHANNEL,
    EVENT_YIELD
};

struct event_record
{
    enum event_kind kind;
    fw_addr_t first;
    u16 second;
};

static u8 memory[1u << 24];
static struct event_record events[96];
static size_t event_count;
static u8 ready_values[8];
static size_t ready_count;
static size_t ready_index;
static u8 status_values[24];
static size_t status_count;
static size_t status_index;
static u8 countdown_values[32];
static size_t countdown_count;
static size_t countdown_index;
static size_t fab5_read_count;
static u8 expire_shared_timer_on_yield;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x0180f8ul)
    {
        assert(status_index < status_count);
        return status_values[status_index++];
    }
    if(address == 0x001220ul)
    {
        assert(countdown_index < countdown_count);
        return countdown_values[countdown_index++];
    }
    if(address == 0x00fab5ul)
        fab5_read_count++;
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void RecordEvent(enum event_kind kind, fw_addr_t first, u16 second)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    event_count++;
}

void ObservePort80F8AttemptSetControllerFlag(u8 enabled)
{
    RecordEvent(EVENT_SET_FLAG, enabled, 0u);
}

void ObservePort80F8AttemptWriteParameters(u8 value)
{
    RecordEvent(EVENT_WRITE_PARAMETERS, value, 0u);
}

void ObservePort80F8AttemptWritePort(fw_addr_t address, u8 value)
{
    RecordEvent(EVENT_WRITE_PORT, address, value);
}

u8 ObservePort80F8AttemptReadPort(fw_addr_t address)
{
    u8 value;

    assert(ready_index < ready_count);
    value = ready_values[ready_index++];
    RecordEvent(EVENT_READ_PORT, address, value);
    return value;
}

void ObservePort80F8AttemptWaitForClear(void)
{
    RecordEvent(EVENT_WAIT_CLEAR, 0u, 0u);
}

void ObservePort80F8AttemptWaitTimer(u16 duration, u8 yield_mode)
{
    RecordEvent(EVENT_WAIT_TIMER, duration, yield_mode);
}

void ObservePort80F8AttemptSetChannel(u8 channel, u8 value)
{
    RecordEvent(EVENT_SET_CHANNEL, channel, value);
}

void ObservePort80F8AttemptResetChannel(u8 channel)
{
    RecordEvent(EVENT_RESET_CHANNEL, channel, 0u);
}

void ObservePort80F8AttemptYield(void)
{
    RecordEvent(EVENT_YIELD, 0u, 0u);
    if(expire_shared_timer_on_yield != 0u)
    {
        memory[0x0011fcu] = 0u;
        memory[0x0011fdu] = 0u;
        memory[0x0011feu] = 0u;
        memory[0x0011ffu] = 0u;
    }
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(ready_values, 0, sizeof(ready_values));
    memset(status_values, 0, sizeof(status_values));
    memset(countdown_values, 0xff, sizeof(countdown_values));
    event_count = 0u;
    ready_count = 1u;
    ready_index = 0u;
    status_count = 0u;
    status_index = 0u;
    countdown_count = 1u;
    countdown_index = 0u;
    fab5_read_count = 0u;
    expire_shared_timer_on_yield = 0u;
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

static void AssertInputsConsumed(void)
{
    assert(ready_index == ready_count);
    assert(status_index == status_count);
    assert(countdown_index == countdown_count);
}

static void SetFourErrorCountdown(void)
{
    static const u8 values[] = { 50u, 48u, 48u, 48u, 46u, 46u, 46u, 44u, 44u, 44u, 42u, 42u, 0xffu };
    memcpy(countdown_values, values, sizeof(values));
    countdown_count = sizeof(values);
}

static void SetThreeErrorCountdown(void)
{
    static const u8 values[] = { 50u, 48u, 48u, 48u, 46u, 46u, 46u, 44u, 44u, 0xffu };
    memcpy(countdown_values, values, sizeof(values));
    countdown_count = sizeof(values);
}

static void TestModeZeroImmediateStatusSuccess(void)
{
    const struct event_record *event;

    ResetState();
    ready_values[0] = 1u;
    ready_values[1] = 0u;
    ready_count = 2u;
    status_values[0] = 2u;
    status_values[1] = 1u;
    status_count = 2u;

    assert(RunControllerPort80F8CalibrationAttempt(0u) == 1u);
    assert(CountEvents(EVENT_SET_FLAG) == 0u);
    assert(CountEvents(EVENT_WRITE_PARAMETERS) == 0u);
    assert(CountEvents(EVENT_WAIT_CLEAR) == 1u);
    assert(CountEvents(EVENT_SET_CHANNEL) == 1u);
    event = FindEvent(EVENT_SET_CHANNEL, 0u);
    assert(event->first == 0u && event->second == 0x9du);
    assert(CountEvents(EVENT_RESET_CHANNEL) == 0u);
    AssertInputsConsumed();
}

static void TestModeOneSecondHandshakeFailure(void)
{
    ResetState();
    memory[0x00fab4u] = 0u;
    memory[0x00fab5u] = 0x66u;
    memory[0x001008u] = 2u;
    status_values[0] = 0u;
    status_values[1] = 0u;
    status_count = 2u;
    countdown_count = 0u;

    assert(RunControllerPort80F8CalibrationAttempt(1u) == 0u);
    assert(CountEvents(EVENT_SET_FLAG) == 3u);
    assert(FindEvent(EVENT_SET_FLAG, 0u)->first == 0u);
    assert(FindEvent(EVENT_SET_FLAG, 1u)->first == 1u);
    assert(FindEvent(EVENT_SET_FLAG, 2u)->first == 0u);
    assert(CountEvents(EVENT_WRITE_PARAMETERS) == 3u);
    assert(FindEvent(EVENT_WRITE_PARAMETERS, 0u)->first == 0u);
    assert(FindEvent(EVENT_WRITE_PARAMETERS, 1u)->first == 0x66u);
    assert(FindEvent(EVENT_WRITE_PARAMETERS, 2u)->first == 0u);
    assert(fab5_read_count == 2u);
    assert(CountEvents(EVENT_WAIT_TIMER) == 1u);
    assert(FindEvent(EVENT_WAIT_TIMER, 0u)->first == 20u && FindEvent(EVENT_WAIT_TIMER, 0u)->second == 0u);
    assert(CountEvents(EVENT_WAIT_CLEAR) == 2u);
    assert(CountEvents(EVENT_SET_CHANNEL) == 0u);
    AssertInputsConsumed();
}

static void TestFourSampleErrorsFail(void)
{
    size_t index;

    ResetState();
    memory[0x00fab4u] = 0x80u;
    memory[0xf7c01fu] = 1u;
    status_values[0] = 0u;
    status_values[1] = 2u;
    for(index = 2u; index < 6u; ++index)
        status_values[index] = 0u;
    status_values[6] = 1u;
    status_count = 7u;
    SetFourErrorCountdown();

    assert(RunControllerPort80F8CalibrationAttempt(1u) == 0u);
    assert(CountEvents(EVENT_SET_CHANNEL) == 2u);
    assert(FindEvent(EVENT_SET_CHANNEL, 0u)->second == 0x9du);
    assert(FindEvent(EVENT_SET_CHANNEL, 1u)->second == 0x99u);
    assert(CountEvents(EVENT_RESET_CHANNEL) == 1u);
    assert(fab5_read_count == 0u);
    AssertInputsConsumed();
}

static void TestThreeSampleErrorsSucceed(void)
{
    size_t index;

    ResetState();
    memory[0xf7c01fu] = 1u;
    status_values[0] = 2u;
    for(index = 1u; index < 4u; ++index)
        status_values[index] = 0u;
    status_values[4] = 1u;
    status_count = 5u;
    SetThreeErrorCountdown();

    assert(RunControllerPort80F8CalibrationAttempt(0u) == 1u);
    assert(CountEvents(EVENT_SET_CHANNEL) == 1u);
    assert(CountEvents(EVENT_RESET_CHANNEL) == 0u);
    AssertInputsConsumed();
}

static void TestFlaggedRouteStatusClearFails(void)
{
    ResetState();
    memory[0x001304u] = 1u;
    memory[0x00fbecu] = 0u;
    status_values[0] = 2u;
    status_values[1] = 0u;
    status_count = 2u;

    assert(RunControllerPort80F8CalibrationAttempt(0u) == 0u);
    assert(CountEvents(EVENT_YIELD) == 1u);
    assert(CountEvents(EVENT_RESET_CHANNEL) == 1u);
    assert(FindEvent(EVENT_SET_CHANNEL, 1u)->second == 0x99u);
    assert(memory[0x0011fcu] == 0u && memory[0x0011fdu] == 0u && memory[0x0011feu] == 0u && memory[0x0011ffu] == 0xfau);
    AssertInputsConsumed();
}

static void TestFlaggedRouteTimerExpirySucceeds(void)
{
    ResetState();
    memory[0x001304u] = 1u;
    memory[0x00fbecu] = 0u;
    status_values[0] = 2u;
    status_values[1] = 1u;
    status_count = 2u;
    expire_shared_timer_on_yield = 1u;

    assert(RunControllerPort80F8CalibrationAttempt(0u) == 1u);
    assert(CountEvents(EVENT_YIELD) == 1u);
    assert(CountEvents(EVENT_RESET_CHANNEL) == 0u);
    assert(memory[0x0011fcu] == 0u && memory[0x0011fdu] == 0u && memory[0x0011feu] == 0u && memory[0x0011ffu] == 0u);
    AssertInputsConsumed();
}

int main(void)
{
    TestModeZeroImmediateStatusSuccess();
    TestModeOneSecondHandshakeFailure();
    TestFourSampleErrorsFail();
    TestThreeSampleErrorsSucceed();
    TestFlaggedRouteStatusClearFails();
    TestFlaggedRouteTimerExpirySucceeds();
    return 0;
}
