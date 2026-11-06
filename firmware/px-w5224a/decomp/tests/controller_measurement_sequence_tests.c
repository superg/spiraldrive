#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_sequence.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 acquire_results[8];
static u8 acquire_count;
static u8 calibration_results[4];
static u8 calibration_count;
static u8 tolerance_results[4];
static u8 tolerance_count;
static u8 tolerance_arguments[4];
static u8 wait_results[16];
static u8 wait_count;
static s32 wait_targets[16];
static u8 readiness_results[8];
static u8 readiness_count;
static u8 window_results[8];
static u8 window_count;
static u8 restore_arguments[8];
static u8 restore_count;
static u8 reset_count;
static u8 enable_count;
static u8 apply_count;
static u8 configure_count;
static u8 data_reset_count;
static u8 profile_count;
static u8 capture_window_count;
static u8 capture_target_count;
static u8 parameter_code_count;
static u8 parameter_restore_count;
static u8 capture_status;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

void ComputeMediaState6MeasurementTargetFromLeadInOffset(void) {}
u8 AcquireDiscReadPosition(void)
{
    return acquire_results[acquire_count++];
}
void ResetDriveControlStateToEight(void)
{
    ++reset_count;
}
u8 InitializeControllerOperatingStateAndRunCalibration(void)
{
    return calibration_results[calibration_count++];
}
void SetControllerOperatingEnableState(u8 enabled)
{
    assert(enabled == 1u);
    ++enable_count;
}
u8 CheckControllerMeasurementToleranceOrTimeout(u8 restart_timeout)
{
    tolerance_arguments[tolerance_count] = restart_timeout;
    return tolerance_results[tolerance_count++];
}
void ApplyOperatingCodeAndFixedControllerChannels0Through2(void)
{
    ++apply_count;
}
u8 WaitForDecodedFramePositionPastTarget(s32 target)
{
    wait_targets[wait_count] = target;
    return wait_results[wait_count++];
}
void ConfigureControllerPortsAndParametersForCurrentOperatingCode(u8 enabled)
{
    assert(enabled == 1u);
    ++configure_count;
}
void ResetControllerDataPathRegisters(void)
{
    ++data_reset_count;
}
void ConfigureControllerRegisterProfileForCurrentMediaState(void)
{
    ++profile_count;
}
u8 RunControllerReadPathReadinessProbe(void)
{
    return readiness_results[readiness_count++];
}
u8 IsControllerReadPathWithinOperatingWindow(u8 wide)
{
    assert(wide == 1u);
    return window_results[window_count++];
}
void RestoreControllerReadPathConfiguration(u8 refresh)
{
    restore_arguments[restore_count++] = refresh;
}
void WriteControllerParametersD4AndE2ForCode(u8 code)
{
    assert(code == 0xffu);
    ++parameter_code_count;
}
void WriteControllerParametersD412AndE2Value58(void)
{
    ++parameter_restore_count;
}
void CaptureControllerMeasurementPacketInTargetWindow(void)
{
    ++capture_window_count;
    memory[0x00ef7ful] = capture_status;
}
u8 CaptureControllerMeasurementPacketAtComputedTarget(void)
{
    ++capture_target_count;
    memory[0x00ef7ful] = capture_status;
    return 0u;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(acquire_results, 2, sizeof(acquire_results));
    memset(calibration_results, 1, sizeof(calibration_results));
    memset(tolerance_results, 1, sizeof(tolerance_results));
    memset(wait_results, 1, sizeof(wait_results));
    memset(readiness_results, 1, sizeof(readiness_results));
    memset(window_results, 1, sizeof(window_results));
    acquire_count = calibration_count = tolerance_count = wait_count = 0u;
    readiness_count = window_count = restore_count = 0u;
    reset_count = enable_count = apply_count = configure_count = 0u;
    data_reset_count = profile_count = capture_window_count = 0u;
    capture_target_count = parameter_code_count = parameter_restore_count = 0u;
    capture_status = 1u;
    memory[0x000e47ul] = 4u;
    memory[0x00ef7ful] = 1u;
    WriteDword(0x00103dul, 10000ul);
    WriteDword(0x00114ful, 10000ul);
}

static void CheckSuccessfulModeTwoCapture(void)
{
    Reset();
    memory[0x00ef72ul] = 2u;
    ExecuteControllerMeasurementSequenceAtComputedTarget();
    assert(acquire_count == 1u && wait_count == 3u);
    assert(wait_targets[0] == 9740 && wait_targets[1] == 9820 && wait_targets[2] == 9920);
    assert(apply_count == 2u && configure_count == 1u && data_reset_count == 1u && profile_count == 1u);
    assert(capture_window_count == 1u && capture_target_count == 0u);
    assert(parameter_code_count == 1u && parameter_restore_count == 1u);
    assert(restore_count == 1u && restore_arguments[0] == 1u);
    assert((memory[0x001301ul] & 1u) == 0u && memory[0x000efcul] == 0u && memory[0x018005ul] == 0u);
}

static void CheckOrdinaryRecoveryAndStatusThree(void)
{
    Reset();
    capture_status = 3u;
    window_results[0] = 0u;
    window_results[1] = 1u;
    ExecuteControllerMeasurementSequenceAtComputedTarget();
    assert(wait_count == 4u);
    assert(wait_targets[2] == 9970 && wait_targets[3] == 9985);
    assert(restore_count == 2u && restore_arguments[0] == 0u && restore_arguments[1] == 1u);
    assert(readiness_count == 2u);
    assert(capture_target_count == 1u && capture_window_count == 0u);
    assert(memory[0x00ef7eul] == 0x1eu && memory[0x000faful] == 0u);
}

static void CheckToleranceRepollAndClampScale(void)
{
    Reset();
    acquire_results[0] = 1u;
    acquire_results[1] = 2u;
    tolerance_results[0] = 0u;
    ExecuteControllerMeasurementSequenceAtComputedTarget();
    assert(acquire_count == 2u && tolerance_count == 1u && tolerance_arguments[0] == 1u);
    assert(ReadDword(0x000e3bul) == 9700ul);
    assert(wait_targets[0] == 9740 && wait_targets[1] == 9820);
}

static void CheckSpecialSpeedOverride(void)
{
    Reset();
    memory[0x00fb87ul] = 0x40u;
    memory[0x000e47ul] = 7u;
    ExecuteControllerMeasurementSequenceAtComputedTarget();
    assert(memory[0x000e4bul] == 4u && memory[0x000e44ul] == 4u);
    assert(wait_targets[0] == 9740 && wait_targets[1] == 9820);
}

static void CheckCalibrationAndAttemptFailures(void)
{
    Reset();
    acquire_results[0] = 0u;
    calibration_results[0] = 0u;
    ExecuteControllerMeasurementSequenceAtComputedTarget();
    assert(reset_count == 1u && calibration_count == 1u);
    assert(memory[0x000e40ul] == 0u && memory[0x000e41ul] == 7u && memory[0x00ef7eul] == 7u && memory[0x00ef7ful] == 4u);

    Reset();
    wait_results[0] = 0u;
    wait_results[1] = 0u;
    wait_results[2] = 0u;
    ExecuteControllerMeasurementSequenceAtComputedTarget();
    assert(acquire_count == 3u && wait_count == 3u);
    assert(memory[0x000e41ul] == 5u && memory[0x00ef7eul] == 5u && memory[0x00ef7ful] == 4u && memory[0x000e40ul] == 0u);
}

int main(void)
{
    CheckSuccessfulModeTwoCapture();
    CheckOrdinaryRecoveryAndStatusThree();
    CheckToleranceRepollAndClampScale();
    CheckSpecialSpeedOverride();
    CheckCalibrationAndAttemptFailures();
    return 0;
}
