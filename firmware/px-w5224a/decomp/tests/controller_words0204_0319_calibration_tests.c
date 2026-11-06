#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_words0204_0319_calibration.h"
#include "platform/firmware_memory.h"

enum
{
    EVENT_CONFIGURE,
    EVENT_PARAMETER,
    EVENT_SUBMIT,
    EVENT_LOWER_F4,
    EVENT_APPLY_SETTLE,
    EVENT_OUTCOME,
    EVENT_CALIBRATE_0319
};

typedef struct
{
    u8 kind;
    u16 first;
    u16 second;
} Event;

static u8 memory_bytes[0x20000];
static Event events[1024];
static size_t event_count;
static u16 command_results[120];
static size_t command_count;
static size_t command_index;
static u8 pulse_result;
static u8 indexed_state_result;
static u8 calibration_0319_result;

static void Record(u8 kind, u16 first, u16 second)
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
    event_count = 0u;
    command_count = 0u;
    command_index = 0u;
    pulse_result = 1u;
    indexed_state_result = 0u;
    calibration_0319_result = 1u;
    memory_bytes[0x11ef] = 2u;
    memory_bytes[0xfc14] = 0x10u;
    memory_bytes[0xf99a] = 0x24u;
    memory_bytes[0xf99b] = 0x68u;
}

static void AppendPlateau(u8 first, u8 last)
{
    u8 index;

    for(index = 0u; index < 40u; ++index)
    {
        assert(command_count < sizeof(command_results) / sizeof(command_results[0]));
        command_results[command_count++] = index >= first && index <= last ? 1000u : 100u;
    }
}

static void AppendThresholdWrapPattern(void)
{
    u8 index;

    for(index = 0u; index < 40u; ++index)
        command_results[command_count++] = index >= 10u && index <= 30u ? 0x4064u : 100u;
}

static u8 HasEvent(u8 kind)
{
    size_t index;

    for(index = 0u; index < event_count; ++index)
        if(events[index].kind == kind)
            return 1u;
    return 0u;
}

static size_t FindEvent(u8 kind, size_t start)
{
    while(start < event_count && events[start].kind != kind)
        ++start;
    assert(start < event_count);
    return start;
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

s16 AbsoluteValueI16(s16 value)
{
    return value < 0 ? (s16)-value : value;
}

void ObserveWords02040319Yield(void)
{
    if((s8)memory_bytes[0x120a] >= 0)
        memory_bytes[0x120a] = 0xffu;
    if((s8)memory_bytes[0x1220] > 0)
        memory_bytes[0x1220] = 0u;
}

void ObserveWords02040319Configure(void)
{
    Record(EVENT_CONFIGURE, 0u, 0u);
}

void ObserveWords02040319WriteParameter(u16 value)
{
    Record(EVENT_PARAMETER, value, 0u);
}

void ObserveWords02040319Submit(u16 value, u16 selector)
{
    Record(EVENT_SUBMIT, value, selector);
}

u8 ObserveWords02040319Pulse(u8 sample_count)
{
    assert(sample_count == 10u);
    return pulse_result;
}

u8 ObserveWords02040319IndexedState(u8 state)
{
    assert(state == 5u);
    return indexed_state_result;
}

void ObserveWords02040319LowerF4(void)
{
    Record(EVENT_LOWER_F4, 0u, 0u);
}

u16 ObserveWords02040319Command0E(void)
{
    assert(command_index < command_count);
    return command_results[command_index++];
}

void ObserveWords02040319ApplyAndSettle(void)
{
    Record(EVENT_APPLY_SETTLE, 0u, 0u);
}

void ObserveWords02040319Outcome(u8 stage)
{
    Record(EVENT_OUTCOME, stage, 0u);
}

u8 ObserveWords02040319Calibrate0319(void)
{
    Record(EVENT_CALIBRATE_0319, calibration_0319_result, 0u);
    return calibration_0319_result;
}

static void TestTwoStablePassesPublishSelectedWordAndFeedback(void)
{
    size_t index;

    Reset();
    AppendPlateau(10u, 30u);
    AppendPlateau(10u, 30u);
    assert(CalibrateControllerWords0204And0319Offset() == 1u);
    assert(command_index == 80u);
    assert(ReadWord(0x11c4u) == 0xf800u);
    assert(ReadWord(0x11c6u) == 0xf800u);
    assert(ReadWord(0x11c8u) == 0u);
    assert(ReadWord(0x11c0u) == 0xf800u);
    assert(ReadWord(0xf990u) == 0xf800u);
    assert(ReadWord(0x116eu) == 100u);
    assert(ReadWord(0x1180u) == 400u);
    assert(ReadWord(0x1182u) == 700u);
    assert(ReadWord(0x1184u) == 1000u);
    assert(ReadWord(0x11aau) == 700u);
    assert(ReadWord(0x11acu) == 400u);
    assert(ReadWord(0x11aeu) == 100u);
    assert(ReadWord(0x11c2u) == 0u);

    index = FindEvent(EVENT_PARAMETER, 0u);
    assert(events[index].first == 0xffecu);
    index = FindEvent(EVENT_APPLY_SETTLE, index + 1u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0xf800u && events[index].second == 0x0204u);
    index = FindEvent(EVENT_SUBMIT, index + 1u);
    assert(events[index].first == 0x2468u && events[index].second == 0x0319u);
    assert(HasEvent(EVENT_CALIBRATE_0319) == 1u);
}

static void TestCaptureFailuresPublishExactCodes(void)
{
    Reset();
    pulse_result = 0u;
    indexed_state_result = 0u;
    assert(CalibrateControllerWords0204And0319Offset() == 0u);
    assert(ReadWord(0x11beu) == 2u);
    assert(command_index == 0u);
    assert(HasEvent(EVENT_LOWER_F4) == 1u);

    Reset();
    memory_bytes[0x11ef] = 0u;
    assert(CalibrateControllerWords0204And0319Offset() == 0u);
    assert(ReadWord(0x11beu) == 3u);
    assert(command_index == 0u);

    Reset();
    command_results[0] = 0u;
    command_count = 1u;
    assert(CalibrateControllerWords0204And0319Offset() == 0u);
    assert(ReadWord(0x11beu) == 4u);
    assert(command_index == 1u);
}

static void TestWrappedProminencePublishesSelectionFailure(void)
{
    Reset();
    AppendThresholdWrapPattern();
    assert(CalibrateControllerWords0204And0319Offset() == 0u);
    assert(ReadWord(0x11beu) == 5u);
    assert(command_index == 40u);
}

static void TestThirdPassChoosesClosestCandidatePair(void)
{
    Reset();
    AppendPlateau(4u, 8u);
    AppendPlateau(20u, 24u);
    AppendPlateau(21u, 25u);
    assert(CalibrateControllerWords0204And0319Offset() == 1u);
    assert(command_index == 120u);
    assert(ReadWord(0x11c4u) == 0xdc00u);
    assert(ReadWord(0x11c6u) == 0xfc00u);
    assert(ReadWord(0x11c8u) == 0xfe00u);
    assert(ReadWord(0x11c0u) == 0xfd00u);
}

static void TestIndexedTerminalRouteClearsFlagsAndSkips0319(void)
{
    Reset();
    AppendPlateau(10u, 30u);
    AppendPlateau(10u, 30u);
    indexed_state_result = 1u;
    memory_bytes[0x1008] = 1u;
    memory_bytes[0x1306] = 0x40u;
    memory_bytes[0xfab4] = 0xffu;
    assert(CalibrateControllerWords0204And0319Offset() == 1u);
    assert(ReadWord(0xfab8u) == 0xf800u);
    assert(memory_bytes[0xfab4] == 0xdbu);
    assert(HasEvent(EVENT_OUTCOME) == 1u);
    assert(HasEvent(EVENT_CALIBRATE_0319) == 0u);
}

static void TestSubordinateFailureSetsDriveControlBit(void)
{
    Reset();
    AppendPlateau(10u, 30u);
    AppendPlateau(10u, 30u);
    calibration_0319_result = 0u;
    memory_bytes[0x1304] = 1u;
    assert(CalibrateControllerWords0204And0319Offset() == 0u);
    assert(memory_bytes[0x1304] == 5u);
}

int main(void)
{
    TestTwoStablePassesPublishSelectedWordAndFeedback();
    TestCaptureFailuresPublishExactCodes();
    TestWrappedProminencePublishesSelectionFailure();
    TestThirdPassChoosesClosestCandidatePair();
    TestIndexedTerminalRouteClearsFlagsAndSkips0319();
    TestSubordinateFailureSetsDriveControlBit();
    puts("controller words 0204/0319 calibration tests passed");
    return 0;
}
