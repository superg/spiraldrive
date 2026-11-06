#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/read_acquisition_calibration_sequence.h"

typedef struct CalibrationEventRecord
{
    u8 event;
    u32 argument0;
    u32 argument1;
    u8 active_flag;
} CalibrationEventRecord;

static u8 firmware_memory[0x1000000ul];
static u8 sfr_memory[0x100u];
static CalibrationEventRecord events[128];
static u32 scripted_results[READ_CALIBRATION_RESTORE_CONFIGURATION + 1u][8];
static u8 scripted_result_count[READ_CALIBRATION_RESTORE_CONFIGURATION + 1u];
static u8 scripted_result_index[READ_CALIBRATION_RESTORE_CONFIGURATION + 1u];
static unsigned event_count;
static u8 set_completion_at_range_start;
static u8 auto_expire_countdown;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(firmware_memory));
    if(address == 0x001217ul && auto_expire_countdown != 0u)
    {
        firmware_memory[address] = 0u;
        auto_expire_countdown = 0u;
    }
    return firmware_memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware_memory));
    firmware_memory[address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    return sfr_memory[address];
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    sfr_memory[address] = value;
}

u32 ObserveReadAcquisitionCalibration(u8 event, u32 argument0, u32 argument1)
{
    u8 result_index;

    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].event = event;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    events[event_count].active_flag = firmware_memory[0x1301u] & 1u;
    ++event_count;

    if(event == READ_CALIBRATION_WAIT_RANGE_START && set_completion_at_range_start != 0u)
        firmware_memory[0x0e3fu] = 1u;

    result_index = scripted_result_index[event]++;
    if(result_index < scripted_result_count[event])
        return scripted_results[event][result_index];
    return 0ul;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    firmware_memory[address] = (u8)(value >> 24);
    firmware_memory[address + 1u] = (u8)(value >> 16);
    firmware_memory[address + 2u] = (u8)(value >> 8);
    firmware_memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)firmware_memory[address] << 24) | ((u32)firmware_memory[address + 1u] << 16) | ((u32)firmware_memory[address + 2u] << 8) | firmware_memory[address + 3u];
}

static void ScriptResult(u8 event, u32 value)
{
    u8 count = scripted_result_count[event];

    assert(count < 8u);
    scripted_results[event][count] = value;
    scripted_result_count[event] = (u8)(count + 1u);
}

static unsigned CountEvent(u8 event)
{
    unsigned index;
    unsigned count = 0u;

    for(index = 0u; index < event_count; ++index)
        if(events[index].event == event)
            ++count;
    return count;
}

static const CalibrationEventRecord *FindEvent(u8 event, unsigned occurrence)
{
    unsigned index;

    for(index = 0u; index < event_count; ++index)
    {
        if(events[index].event == event)
        {
            if(occurrence == 0u)
                return &events[index];
            --occurrence;
        }
    }
    return 0;
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0, sizeof(firmware_memory));
    memset(sfr_memory, 0, sizeof(sfr_memory));
    memset(events, 0, sizeof(events));
    memset(scripted_results, 0, sizeof(scripted_results));
    memset(scripted_result_count, 0, sizeof(scripted_result_count));
    memset(scripted_result_index, 0, sizeof(scripted_result_index));
    event_count = 0u;
    set_completion_at_range_start = 0u;
    auto_expire_countdown = 0u;
    firmware_memory[0x103cu] = 0xaau;
    firmware_memory[0x103au] = 0x80u;
    sfr_memory[0xb0u] = 0xfcu;
    WriteU32(0x114fu, 10000ul);
    WriteU32(0x1153u, 12000ul);
}

static void TestModeOnePositionSentinelReturnsImmediately(void)
{
    ResetFixture();
    firmware_memory[0xf08cu] = 1u;
    firmware_memory[0x0e47u] = 2u;
    WriteU32(0xf091u, 9000ul);
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 0xffu);

    assert(ExecuteReadAcquisitionCalibrationSequence() == 1u);
    assert(firmware_memory[0x0e44u] == 2u);
    assert(ReadU32(0x0e3bu) == 8850ul);
    assert(firmware_memory[0x103cu] == 0xa9u);
    assert(firmware_memory[0x103au] == 0x81u);
    assert(CountEvent(READ_CALIBRATION_APPLY_OPERATING_CODE) == 0u);
}

static void TestControllerRecoveryFailurePublishesSeven(void)
{
    ResetFixture();
    firmware_memory[0xf08cu] = 2u;
    firmware_memory[0xfb88u] = 2u;
    firmware_memory[0x0e41u] = 0x34u;
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 0u);
    ScriptResult(READ_CALIBRATION_INITIALIZE_CONTROLLER, 0u);

    assert(ExecuteReadAcquisitionCalibrationSequence() == 0u);
    assert(CountEvent(READ_CALIBRATION_RESET_DRIVE_CONTROL) == 1u);
    assert(firmware_memory[0x0e40u] == 0u);
    assert(firmware_memory[0x0e41u] == 7u);
    assert(firmware_memory[0xf08du] == 7u);
}

static void TestModeTwoCompleteMeasurementReturnsCaptureResult(void)
{
    const CalibrationEventRecord *wait_event;
    const CalibrationEventRecord *capture_event;
    static const fw_addr_t expected_strobe_addresses[5] = { 0x018080ul, 0x0180b9ul, 0x0180baul, 0x0180bdul, 0x0180bdul };
    static const u8 expected_strobe_values[5] = { 0x80u, 0x40u, 0x50u, 0x81u, 0x85u };
    unsigned index;

    ResetFixture();
    firmware_memory[0xf08cu] = 2u;
    firmware_memory[0xfb88u] = 2u;
    firmware_memory[0x0c8au] = 8u;
    firmware_memory[0x1302u] = 0x5au;
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 0xffu);
    ScriptResult(READ_CALIBRATION_WAIT_FOR_TARGET, 1u);
    ScriptResult(READ_CALIBRATION_PROBE_READINESS, 1u);
    ScriptResult(READ_CALIBRATION_WAIT_RANGE_START, 1u);
    ScriptResult(READ_CALIBRATION_ACQUIRE_RANGE, 0x5au);

    assert(ExecuteReadAcquisitionCalibrationSequence() == 0x5au);
    assert(firmware_memory[0x0e44u] == 8u);
    assert(ReadU32(0xf091u) == 7339ul);
    assert(ReadU32(0x0e3bu) == 7140ul);
    wait_event = FindEvent(READ_CALIBRATION_WAIT_FOR_TARGET, 0u);
    assert(wait_event != 0 && (s32)wait_event->argument0 == 7286L);
    assert(CountEvent(READ_CALIBRATION_STROBE_PORT) == 5u);
    for(index = 0u; index < 5u; ++index)
    {
        const CalibrationEventRecord *strobe = FindEvent(READ_CALIBRATION_STROBE_PORT, index);

        assert(strobe != 0);
        assert(strobe->argument0 == expected_strobe_addresses[index]);
        assert(strobe->argument1 == expected_strobe_values[index]);
    }
    assert(FindEvent(READ_CALIBRATION_WAIT_RANGE_START, 0u)->active_flag == 1u);
    capture_event = FindEvent(READ_CALIBRATION_ACQUIRE_RANGE, 0u);
    assert(capture_event != 0 && capture_event->active_flag == 1u);
    assert((firmware_memory[0x1301u] & 1u) == 0u);
    assert(firmware_memory[0xefcu] == 0u);
    assert(firmware_memory[0x18005u] == 0u);
    assert((firmware_memory[0x1302u] & 0x10u) == 0u);
    assert((sfr_memory[0xb0u] & 3u) == 1u);
    assert(CountEvent(READ_CALIBRATION_RESTORE_CONFIGURATION) == 1u);
}

static void TestCompletionShortcutRestoresWithoutRangeCapture(void)
{
    ResetFixture();
    firmware_memory[0xf08cu] = 3u;
    firmware_memory[0xfb88u] = 2u;
    set_completion_at_range_start = 1u;
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 2u);
    ScriptResult(READ_CALIBRATION_WAIT_FOR_TARGET, 1u);
    ScriptResult(READ_CALIBRATION_PROBE_READINESS, 1u);
    ScriptResult(READ_CALIBRATION_WAIT_RANGE_START, 1u);

    assert(ExecuteReadAcquisitionCalibrationSequence() == 1u);
    assert(CountEvent(READ_CALIBRATION_ACQUIRE_RANGE) == 0u);
    assert(CountEvent(READ_CALIBRATION_RESTORE_CONFIGURATION) == 1u);
    assert((firmware_memory[0x1301u] & 1u) == 0u);
}

static void TestPositionWaitFailureExhaustsThreeAttempts(void)
{
    ResetFixture();
    firmware_memory[0xf08cu] = 2u;
    firmware_memory[0xfb88u] = 2u;
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 0xffu);
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 0xffu);
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 0xffu);
    ScriptResult(READ_CALIBRATION_WAIT_FOR_TARGET, 0u);
    ScriptResult(READ_CALIBRATION_WAIT_FOR_TARGET, 0u);
    ScriptResult(READ_CALIBRATION_WAIT_FOR_TARGET, 0u);

    assert(ExecuteReadAcquisitionCalibrationSequence() == 0u);
    assert(CountEvent(READ_CALIBRATION_WAIT_FOR_TARGET) == 3u);
    assert(firmware_memory[0x0e40u] == 0u);
}

static void TestToleranceRetryClearsRestartAndWaitsForCountdown(void)
{
    const CalibrationEventRecord *first_tolerance;
    const CalibrationEventRecord *second_tolerance;

    ResetFixture();
    firmware_memory[0xf08cu] = 2u;
    firmware_memory[0xfb88u] = 2u;
    auto_expire_countdown = 1u;
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 1u);
    ScriptResult(READ_CALIBRATION_ACQUIRE_POSITION, 1u);
    ScriptResult(READ_CALIBRATION_CHECK_TOLERANCE, 0u);
    ScriptResult(READ_CALIBRATION_CHECK_TOLERANCE, 1u);
    ScriptResult(READ_CALIBRATION_WAIT_FOR_TARGET, 1u);
    ScriptResult(READ_CALIBRATION_PROBE_READINESS, 1u);
    ScriptResult(READ_CALIBRATION_WAIT_RANGE_START, 1u);
    ScriptResult(READ_CALIBRATION_ACQUIRE_RANGE, 1u);

    assert(ExecuteReadAcquisitionCalibrationSequence() == 1u);
    first_tolerance = FindEvent(READ_CALIBRATION_CHECK_TOLERANCE, 0u);
    second_tolerance = FindEvent(READ_CALIBRATION_CHECK_TOLERANCE, 1u);
    assert(first_tolerance != 0 && first_tolerance->argument0 == 1u);
    assert(second_tolerance != 0 && second_tolerance->argument0 == 0u);
    assert(firmware_memory[0x1217u] == 0u);
    assert(firmware_memory[0x0e40u] == 0u);
}

int main(void)
{
    TestModeOnePositionSentinelReturnsImmediately();
    TestControllerRecoveryFailurePublishesSeven();
    TestModeTwoCompleteMeasurementReturnsCaptureResult();
    TestCompletionShortcutRestoresWithoutRangeCapture();
    TestPositionWaitFailureExhaustsThreeAttempts();
    TestToleranceRetryClearsRestartAndWaitsForCountdown();
    puts("read-acquisition calibration sequence tests passed");
    return 0;
}
