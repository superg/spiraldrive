#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_current_state_calibration.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_YIELD,
    EVENT_APPLY_STORED,
    EVENT_PULSE_TEST,
    EVENT_RUN_FULL,
    EVENT_CAPTURE_REFERENCES,
    EVENT_SET_P3_HIGH
};

struct event_record
{
    enum event_kind kind;
    u8 argument;
};

static u8 memory[1u << 24];
static struct event_record events[24];
static size_t event_count;
static u8 apply_result;
static u8 pulse_result;
static u8 full_result;
static u8 references_result;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void RecordEvent(enum event_kind kind, u8 argument)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].argument = argument;
    event_count++;
}

void ObserveCurrentStateCalibrationYield(void)
{
    RecordEvent(EVENT_YIELD, 0u);
}

u8 ObserveCurrentStateCalibrationApplyStoredWord(void)
{
    RecordEvent(EVENT_APPLY_STORED, 0u);
    return apply_result;
}

u8 ObserveCurrentStateCalibrationPulseTest(u8 sample_count)
{
    RecordEvent(EVENT_PULSE_TEST, sample_count);
    return pulse_result;
}

u8 ObserveCurrentStateCalibrationRunFull(void)
{
    RecordEvent(EVENT_RUN_FULL, 0u);
    return full_result;
}

u8 ObserveCurrentStateCalibrationCaptureReferences(void)
{
    RecordEvent(EVENT_CAPTURE_REFERENCES, 0u);
    return references_result;
}

void ObserveCurrentStateCalibrationSetP3High(void)
{
    RecordEvent(EVENT_SET_P3_HIGH, 0u);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    apply_result = 1u;
    pulse_result = 1u;
    full_result = 1u;
    references_result = 1u;
    memory[0x001304u] = 1u;
}

static void AssertEvent(size_t index, enum event_kind kind, u8 argument)
{
    assert(index < event_count);
    assert(events[index].kind == kind);
    assert(events[index].argument == argument);
}

static void TestInactiveGate(void)
{
    ResetState();
    memory[0x001006u] = 1u;
    memory[0x001304u] = 0x80u;
    assert(RunControllerCalibrationForCurrentDriveState() == 1u);
    assert(event_count == 0u);
}

static void TestDisabledStateOneCodeFive(void)
{
    ResetState();
    memory[0x001308u] = 1u;
    memory[0x00130au] = 5u;

    assert(RunControllerCalibrationForCurrentDriveState() == 1u);
    assert(memory[0x00fab1u] == 1u);
    assert(event_count == 3u);
    AssertEvent(0u, EVENT_RUN_FULL, 0u);
    AssertEvent(1u, EVENT_SET_P3_HIGH, 0u);
    AssertEvent(2u, EVENT_CAPTURE_REFERENCES, 0u);

    ResetState();
    memory[0x001308u] = 1u;
    memory[0x00130au] = 5u;
    memory[0x001304u] = 0x81u;
    full_result = 0u;
    assert(RunControllerCalibrationForCurrentDriveState() == 0u);
    assert(memory[0x001304u] == 0x85u);
    assert(event_count == 1u);
}

static void TestDisabledStoredAndFullSelection(void)
{
    ResetState();
    memory[0x001308u] = 1u;
    memory[0x00130au] = 4u;
    apply_result = 2u;
    assert(RunControllerCalibrationForCurrentDriveState() == 1u);
    AssertEvent(0u, EVENT_APPLY_STORED, 0u);

    ResetState();
    memory[0x001308u] = 2u;
    memory[0x00fab4u] = 0x20u;
    full_result = 2u;
    assert(RunControllerCalibrationForCurrentDriveState() == 0u);
    assert(memory[0x001304u] == 5u);
    AssertEvent(0u, EVENT_RUN_FULL, 0u);
}

static void TestEnabledStateOneCodeFive(void)
{
    ResetState();
    memory[0x001006u] = 1u;
    memory[0x001308u] = 1u;
    memory[0x00130au] = 5u;
    pulse_result = 1u;

    assert(RunControllerCalibrationForCurrentDriveState() == 1u);
    assert(memory[0x00fab1u] == 0u);
    assert(event_count == 4u);
    AssertEvent(0u, EVENT_YIELD, 0u);
    AssertEvent(1u, EVENT_PULSE_TEST, 10u);
    AssertEvent(2u, EVENT_RUN_FULL, 0u);
    AssertEvent(3u, EVENT_CAPTURE_REFERENCES, 0u);

    ResetState();
    memory[0x001006u] = 1u;
    memory[0x001308u] = 1u;
    memory[0x00130au] = 5u;
    pulse_result = 2u;
    apply_result = 1u;
    assert(RunControllerCalibrationForCurrentDriveState() == 1u);
    assert(memory[0x00fab1u] == 1u);
    AssertEvent(1u, EVENT_PULSE_TEST, 10u);
    AssertEvent(2u, EVENT_APPLY_STORED, 0u);
    AssertEvent(3u, EVENT_SET_P3_HIGH, 0u);
}

static void TestEnabledStateAndBitSelection(void)
{
    ResetState();
    memory[0x001006u] = 1u;
    memory[0x001308u] = 2u;
    memory[0x00fab4u] = 0u;
    assert(RunControllerCalibrationForCurrentDriveState() == 1u);
    AssertEvent(0u, EVENT_YIELD, 0u);
    AssertEvent(1u, EVENT_APPLY_STORED, 0u);

    ResetState();
    memory[0x001006u] = 1u;
    memory[0x001308u] = 2u;
    memory[0x00fab4u] = 4u;
    pulse_result = 2u;
    assert(RunControllerCalibrationForCurrentDriveState() == 1u);
    assert(event_count == 3u);
    AssertEvent(1u, EVENT_PULSE_TEST, 10u);
    AssertEvent(2u, EVENT_CAPTURE_REFERENCES, 0u);

    ResetState();
    memory[0x001006u] = 1u;
    memory[0x001308u] = 2u;
    memory[0x00fab4u] = 4u;
    pulse_result = 1u;
    full_result = 0u;
    assert(RunControllerCalibrationForCurrentDriveState() == 0u);
    assert(memory[0x001304u] == 5u);
}

static void TestReferenceFailure(void)
{
    ResetState();
    memory[0x001304u] = 0x41u;
    memory[0x001308u] = 1u;
    memory[0x00130au] = 4u;
    references_result = 0u;

    assert(RunControllerCalibrationForCurrentDriveState() == 0u);
    assert(memory[0x001304u] == 0x45u);
    assert(event_count == 2u);
    AssertEvent(0u, EVENT_APPLY_STORED, 0u);
    AssertEvent(1u, EVENT_CAPTURE_REFERENCES, 0u);
}

int main(void)
{
    TestInactiveGate();
    TestDisabledStateOneCodeFive();
    TestDisabledStoredAndFullSelection();
    TestEnabledStateOneCodeFive();
    TestEnabledStateAndBitSelection();
    TestReferenceFailure();
    return 0;
}
