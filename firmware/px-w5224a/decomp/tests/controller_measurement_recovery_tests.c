#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_measurement_recovery.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_ACQUIRE,
    EVENT_RESET,
    EVENT_INITIALIZE,
    EVENT_ENABLE,
    EVENT_TOLERANCE,
    EVENT_CONFIGURE,
    EVENT_APPLY,
    EVENT_WRITE_D4_E2,
    EVENT_RAISE_F4,
    EVENT_POLL,
    EVENT_RESTORE_D4_E2,
    EVENT_LOWER_F4,
    EVENT_TAIL,
    EVENT_SERIES
};

struct event_record
{
    enum event_kind kind;
    u8 argument;
};

static u8 memory[1u << 24];
static struct event_record events[128];
static size_t event_count;
static u8 acquire_results[16];
static size_t acquire_count;
static size_t acquire_index;
static u8 initialize_results[16];
static size_t initialize_count;
static size_t initialize_index;
static u8 tolerance_results[16];
static size_t tolerance_count;
static size_t tolerance_index;
static u8 poll_status;
static u8 series_result;

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

u8 ObserveMeasurementRecoveryAcquirePosition(void)
{
    RecordEvent(EVENT_ACQUIRE, 0u);
    assert(acquire_index < acquire_count);
    return acquire_results[acquire_index++];
}

void ObserveMeasurementRecoveryResetDriveControl(void)
{
    RecordEvent(EVENT_RESET, 0u);
}

u8 ObserveMeasurementRecoveryInitializeController(void)
{
    RecordEvent(EVENT_INITIALIZE, 0u);
    assert(initialize_index < initialize_count);
    return initialize_results[initialize_index++];
}

void ObserveMeasurementRecoverySetOperatingEnable(u8 enabled)
{
    RecordEvent(EVENT_ENABLE, enabled);
}

u8 ObserveMeasurementRecoveryCheckTolerance(u8 restart_timeout)
{
    RecordEvent(EVENT_TOLERANCE, restart_timeout);
    assert(tolerance_index < tolerance_count);
    return tolerance_results[tolerance_index++];
}

void ObserveMeasurementRecoveryConfigurePorts(u8 mode_selector)
{
    RecordEvent(EVENT_CONFIGURE, mode_selector);
}

void ObserveMeasurementRecoveryApplyChannels(void)
{
    RecordEvent(EVENT_APPLY, 0u);
}

void ObserveMeasurementRecoveryWriteD4E2(u8 code)
{
    RecordEvent(EVENT_WRITE_D4_E2, code);
}

void ObserveMeasurementRecoveryRaiseF4(void)
{
    RecordEvent(EVENT_RAISE_F4, 0u);
}

void ObserveMeasurementRecoveryPollPacket(void)
{
    RecordEvent(EVENT_POLL, 0u);
    memory[0x00ef7fu] = poll_status;
}

void ObserveMeasurementRecoveryRestoreD4E2(void)
{
    RecordEvent(EVENT_RESTORE_D4_E2, 0u);
}

void ObserveMeasurementRecoveryLowerF4(void)
{
    RecordEvent(EVENT_LOWER_F4, 0u);
}

void ObserveMeasurementRecoveryEvaluateTail(void)
{
    RecordEvent(EVENT_TAIL, 0u);
}

u8 ObserveMeasurementRecoveryEvaluateSeries(u8 fallback)
{
    RecordEvent(EVENT_SERIES, fallback);
    return series_result;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(acquire_results, 0, sizeof(acquire_results));
    memset(initialize_results, 0, sizeof(initialize_results));
    memset(tolerance_results, 0, sizeof(tolerance_results));
    event_count = 0u;
    acquire_count = 0u;
    acquire_index = 0u;
    initialize_count = 0u;
    initialize_index = 0u;
    tolerance_count = 0u;
    tolerance_index = 0u;
    poll_status = 0u;
    series_result = 0u;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void AssertEvent(size_t index, enum event_kind kind, u8 argument)
{
    assert(index < event_count);
    assert(events[index].kind == kind);
    assert(events[index].argument == argument);
}

static void TestAcquisitionFailureExhaustion(void)
{
    size_t index;

    ResetState();
    memory[0x000e47u] = 4u;
    memory[0x000e41u] = 0x22u;
    WriteU32(0x00103dul, 100ul);
    acquire_count = 4u;
    initialize_count = 3u;
    for(index = 0u; index < initialize_count; ++index)
        initialize_results[index] = 1u;

    RetryControllerMeasurementEvaluationWithReadPositionRecovery();

    assert(acquire_index == 4u);
    assert(initialize_index == 3u);
    assert(memory[0x000e4bu] == 4u && memory[0x000e44u] == 4u);
    assert(ReadU32(0x000e3bul) == 80ul);
    assert(memory[0x00ef7eu] == 0x22u);
    assert(memory[0x00ef7fu] == 4u);
    assert(memory[0x000e40u] == 0u && memory[0x000fafu] == 0u);
    assert(event_count == 13u);
    AssertEvent(0u, EVENT_ACQUIRE, 0u);
    AssertEvent(1u, EVENT_RESET, 0u);
    AssertEvent(2u, EVENT_INITIALIZE, 0u);
    AssertEvent(3u, EVENT_ENABLE, 1u);
    AssertEvent(12u, EVENT_ACQUIRE, 0u);
}

static void TestControllerInitializationFailure(void)
{
    ResetState();
    acquire_count = 1u;
    initialize_count = 1u;

    RetryControllerMeasurementEvaluationWithReadPositionRecovery();

    assert(memory[0x000e41u] == 7u);
    assert(memory[0x00ef7eu] == 7u);
    assert(memory[0x00ef7fu] == 4u);
    assert(memory[0x000e40u] == 0u && memory[0x000fafu] == 0u);
    assert(event_count == 3u);
    AssertEvent(0u, EVENT_ACQUIRE, 0u);
    AssertEvent(1u, EVENT_RESET, 0u);
    AssertEvent(2u, EVENT_INITIALIZE, 0u);
}

static void TestToleranceRestartAndStatusThree(void)
{
    ResetState();
    memory[0x000e47u] = 8u;
    memory[0x001039u] = 0xffu;
    WriteU32(0x00103dul, 100ul);
    acquire_results[0] = 1u;
    acquire_results[1] = 1u;
    acquire_count = 2u;
    tolerance_results[0] = 0u;
    tolerance_results[1] = 1u;
    tolerance_count = 2u;
    poll_status = 3u;

    RetryControllerMeasurementEvaluationWithReadPositionRecovery();

    assert(memory[0x000e4bu] == 8u && memory[0x000e44u] == 8u);
    assert(ReadU32(0x000e3bul) == 60ul);
    assert(memory[0x000e40u] == 0u);
    assert(memory[0x001301u] == 0u);
    assert(memory[0x001039u] == 0xf6u);
    assert(memory[0x00ef7eu] == 0x1eu && memory[0x000fafu] == 0u);
    AssertEvent(1u, EVENT_TOLERANCE, 1u);
    AssertEvent(3u, EVENT_TOLERANCE, 0u);
    AssertEvent(4u, EVENT_CONFIGURE, 0u);
    AssertEvent(6u, EVENT_WRITE_D4_E2, 0x74u);
}

static void TestMeasurementStatusFourExhaustion(void)
{
    size_t index;
    unsigned poll_events = 0u;

    ResetState();
    acquire_count = 4u;
    for(index = 0u; index < acquire_count; ++index)
        acquire_results[index] = 2u;
    poll_status = 4u;

    RetryControllerMeasurementEvaluationWithReadPositionRecovery();

    for(index = 0u; index < event_count; ++index)
        if(events[index].kind == EVENT_POLL)
            poll_events++;
    assert(acquire_index == 4u && poll_events == 4u);
    assert(memory[0x000e41u] == 5u);
    assert(memory[0x00ef7eu] == 5u);
    assert(memory[0x000fafu] == 0u);
    assert(memory[0x000e40u] == 1u);
}

static void TestSeriesFallbackValues(void)
{
    ResetState();
    acquire_results[0] = 1u;
    acquire_count = 1u;
    tolerance_results[0] = 1u;
    tolerance_count = 1u;
    series_result = 1u;
    RetryControllerMeasurementEvaluationWithReadPositionRecovery();
    AssertEvent(event_count - 1u, EVENT_SERIES, 1u);

    ResetState();
    acquire_results[0] = 1u;
    acquire_results[1] = 1u;
    acquire_count = 2u;
    tolerance_results[0] = 0u;
    tolerance_results[1] = 1u;
    tolerance_count = 2u;
    series_result = 1u;
    RetryControllerMeasurementEvaluationWithReadPositionRecovery();
    AssertEvent(event_count - 1u, EVENT_SERIES, 0u);
}

int main(void)
{
    TestAcquisitionFailureExhaustion();
    TestControllerInitializationFailure();
    TestToleranceRestartAndStatusThree();
    TestMeasurementStatusFourExhaustion();
    TestSeriesFallbackValues();
    return 0;
}
