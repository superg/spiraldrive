#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_state6_operating_path.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_PROBE,
    EVENT_VALIDATE,
    EVENT_ENTER_STATE5,
    EVENT_HOME,
    EVENT_PROFILE,
    EVENT_WRITE_PORT,
    EVENT_RETRY,
    EVENT_SET_P1,
    EVENT_PULSE,
    EVENT_SET_CHANNEL,
    EVENT_RESET_CHANNEL,
    EVENT_INDEXED_STATE,
    EVENT_CLEAR_P3,
    EVENT_CALIBRATE_FB,
    EVENT_SET_CONTROLLER_FLAG,
    EVENT_FINALIZE,
    EVENT_LOAD_PROFILE,
    EVENT_CONFIGURE_8006,
    EVENT_APPLY_CHANNELS,
    EVENT_CONFIGURE_RANGE,
    EVENT_READ_P0
};

struct event_record
{
    enum event_kind kind;
    u32 first;
    u16 second;
};

static u8 memory[1u << 24];
static struct event_record events[96];
static size_t event_count;
static u8 validate_result;
static u8 enter_result;
static u8 retry_results[4];
static size_t retry_count;
static size_t retry_index;
static u8 indexed_result;
static u8 calibrate_result;
static u8 finalize_result;
static u8 p0_results[4];
static size_t p0_count;
static size_t p0_index;
static u8 expire_countdown_after_p0;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void RecordEvent(enum event_kind kind, u32 first, u16 second)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    event_count++;
}

void ObserveState6ProbeController(void)
{
    RecordEvent(EVENT_PROBE, 0u, 0u);
}

u8 ObserveState6ValidateCalibrationPath(void)
{
    RecordEvent(EVENT_VALIDATE, 0u, 0u);
    return validate_result;
}

u8 ObserveState6EnterControllerState5(void)
{
    RecordEvent(EVENT_ENTER_STATE5, 0u, 0u);
    return enter_result;
}

void ObserveState6HomePickup(void)
{
    RecordEvent(EVENT_HOME, 0u, 0u);
}

void ObserveState6ConfigureProfile(u8 enabled, u16 profile_value)
{
    RecordEvent(EVENT_PROFILE, enabled, profile_value);
}

void ObserveState6WriteControllerPort(fw_addr_t address, u8 value)
{
    RecordEvent(EVENT_WRITE_PORT, address, value);
}

u8 ObserveState6RetryPortCalibration(u8 calibration_mode)
{
    RecordEvent(EVENT_RETRY, calibration_mode, 0u);
    assert(retry_index < retry_count);
    return retry_results[retry_index++];
}

void ObserveState6SetP1TimerPins(void)
{
    RecordEvent(EVENT_SET_P1, 0u, 0u);
}

void ObserveState6PulseControllerWord(u16 duration_units)
{
    RecordEvent(EVENT_PULSE, duration_units, 0u);
}

void ObserveState6SetIndexedChannel(u8 channel, u8 value)
{
    RecordEvent(EVENT_SET_CHANNEL, channel, value);
}

void ObserveState6ResetIndexedChannel(u8 channel)
{
    RecordEvent(EVENT_RESET_CHANNEL, channel, 0u);
}

u8 ObserveState6IsIndexedState(u8 state_code)
{
    RecordEvent(EVENT_INDEXED_STATE, state_code, 0u);
    return indexed_result;
}

void ObserveState6ClearP3Pins(void)
{
    RecordEvent(EVENT_CLEAR_P3, 0u, 0u);
}

u8 ObserveState6CalibrateParameterFb(void)
{
    RecordEvent(EVENT_CALIBRATE_FB, 0u, 0u);
    return calibrate_result;
}

void ObserveState6SetControllerFlag(u8 enabled)
{
    RecordEvent(EVENT_SET_CONTROLLER_FLAG, enabled, 0u);
}

u8 ObserveState6FinalizeCalibration(void)
{
    RecordEvent(EVENT_FINALIZE, 0u, 0u);
    return finalize_result;
}

void ObserveState6LoadOperatingProfile(u8 profile_index)
{
    RecordEvent(EVENT_LOAD_PROFILE, profile_index, 0u);
}

void ObserveState6ConfigurePort8006(u8 enabled)
{
    RecordEvent(EVENT_CONFIGURE_8006, enabled, 0u);
}

void ObserveState6ApplyOperatingChannels(void)
{
    RecordEvent(EVENT_APPLY_CHANNELS, 0u, 0u);
}

void ObserveState6ConfigureOperatingRange(void)
{
    RecordEvent(EVENT_CONFIGURE_RANGE, 0u, 0u);
}

u8 ObserveState6ReadP0Bit7(void)
{
    u8 result;

    RecordEvent(EVENT_READ_P0, 0u, 0u);
    assert(p0_index < p0_count);
    result = p0_results[p0_index++];
    if(expire_countdown_after_p0 != 0u)
        memory[0x001209u] = 0u;
    return result;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(retry_results, 0, sizeof(retry_results));
    memset(p0_results, 0, sizeof(p0_results));
    event_count = 0u;
    validate_result = 1u;
    enter_result = 1u;
    retry_count = 0u;
    retry_index = 0u;
    indexed_result = 1u;
    calibrate_result = 1u;
    finalize_result = 1u;
    p0_count = 0u;
    p0_index = 0u;
    expire_countdown_after_p0 = 0u;
    memory[0x00fbecu] = 1u;
}

static size_t CountEvents(enum event_kind kind)
{
    size_t count = 0u;
    size_t index;

    for(index = 0u; index < event_count; ++index)
        if(events[index].kind == kind)
            count++;
    return count;
}

static void TestValidationAndState5Failures(void)
{
    ResetState();
    validate_result = 0u;
    assert(EstablishControllerDriveState6OperatingPath() == 0u);
    assert(event_count == 2u);
    assert(events[0].kind == EVENT_PROBE);
    assert(events[1].kind == EVENT_VALIDATE);

    ResetState();
    enter_result = 0u;
    assert(EstablishControllerDriveState6OperatingPath() == 0u);
    assert(event_count == 3u);
    assert(events[2].kind == EVENT_ENTER_STATE5);
}

static void TestInitialRetryFailureTogglesAtipState(void)
{
    ResetState();
    retry_count = 1u;
    retry_results[0] = 0u;
    memory[0x00fb98u] = 0u;
    memory[0x001304u] = 0x80u;

    assert(EstablishControllerDriveState6OperatingPath() == 0u);
    assert(memory[0x00fb98u] == 2u);
    assert(memory[0x001304u] == 0x84u);
    assert(CountEvents(EVENT_HOME) == 0u);

    ResetState();
    retry_count = 1u;
    retry_results[0] = 2u;
    memory[0x00fb98u] = 7u;
    assert(EstablishControllerDriveState6OperatingPath() == 0u);
    assert(memory[0x00fb98u] == 0u);
}

static void TestSecondRetryRecoveryAndParameterFailure(void)
{
    ResetState();
    memory[0x00fbecu] = 0u;
    memory[0x001303u] = 1u;
    retry_count = 2u;
    retry_results[0] = 0u;
    retry_results[1] = 1u;
    indexed_result = 0u;
    calibrate_result = 0u;

    assert(EstablishControllerDriveState6OperatingPath() == 0u);
    assert(memory[0x00fbecu] == 2u);
    assert(memory[0x001300u] == 6u);
    assert(memory[0x001303u] == 0x41u);
    assert(memory[0x001304u] == 4u);
    assert(CountEvents(EVENT_HOME) == 2u);
    assert(CountEvents(EVENT_PROFILE) == 1u);
    assert(CountEvents(EVENT_WRITE_PORT) == 1u);
    assert(CountEvents(EVENT_SET_P1) == 1u);
    assert(CountEvents(EVENT_PULSE) == 1u);
    assert(CountEvents(EVENT_SET_CHANNEL) == 1u);
    assert(CountEvents(EVENT_CLEAR_P3) == 1u);
    assert(CountEvents(EVENT_SET_CONTROLLER_FLAG) == 1u);
    assert(events[4].kind == EVENT_PROFILE && events[4].first == 0u && events[4].second == 0x0400u);
    assert(events[5].kind == EVENT_WRITE_PORT && events[5].first == 0x01849dul && events[5].second == 0u);
}

static void TestMandatoryHomingRetryFailure(void)
{
    ResetState();
    memory[0x00fbecu] = 0u;
    retry_count = 2u;
    retry_results[0] = 1u;
    retry_results[1] = 0u;

    assert(EstablishControllerDriveState6OperatingPath() == 0u);
    assert(memory[0x001304u] == 4u);
    assert(CountEvents(EVENT_RESET_CHANNEL) == 1u);
    assert(CountEvents(EVENT_HOME) == 2u);
    assert(CountEvents(EVENT_RETRY) == 2u);
    assert(CountEvents(EVENT_CALIBRATE_FB) == 0u);
}

static void TestNormalFinalization(void)
{
    ResetState();
    retry_count = 1u;
    retry_results[0] = 1u;
    finalize_result = 1u;

    assert(EstablishControllerDriveState6OperatingPath() == 1u);
    assert(memory[0x001300u] == 6u);
    assert(CountEvents(EVENT_FINALIZE) == 1u);

    ResetState();
    retry_count = 1u;
    retry_results[0] = 1u;
    finalize_result = 2u;
    assert(EstablishControllerDriveState6OperatingPath() == 0u);
}

static void TestMediaState2ReadyAndExpiry(void)
{
    ResetState();
    memory[0x001008u] = 2u;
    retry_count = 1u;
    retry_results[0] = 1u;
    p0_count = 1u;
    p0_results[0] = 1u;

    assert(EstablishControllerDriveState6OperatingPath() == 1u);
    assert(memory[0x001006u] == 1u);
    assert(memory[0x001209u] == 5u);
    assert(CountEvents(EVENT_LOAD_PROFILE) == 1u);
    assert(CountEvents(EVENT_CONFIGURE_8006) == 1u);
    assert(CountEvents(EVENT_APPLY_CHANNELS) == 1u);
    assert(CountEvents(EVENT_CONFIGURE_RANGE) == 1u);
    assert(CountEvents(EVENT_READ_P0) == 1u);

    ResetState();
    memory[0x001008u] = 2u;
    memory[0x001304u] = 0x20u;
    retry_count = 1u;
    retry_results[0] = 1u;
    p0_count = 1u;
    expire_countdown_after_p0 = 1u;

    assert(EstablishControllerDriveState6OperatingPath() == 0u);
    assert(memory[0x001209u] == 0u);
    assert(memory[0x001304u] == 0x24u);
    assert(memory[0x00fb98u] == 1u);
    assert(CountEvents(EVENT_READ_P0) == 1u);
}

int main(void)
{
    TestValidationAndState5Failures();
    TestInitialRetryFailureTogglesAtipState();
    TestSecondRetryRecoveryAndParameterFailure();
    TestMandatoryHomingRetryFailure();
    TestNormalFinalization();
    TestMediaState2ReadyAndExpiry();
    return 0;
}
