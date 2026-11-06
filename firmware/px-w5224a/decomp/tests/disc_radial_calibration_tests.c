#include <assert.h>
#include <string.h>
#include "platform/disc_radial_calibration.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static u8 target_results[4], acquire_results[8], within_results[8];
static u8 q_results[8];
static unsigned target_count, acquire_count, within_count, q_count;
static unsigned reset_calibration_count, reset_state_count, measure_count;
static unsigned set_scale_count, pulse_count, rearm_count, yield_count;
static unsigned measurement8_count, span_count, rebuild_count, refine_count;
static u8 reset_calibration_result, span_result;
static u16 measured_value, set_scale_value;
static s32 target_arguments[4], within_arguments[8];
static u8 measurement_saw_variation_flag;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 AcquireDiscPositionAtTargetWithRetryBudget(s32 target)
{
    target_arguments[target_count] = target;
    return target_results[target_count++];
}
s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t address)
{
    assert(address == 0xf2bfu);
    return 1234;
}
u8 AcquireDiscReadPosition(void)
{
    u8 result = acquire_results[acquire_count++];
    if(result != 1u && acquire_count >= 2u)
        memory[0x001203ul] = 0u;
    return result;
}
u8 IsDecodedFramePositionWithinTwoFramesOfTarget(s32 target)
{
    within_arguments[within_count] = target;
    return within_results[within_count++];
}
u8 ResetDriveControlAndInitializeControllerCalibration(void)
{
    ++reset_calibration_count;
    return reset_calibration_result;
}
u16 MeasureAverageControllerPositionWord(void)
{
    ++measure_count;
    measurement_saw_variation_flag = (u8)(memory[0x00fc32ul] & 1u);
    return measured_value;
}
void SetRadialModelMeasurementAndScale(u16 value)
{
    ++set_scale_count;
    set_scale_value = value;
}
void UpdateP1Cex0PulseCalibrationStatus(void)
{
    ++pulse_count;
}
void RearmControllerQPacketCaptureWithTimeout(void)
{
    ++rearm_count;
}
u8 ProcessCapturedControllerQPacket(void)
{
    return q_results[q_count++];
}
void YieldCoroutine1(void)
{
    ++yield_count;
}
void UpdateReadAcquisitionMeasurementFrom8Samples(void)
{
    ++measurement8_count;
}
u8 CalibrateRadialModelSpanAtFrame90000(void)
{
    ++span_count;
    return span_result;
}
void RebuildRadialPositionTableAndThresholds(void)
{
    ++rebuild_count;
}
void RefineRadialCalibrationAtFrame150(void)
{
    ++refine_count;
}
void ResetDriveControlStateToEight(void)
{
    ++reset_state_count;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}
static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
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

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(target_results, 0, sizeof(target_results));
    memset(acquire_results, 0, sizeof(acquire_results));
    memset(within_results, 0, sizeof(within_results));
    memset(q_results, 0, sizeof(q_results));
    target_count = acquire_count = within_count = q_count = 0u;
    reset_calibration_count = reset_state_count = measure_count = 0u;
    set_scale_count = pulse_count = rearm_count = yield_count = 0u;
    measurement8_count = span_count = rebuild_count = refine_count = 0u;
    reset_calibration_result = 1u;
    span_result = 0u;
    measured_value = 700u;
    set_scale_value = 0u;
    measurement_saw_variation_flag = 0u;
    memory[0x0012bful] = 1u;
    WriteU16(0x00f848feul, 0x22f4u);
    WriteU16(0x000cf9ul, 1600u);
    WriteU32(0x000cf5ul, 0x12345678ul);
}

static void TestInitialTargetFailure(void)
{
    ResetState();
    memory[0x001008ul] = 1u;
    assert(CalibrateDiscRadialPositionModel() == 0x11u);
    assert(target_count == 1u && target_arguments[0] == 150);
    assert(memory[0x001203ul] == 2u);
    assert(acquire_count == 0u);
}

static void TestOrdinarySuccess(void)
{
    ResetState();
    memory[0x00fbecul] = 1u;
    acquire_results[0] = 1u;
    within_results[0] = 1u;
    assert(CalibrateDiscRadialPositionModel() == 0u);
    assert(acquire_count == 1u && within_count == 1u);
    assert(within_arguments[0] == 147);
    assert(measure_count == 1u && measurement_saw_variation_flag == 1u);
    assert(set_scale_count == 1u && set_scale_value == 700u);
    assert((memory[0x00fc32ul] & 1u) == 0u);
    assert(pulse_count == 1u && measurement8_count == 1u);
    assert(span_count == 1u && rebuild_count == 1u);
    assert(ReadU16(0x001000ul) == 0x22f4u);
    assert(ReadU32(0x00fbe6ul) == 0x12345678ul);
    assert(ReadU16(0x00fbeaul) == 1600u);
}

static void TestAcquisitionResetFailure(void)
{
    ResetState();
    reset_calibration_result = 0u;
    assert(CalibrateDiscRadialPositionModel() == 0x11u);
    assert(acquire_count == 1u && reset_calibration_count == 1u);
}

static void TestMode2ControllerQAndSpanFailure(void)
{
    ResetState();
    memory[0x001008ul] = 2u;
    memory[0x001132ul] = 0x20u;
    memory[0x001305ul] = 0x80u;
    target_results[0] = 1u;
    target_results[1] = 1u;
    acquire_results[0] = 1u;
    within_results[0] = 1u;
    q_results[0] = 0xfeu;
    q_results[1] = 0xfeu;
    q_results[2] = 1u;
    span_result = 7u;
    assert(CalibrateDiscRadialPositionModel() == 7u);
    assert(target_count == 2u);
    assert(target_arguments[0] == 150 && target_arguments[1] == 132);
    assert(rearm_count == 1u && q_count == 3u && yield_count == 2u);
    assert((memory[0x001132ul] & 0x20u) == 0u);
    assert(rebuild_count == 0u);
}

static void TestNoMeasurementTerminalPaths(void)
{
    ResetState();
    memory[0x001308ul] = 1u;
    assert(CalibrateDiscRadialPositionModel() == 0u);
    assert(acquire_count == 2u && reset_calibration_count == 2u);
    assert(reset_state_count == 0u);

    ResetState();
    assert(CalibrateDiscRadialPositionModel() == 0x11u);
    assert(reset_state_count == 1u);
}

static void TestRefinementGate(void)
{
    ResetState();
    memory[0x001008ul] = 1u;
    target_results[0] = 1u;
    acquire_results[0] = 1u;
    within_results[0] = 1u;
    memory[0x001132ul] = 0u;
    memory[0x00fc18ul] = 0x08u;
    memory[0x00f553ul] = 0xffu;
    WriteU32(0x0012c1ul, 99000ul);
    assert(CalibrateDiscRadialPositionModel() == 0u);
    assert(refine_count == 1u);
}

int main(void)
{
    TestInitialTargetFailure();
    TestOrdinarySuccess();
    TestAcquisitionResetFailure();
    TestMode2ControllerQAndSpanFailure();
    TestNoMeasurementTerminalPaths();
    TestRefinementGate();
    return 0;
}
