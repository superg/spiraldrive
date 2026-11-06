#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/read_acquisition_state.h"
#include "platform/firmware_memory.h"

enum
{
    STEP_UPDATE_MEASUREMENT = 1,
    STEP_RESET_EIGHT_CHANNEL = 2,
    STEP_ADJUST_TARGET = 3,
    STEP_SELECT_OPERATING_CODE = 4,
    STEP_YIELD = 5,
    STEP_CONFIGURE_ALTERNATE = 6,
    STEP_CONFIGURE_NORMAL = 7,
    STEP_APPLY_OPERATING_CODE = 8,
    STEP_RESET_PORTS = 9,
    STEP_APPLY_CONDITIONAL = 10,
    STEP_PREPARE_COMMAND0E = 11,
    STEP_CLEAR_RAW_PORTS = 12,
    STEP_REARM_Q = 13,
    STEP_ARM_THREE_BYTE = 14,
    STEP_COUNT_MATCH = 15,
    STEP_SERVICE_EIGHT_CHANNEL = 16,
    STEP_SERVICE_COMMAND0E = 17,
    STEP_FINALIZE_EIGHT_CHANNEL = 18,
    STEP_SET_TARGET = 19,
    STEP_RESET_DRIVE_CONTROL = 20
};

static u8 memory_image[0x20000];
static u8 bit_values[0x100];
static u16 step_counts[21];
static s8 acquisition_results[8];
static u8 q_results[16];
static u8 three_byte_results[8];
static u8 tolerance_results[8];
static u8 calibration_results[8];
static u8 acquisition_count, acquisition_index;
static u8 q_count, q_index, three_byte_count, three_byte_index;
static u8 tolerance_count, tolerance_index;
static u8 calibration_count, calibration_index;
static u8 operating_enable_count, operating_enable_values[8];
static u8 speed_reduction_count, speed_reduction_values[8];
static u8 yield_count, abort_on_yield;
static u8 q_failure_status, service_failure_status;
static u16 dynamic_tolerance;
static u8 pulse_result;

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(bit_values, 0, sizeof(bit_values));
    memset(step_counts, 0, sizeof(step_counts));
    memset(acquisition_results, 0, sizeof(acquisition_results));
    memset(q_results, 0, sizeof(q_results));
    memset(three_byte_results, 0, sizeof(three_byte_results));
    memset(tolerance_results, 0, sizeof(tolerance_results));
    memset(calibration_results, 0, sizeof(calibration_results));
    memset(operating_enable_values, 0, sizeof(operating_enable_values));
    memset(speed_reduction_values, 0, sizeof(speed_reduction_values));
    acquisition_count = acquisition_index = 0u;
    q_count = q_index = three_byte_count = three_byte_index = 0u;
    tolerance_count = tolerance_index = 0u;
    calibration_count = calibration_index = 0u;
    operating_enable_count = speed_reduction_count = 0u;
    yield_count = abort_on_yield = 0u;
    q_failure_status = service_failure_status = 0u;
    dynamic_tolerance = 0u;
    pulse_result = 0u;
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

void ObserveReadAcquisitionStateStep(u8 step)
{
    assert(step < sizeof(step_counts) / sizeof(step_counts[0]));
    ++step_counts[step];
    if(step == STEP_YIELD)
    {
        ++yield_count;
        if(yield_count == abort_on_yield)
            memory_image[0xe3fu] = 1u;
    }
    if(step == STEP_SERVICE_COMMAND0E && service_failure_status != 0u)
        memory_image[0xf79au] = service_failure_status;
}

u16 ObserveReadAcquisitionToleranceFrames(void)
{
    return dynamic_tolerance;
}

void ObserveReadAcquisitionOperatingEnable(u8 enabled)
{
    assert(operating_enable_count < sizeof(operating_enable_values));
    operating_enable_values[operating_enable_count++] = enabled;
    memory_image[0x1006u] = enabled;
}

s8 ObserveReadAcquisitionDiscPosition(void)
{
    assert(acquisition_index < acquisition_count);
    return acquisition_results[acquisition_index++];
}

u8 ObserveReadAcquisitionTolerance(u8 percent, u8 lower_only)
{
    assert(percent == 3u && lower_only == 0u);
    assert(tolerance_index < tolerance_count);
    return tolerance_results[tolerance_index++];
}

u8 ObserveReadAcquisitionQStatus(void)
{
    assert(q_index < q_count);
    if(q_failure_status != 0u)
        memory_image[0xf79au] = q_failure_status;
    return q_results[q_index++];
}

u8 ObserveReadAcquisitionThreeByteStatus(void)
{
    assert(three_byte_index < three_byte_count);
    return three_byte_results[three_byte_index++];
}

u8 ObserveReadAcquisitionPulse(void)
{
    return pulse_result;
}

void ObserveReadAcquisitionSpeedReduction(u8 force_minimum)
{
    assert(speed_reduction_count < sizeof(speed_reduction_values));
    speed_reduction_values[speed_reduction_count++] = force_minimum;
}

u8 ObserveReadAcquisitionCalibration(void)
{
    assert(calibration_index < calibration_count);
    return calibration_results[calibration_index++];
}

u8 ObserveReadAcquisitionBit(u8 bit_address)
{
    return bit_values[bit_address];
}

static void TestAcquisitionTerminalStatuses(void)
{
    ResetHarness();
    memory_image[0x1036u] = 3u;
    acquisition_results[0] = 0;
    acquisition_count = 1u;
    ProcessReadAcquisitionState();
    assert(memory_image[0xe40u] == 0u);
    assert(memory_image[0xfafu] == 0u);
    assert(memory_image[0x1036u] == 2u);
    assert(step_counts[STEP_UPDATE_MEASUREMENT] == 1u);
    assert(step_counts[STEP_RESET_EIGHT_CHANNEL] == 1u);
    assert(step_counts[STEP_ADJUST_TARGET] == 1u);
    assert(step_counts[STEP_SELECT_OPERATING_CODE] == 1u);

    ResetHarness();
    memory_image[0xe3fu] = 1u;
    memory_image[0x1036u] = 3u;
    acquisition_results[0] = -1;
    acquisition_count = 1u;
    ProcessReadAcquisitionState();
    assert(memory_image[0xe3fu] == 0u);
    assert(memory_image[0xe40u] == 1u);
    assert(memory_image[0x1036u] == 2u);
}

static void TestDynamicToleranceAndCalibrationFailure(void)
{
    ResetHarness();
    memory_image[0xf46u] = 1u;
    acquisition_results[0] = 1;
    acquisition_results[1] = 1;
    acquisition_count = 2u;
    dynamic_tolerance = 0x1234u;
    tolerance_results[0] = 0u;
    tolerance_results[1] = 1u;
    tolerance_count = 2u;
    q_results[0] = 0u;
    q_count = 1u;
    q_failure_status = 7u;
    calibration_results[0] = 0u;
    calibration_count = 1u;

    ProcessReadAcquisitionState();

    assert(acquisition_index == 2u && tolerance_index == 2u);
    assert(memory_image[0x1034u] == 0x12u);
    assert(memory_image[0x1035u] == 0x34u);
    assert(memory_image[0x1036u] == 2u);
    assert(step_counts[STEP_CONFIGURE_ALTERNATE] == 1u);
    assert(step_counts[STEP_PREPARE_COMMAND0E] == 1u);
    assert(step_counts[STEP_FINALIZE_EIGHT_CHANNEL] == 1u);
    assert(speed_reduction_count == 1u);
    assert(speed_reduction_values[0] == 1u);
    assert(memory_image[0xe40u] == 0u);
    assert(memory_image[0xe41u] == 7u);
    assert(memory_image[0xfbcu] == 7u);
    assert(memory_image[0xfafu] == 0u);
}

static void TestThreeByteAbortDoesNotFinalize(void)
{
    ResetHarness();
    memory_image[0x1006u] = 1u;
    bit_values[0x0eu] = 0u;
    acquisition_results[0] = 1;
    acquisition_count = 1u;
    three_byte_results[0] = 0xfeu;
    three_byte_count = 1u;
    abort_on_yield = 2u;

    ProcessReadAcquisitionState();

    assert(step_counts[STEP_ARM_THREE_BYTE] == 1u);
    assert(step_counts[STEP_REARM_Q] == 0u);
    assert(step_counts[STEP_FINALIZE_EIGHT_CHANNEL] == 0u);
    assert(yield_count == 2u);
    assert(memory_image[0xe3fu] == 0u);
}

static void TestBypassGateAndQAbortFinalizes(void)
{
    ResetHarness();
    memory_image[0x1006u] = 1u;
    memory_image[0xe46u] = 1u;
    bit_values[0x0eu] = 1u;
    acquisition_results[0] = 1;
    acquisition_count = 1u;
    q_results[0] = 0xfeu;
    q_count = 1u;
    abort_on_yield = 2u;

    ProcessReadAcquisitionState();

    assert(operating_enable_count == 1u);
    assert(operating_enable_values[0] == 0u);
    assert(step_counts[STEP_REARM_Q] == 1u);
    assert(step_counts[STEP_FINALIZE_EIGHT_CHANNEL] == 1u);
    assert(memory_image[0xe3fu] == 0u);
}

static void TestPersistentCaptureFailureExhaustsRetries(void)
{
    u8 index;

    ResetHarness();
    acquisition_results[0] = 1;
    acquisition_count = 1u;
    q_count = 4u;
    q_failure_status = 7u;
    calibration_count = 3u;
    for(index = 0u; index != calibration_count; ++index)
        calibration_results[index] = 1u;

    ProcessReadAcquisitionState();

    assert(q_index == 4u);
    assert(step_counts[STEP_FINALIZE_EIGHT_CHANNEL] == 4u);
    assert(step_counts[STEP_RESET_DRIVE_CONTROL] == 3u);
    assert(calibration_index == 3u);
    assert(speed_reduction_count == 3u);
    for(index = 0u; index != speed_reduction_count; ++index)
        assert(speed_reduction_values[index] == 1u);
    assert(memory_image[0xfbcu] == 7u);
    assert(memory_image[0xfafu] == 0u);
}

int main(void)
{
    TestAcquisitionTerminalStatuses();
    TestDynamicToleranceAndCalibrationFailure();
    TestThreeByteAbortDoesNotFinalize();
    TestBypassGateAndQAbortFinalizes();
    TestPersistentCaptureFailureExhaustsRetries();
    puts("read acquisition state tests passed");
    return 0;
}
