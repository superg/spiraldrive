#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_calibration_path_validation.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_E8E9,
    EVENT_CALIBRATE,
    EVENT_CAPTURE,
    EVENT_STROBE,
    EVENT_CHANNEL,
    EVENT_WRITE_PORT,
    EVENT_F6,
    EVENT_WAIT_F8,
    EVENT_TIMER,
    EVENT_FLAG,
    EVENT_RESET_CHANNEL,
    EVENT_READ_PORT,
    EVENT_STATE2
};

struct event_record
{
    enum event_kind kind;
    u32 first;
    u16 second;
};

static u8 memory[1u << 24];
static struct event_record events[48];
static size_t event_count;
static u8 calibration_results[3];
static size_t calibration_count;
static size_t calibration_index;
static u8 port_sample;

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

void ObserveCalibrationPathWriteE8E9(u8 value)
{
    RecordEvent(EVENT_E8E9, value, 0u);
}

u8 ObserveCalibrationPathRunCalibration(void)
{
    RecordEvent(EVENT_CALIBRATE, 0u, 0u);
    assert(calibration_index < calibration_count);
    return calibration_results[calibration_index++];
}

void ObserveCalibrationPathCaptureSamples(void)
{
    RecordEvent(EVENT_CAPTURE, 0u, 0u);
}

void ObserveCalibrationPathStrobe(fw_addr_t address, u8 value)
{
    RecordEvent(EVENT_STROBE, address, value);
}

void ObserveCalibrationPathSetChannel(u8 channel, u8 value)
{
    RecordEvent(EVENT_CHANNEL, channel, value);
}

void ObserveCalibrationPathWritePort(fw_addr_t address, u8 value)
{
    RecordEvent(EVENT_WRITE_PORT, address, value);
}

void ObserveCalibrationPathWriteF6(u8 value)
{
    RecordEvent(EVENT_F6, value, 0u);
}

void ObserveCalibrationPathWaitF8(void)
{
    RecordEvent(EVENT_WAIT_F8, 0u, 0u);
}

void ObserveCalibrationPathWaitTimer(u16 duration, u8 yield_mode)
{
    RecordEvent(EVENT_TIMER, duration, yield_mode);
}

void ObserveCalibrationPathSetControllerFlag(u8 enabled)
{
    RecordEvent(EVENT_FLAG, enabled, 0u);
}

void ObserveCalibrationPathResetChannel(u8 channel)
{
    RecordEvent(EVENT_RESET_CHANNEL, channel, 0u);
}

u8 ObserveCalibrationPathReadPort(fw_addr_t address)
{
    RecordEvent(EVENT_READ_PORT, address, 0u);
    return port_sample;
}

void ObserveCalibrationPathConfigureState2(void)
{
    RecordEvent(EVENT_STATE2, 0u, 0u);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(calibration_results, 0, sizeof(calibration_results));
    event_count = 0u;
    calibration_count = 1u;
    calibration_index = 0u;
    calibration_results[0] = 1u;
    port_sample = 0x80u;
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

static void TestInitialFailure(void)
{
    ResetState();
    calibration_results[0] = 0u;
    memory[0x001304u] = 0x80u;
    assert(ConfigureControllerCalibrationPathAndValidatePort80C7() == 0u);
    assert(memory[0x001304u] == 0x84u);
    assert(memory[0x000e41u] == 0x16u);
    assert(event_count == 2u);
    assert(events[0].kind == EVENT_E8E9 && events[0].first == 0u);
    assert(events[1].kind == EVENT_CALIBRATE);
}

static void TestConfigurationGate(void)
{
    ResetState();
    memory[0x00faacu] = 0u;
    memory[0x00faadu] = 8u;
    assert(ConfigureControllerCalibrationPathAndValidatePort80C7() == 0u);
    assert(event_count == 4u);
    assert(events[2].kind == EVENT_CAPTURE);
    assert(events[3].kind == EVENT_STROBE);
}

static void TestFullSequenceAcceptsHighSample(void)
{
    ResetState();
    memory[0x00fab4u] = 0u;
    memory[0x00fab5u] = 0x5au;
    port_sample = 0x80u;
    assert(ConfigureControllerCalibrationPathAndValidatePort80C7() == 1u);
    assert(memory[0x00f9a6u] == 0x80u);
    assert(CountEvents(EVENT_WRITE_PORT) == 10u);
    assert(CountEvents(EVENT_WAIT_F8) == 3u);
    assert(CountEvents(EVENT_TIMER) == 2u);
    assert(CountEvents(EVENT_FLAG) == 1u);
    assert(CountEvents(EVENT_RESET_CHANNEL) == 1u);
    assert(CountEvents(EVENT_READ_PORT) == 1u);
    assert(CountEvents(EVENT_STATE2) == 0u);
    assert(CountEvents(EVENT_E8E9) == 3u);
    assert(events[0].kind == EVENT_E8E9 && events[0].first == 0u);
}

static void TestSignedFlagSelectsZeroE8E9(void)
{
    size_t index;
    u8 found_nonzero = 0u;

    ResetState();
    memory[0x00fab4u] = 0x80u;
    memory[0x00fab5u] = 0x5au;
    assert(ConfigureControllerCalibrationPathAndValidatePort80C7() == 1u);
    for(index = 0u; index < event_count; ++index)
        if(events[index].kind == EVENT_E8E9 && events[index].first != 0u)
            found_nonzero = 1u;
    assert(found_nonzero == 0u);
}

static void TestLowAndRangeSamples(void)
{
    ResetState();
    port_sample = 13u;
    assert(ConfigureControllerCalibrationPathAndValidatePort80C7() == 0u);
    assert(memory[0x001304u] == 0u);
    assert(CountEvents(EVENT_STATE2) == 0u);

    ResetState();
    port_sample = 14u;
    calibration_count = 2u;
    calibration_results[1] = 1u;
    assert(ConfigureControllerCalibrationPathAndValidatePort80C7() == 1u);
    assert(CountEvents(EVENT_STATE2) == 1u);
    assert(CountEvents(EVENT_CALIBRATE) == 2u);

    ResetState();
    port_sample = 79u;
    calibration_count = 2u;
    calibration_results[1] = 0u;
    memory[0x001304u] = 0x20u;
    assert(ConfigureControllerCalibrationPathAndValidatePort80C7() == 0u);
    assert(memory[0x001304u] == 0x24u);
    assert(memory[0x000e41u] == 0x16u);
}

static void TestAtipStateBypassesRange(void)
{
    ResetState();
    memory[0x00fb98u] = 1u;
    port_sample = 2u;
    assert(ConfigureControllerCalibrationPathAndValidatePort80C7() == 1u);
    assert(CountEvents(EVENT_STATE2) == 0u);
    assert(CountEvents(EVENT_CALIBRATE) == 1u);
}

int main(void)
{
    TestInitialFailure();
    TestConfigurationGate();
    TestFullSequenceAcceptsHighSample();
    TestSignedFlagSelectsZeroE8E9();
    TestLowAndRangeSamples();
    TestAtipStateBypassesRange();
    return 0;
}
