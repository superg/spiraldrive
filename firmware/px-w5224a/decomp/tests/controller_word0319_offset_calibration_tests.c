#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_word0319_offset_calibration.h"
#include "platform/firmware_memory.h"

enum
{
    EVENT_PULSE,
    EVENT_SET_CHANNEL,
    EVENT_WRITE_PORT,
    EVENT_READ_WORD,
    EVENT_SUBMIT,
    EVENT_UPDATE_CONFIGURATION,
    EVENT_LOAD_FC,
    EVENT_RESET_CHANNEL,
    EVENT_ENTER_MEASUREMENT,
    EVENT_COMMAND_17,
    EVENT_DIVIDE,
    EVENT_INDEXED_STATE,
    EVENT_UPDATE_OUTCOME,
    EVENT_LEAVE_MEASUREMENT,
    EVENT_WAIT,
    EVENT_CLEAR_D1,
    EVENT_WRITE_D2_D3,
    EVENT_WRITE_80E4,
    EVENT_APPLY_D1,
    EVENT_WRITE_D1_D2_D3,
    EVENT_MAP_CHANNEL3,
    EVENT_SELECT_FC
};

typedef struct
{
    u8 kind;
    u32 first;
    u32 second;
} Event;

static u8 memory_bytes[0x20000];
static Event events[160];
static size_t event_count;
static u16 controller_word_0319;
static u16 controller_word_0339;
static u8 load_fc_result;
static u8 indexed_state_result;
static u8 mapped_channel_3;
static u8 command_results[8];
static s8 timer_after_command[8];
static size_t command_count;
static size_t command_index;
static u8 pulse_results[4];
static size_t pulse_count;
static size_t pulse_index;

static void Record(u8 kind, u32 first, u32 second)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    ++event_count;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static void Reset(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(events, 0, sizeof(events));
    memset(command_results, 0, sizeof(command_results));
    memset(timer_after_command, 0x7f, sizeof(timer_after_command));
    memset(pulse_results, 0, sizeof(pulse_results));
    event_count = 0u;
    controller_word_0319 = 0x1000u;
    controller_word_0339 = 0x5678u;
    load_fc_result = 1u;
    indexed_state_result = 0u;
    mapped_channel_3 = 0x5au;
    command_count = 0u;
    command_index = 0u;
    pulse_count = 0u;
    pulse_index = 0u;
    memory_bytes[0x0e43] = 0x20u;
    memory_bytes[0x1006] = 1u;
    memory_bytes[0xf98e] = 0x6cu;
}

static size_t FindEvent(u8 kind, size_t start)
{
    while(start < event_count && events[start].kind != kind)
        ++start;
    assert(start < event_count);
    return start;
}

static u8 HasEvent(u8 kind)
{
    size_t index;

    for(index = 0u; index < event_count; ++index)
        if(events[index].kind == kind)
            return 1u;
    return 0u;
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

u8 ObserveWord0319PulseTest(u8 sample_count)
{
    u8 result;

    assert(pulse_index < pulse_count);
    result = pulse_results[pulse_index++];
    Record(EVENT_PULSE, sample_count, result);
    return result;
}

void ObserveWord0319SetChannel(u8 channel, u8 value)
{
    Record(EVENT_SET_CHANNEL, channel, value);
}

void ObserveWord0319WritePort(fw_addr_t address, u8 value)
{
    Record(EVENT_WRITE_PORT, address, value);
}

u16 ObserveWord0319ReadControllerWord(u16 selector)
{
    Record(EVENT_READ_WORD, selector, 0u);
    if(selector == 0x0319u)
        return controller_word_0319;
    assert(selector == 0x0339u);
    return controller_word_0339;
}

void ObserveWord0319SubmitWord(u16 value, u16 selector)
{
    Record(EVENT_SUBMIT, value, selector);
}

void ObserveWord0319UpdateConfiguration(void)
{
    Record(EVENT_UPDATE_CONFIGURATION, 0u, 0u);
}

u8 ObserveWord0319LoadParameterFC(void)
{
    Record(EVENT_LOAD_FC, load_fc_result, 0u);
    return load_fc_result;
}

void ObserveWord0319ResetChannel(u8 channel)
{
    Record(EVENT_RESET_CHANNEL, channel, 0u);
}

void ObserveWord0319EnterMeasurement(void)
{
    Record(EVENT_ENTER_MEASUREMENT, 0u, 0u);
}

u8 ObserveWord0319RunCommand17(void)
{
    u8 result;
    s8 timer_value;

    assert(command_index < command_count);
    result = command_results[command_index];
    timer_value = timer_after_command[command_index];
    ++command_index;
    if(timer_value != 0x7f)
        memory_bytes[0x120a] = (u8)timer_value;
    Record(EVENT_COMMAND_17, result, (u8)timer_value);
    return result;
}

s16 ObserveWord0319Divide(s16 dividend, u16 divisor)
{
    Record(EVENT_DIVIDE, (u16)dividend, divisor);
    return (s16)(dividend / (s16)divisor);
}

u8 ObserveWord0319IsIndexedState(u8 state)
{
    Record(EVENT_INDEXED_STATE, state, indexed_state_result);
    return indexed_state_result;
}

void ObserveWord0319UpdateOutcome(u8 stage)
{
    Record(EVENT_UPDATE_OUTCOME, stage, 0u);
}

void ObserveWord0319LeaveMeasurement(void)
{
    Record(EVENT_LEAVE_MEASUREMENT, 0u, 0u);
}

void ObserveWord0319Wait(u16 duration, u8 yield_while_waiting)
{
    Record(EVENT_WAIT, duration, yield_while_waiting);
}

void ObserveWord0319ClearD1Bit6(void)
{
    Record(EVENT_CLEAR_D1, 0u, 0u);
}

void ObserveWord0319WriteD2D3(void)
{
    Record(EVENT_WRITE_D2_D3, 0u, 0u);
}

void ObserveWord0319WritePort80E4(void)
{
    Record(EVENT_WRITE_80E4, 0u, 0u);
}

void ObserveWord0319ApplyD1(void)
{
    Record(EVENT_APPLY_D1, 0u, 0u);
}

void ObserveWord0319WriteD1D2D3(void)
{
    Record(EVENT_WRITE_D1_D2_D3, 0u, 0u);
}

u8 ObserveWord0319MapChannel3(u8 operating_code)
{
    Record(EVENT_MAP_CHANNEL3, operating_code, mapped_channel_3);
    return mapped_channel_3;
}

void ObserveWord0319SelectParameterFC(u8 operating_code)
{
    Record(EVENT_SELECT_FC, operating_code, 0u);
}

static void TestPrerequisiteRequiresExactOneAndRestoresSavedState(void)
{
    size_t index;

    Reset();
    load_fc_result = 2u;
    assert(CalibrateControllerWord0319Offset() == 0u);
    assert(ReadWord(0xf99au) == 0x1000u);
    assert(command_index == 0u);

    index = FindEvent(EVENT_LOAD_FC, 0u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x5678u && events[index].second == 0x0339u);
    index = FindEvent(EVENT_CLEAR_D1, index + 1u);
    index = FindEvent(EVENT_WRITE_D2_D3, index + 1u);
    index = FindEvent(EVENT_WRITE_80E4, index + 1u);
    index = FindEvent(EVENT_APPLY_D1, index + 1u);
    index = FindEvent(EVENT_WRITE_D1_D2_D3, index + 1u);
    index = FindEvent(EVENT_SET_CHANNEL, index + 1u);
    assert(events[index].first == 2u && events[index].second == 0x6cu);
    index = FindEvent(EVENT_MAP_CHANNEL3, index + 1u);
    assert(events[index].first == 0x20u);
    index = FindEvent(EVENT_SET_CHANNEL, index + 1u);
    assert(events[index].first == 3u && events[index].second == 0x5au);
}

static void TestSentinelRestoresBaselineAndReturnsFalse(void)
{
    size_t index;

    Reset();
    command_results[0] = 0x7fu;
    command_count = 1u;
    assert(CalibrateControllerWord0319Offset() == 0u);
    assert(ReadWord(0xf992u) == 0x007fu);

    index = FindEvent(EVENT_COMMAND_17, 0u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x1000u && events[index].second == 0x0319u);
    index = FindEvent(EVENT_LEAVE_MEASUREMENT, index + 1u);
    index = FindEvent(EVENT_WAIT, index + 1u);
    assert(events[index].first == 4000u && events[index].second == 0u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0u && events[index].second == 0x0339u);
    index = FindEvent(EVENT_SET_CHANNEL, index + 1u);
    assert(events[index].first == 2u && events[index].second == 0xc1u);
}

static void TestPositiveCorrectionPublishesOffsetAndOutcome(void)
{
    size_t index;

    Reset();
    indexed_state_result = 1u;
    memory_bytes[0xfab4] = 0xffu;
    command_results[0] = 2u;
    command_results[1] = 0u;
    command_count = 2u;
    assert(CalibrateControllerWord0319Offset() == 1u);
    assert(ReadWord(0xf992u) == 0x0100u);
    assert(ReadWord(0xfabau) == 0x0100u);
    assert(memory_bytes[0xfab4] == 0xedu);

    index = FindEvent(EVENT_DIVIDE, 0u);
    assert(events[index].first == 2u && events[index].second == 2u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x1100u && events[index].second == 0x0319u);
    index = FindEvent(EVENT_UPDATE_OUTCOME, index + 1u);
    assert(events[index].first == 2u);
}

static void TestNegativeCorrectionUsesSignedDivisionAndWrap(void)
{
    size_t index;

    Reset();
    command_results[0] = 0xfeu;
    command_results[1] = 0u;
    command_count = 2u;
    assert(CalibrateControllerWord0319Offset() == 1u);
    assert(ReadWord(0xf992u) == 0xff00u);

    index = FindEvent(EVENT_DIVIDE, 0u);
    assert(events[index].first == 0xfffeu && events[index].second == 2u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x0f00u && events[index].second == 0x0319u);
}

static void TestZeroAndNegativeAsynchronousTimerResults(void)
{
    Reset();
    command_results[0] = 2u;
    timer_after_command[0] = 0;
    command_count = 1u;
    assert(CalibrateControllerWord0319Offset() == 1u);
    assert(ReadWord(0xf992u) == 0x0100u);
    assert(HasEvent(EVENT_INDEXED_STATE) == 0u);
}

static void TestNegativeTimerForcesFailureSentinel(void)
{
    Reset();
    command_results[0] = 2u;
    timer_after_command[0] = -1;
    command_count = 1u;
    assert(CalibrateControllerWord0319Offset() == 0u);
    assert(ReadWord(0xf992u) == 0x007fu);
}

static void TestExactIndexedStateResultIsRequired(void)
{
    Reset();
    indexed_state_result = 2u;
    command_results[0] = 0u;
    command_count = 1u;
    assert(CalibrateControllerWord0319Offset() == 1u);
    assert(ReadWord(0xfabau) == 0u);
    assert(HasEvent(EVENT_UPDATE_OUTCOME) == 0u);
}

static void TestStableZeroPulseAdvancesParameterFc(void)
{
    size_t index;

    Reset();
    memory_bytes[0x1008] = 2u;
    memory_bytes[0xfc0f] = 0xf0u;
    pulse_results[0] = 0u;
    pulse_results[1] = 0u;
    pulse_count = 2u;
    command_results[0] = 0u;
    command_count = 1u;
    assert(CalibrateControllerWord0319Offset() == 1u);
    assert(pulse_index == 2u);
    assert(memory_bytes[0xfc0f] == 0x10u);
    index = FindEvent(EVENT_SELECT_FC, 0u);
    assert(events[index].first == 4u);
    assert(HasEvent(EVENT_UPDATE_OUTCOME) == 0u);
}

int main(void)
{
    TestPrerequisiteRequiresExactOneAndRestoresSavedState();
    TestSentinelRestoresBaselineAndReturnsFalse();
    TestPositiveCorrectionPublishesOffsetAndOutcome();
    TestNegativeCorrectionUsesSignedDivisionAndWrap();
    TestZeroAndNegativeAsynchronousTimerResults();
    TestNegativeTimerForcesFailureSentinel();
    TestExactIndexedStateResultIsRequired();
    TestStableZeroPulseAdvancesParameterFc();
    puts("controller word 0319 offset calibration tests passed");
    return 0;
}
