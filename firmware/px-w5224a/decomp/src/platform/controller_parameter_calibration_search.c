#include "controller_parameter_calibration_search.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_parameter_calibration_search_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCalibrationSearchU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u16 CaptureCalibrationSearchMeasurement(u16 delay)
{
    u16 measurement;

    WaitTimer1ScaledDuration(delay, 1u);
    DISABLE_CALIBRATION_INTERRUPTS();
    measurement = MeasureAverageSfrDbSamples(8u);
    RESTORE_CALIBRATION_INTERRUPTS();
    return measurement;
}

static u8 AdjustCalibrationSearchCandidate(u8 parameter_code, u8 candidate, u16 measurement, u16 baseline, u8 step)
{
    u8 reverse_direction = (u8)(parameter_code == 0x00u || parameter_code == 0x04u);

    if(measurement > baseline)
        return reverse_direction != 0u ? (u8)(candidate - step) : (u8)(candidate + step);
    return reverse_direction != 0u ? (u8)(candidate + step) : (u8)(candidate - step);
}

static u8 IsCalibrationSearchCandidateOutOfRange(u8 candidate)
{
    return (u8)((candidate >= 0x80u && candidate <= 0x82u) || (candidate >= 0x7cu && candidate <= 0x7fu));
}

static void InitializeCalibrationSearchHardware(u8 parameter_code)
{
    if(parameter_code == 0x00u)
    {
        StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018080, 0x80u);
        WriteControllerRegister01Clamped(0x0cu);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x10u);
    }

    WriteCalibratedControllerParameterForCode(parameter_code, 0u);
}

static void ConfigureCalibrationSearchParameter(u8 parameter_code)
{
    WriteControllerParameterE2ValueAndRefreshEE(parameter_code);
    switch(parameter_code)
    {
    case 0x24u:
        WriteControllerParameterWord(FW_NEAR_UNKNOWN_FB81);
        break;
    case 0x2cu:
        WriteControllerParameterF6ValueWithBits0And1Set(0xf8u);
        break;
    case 0x3cu:
        WriteControllerParameterWord(0xffe0u);
        WriteControllerParameterWord(0xe0e0u);
        break;
    default:
        break;
    }
}

static void CommitCalibrationSearchResult(u8 parameter_code, u8 candidate)
{
    switch(parameter_code)
    {
    case 0x00u:
        candidate = (u8)(candidate + 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC09, candidate);
        WriteControllerParameterWord((u16)(0xf200u | candidate));
        StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018080, FirmwareReadNear8(FW_NEAR_UNKNOWN_131C));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018001, 4u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFB));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFC));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x80u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
        break;
    case 0x04u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0A, candidate);
        break;
    case 0x1cu:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0C, candidate);
        break;
    case 0x24u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0E, candidate);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0D, candidate);
        break;
    case 0x2cu:
        SelectAndWriteControllerParameterF6ForOperatingCode(1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0B, candidate);
        break;
    case 0x3cu:
    {
        u8 value = ComputeControllerParametersFFE0Value();

        WriteControllerParameterWord((u16)(0xff00u | value));
        WriteControllerParameterWord((u16)(0xe000u | value));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC10, candidate);
        break;
    }
    default:
        break;
    }
    WriteControllerParameterE2ValueAndRefreshEE(0x58u);
}

/* RAM:fc9a5b */
u8 PLEXTOR_NEAR_FUNCTION CalibrateControllerParameterForCode(u8 parameter_code)
{
    u16 baseline;
    u16 wide_upper;
    u16 wide_lower;
    u16 narrow_upper;
    u16 narrow_lower;
    u16 measurement;
    u16 previous_measurement = 0u;
    u8 candidate = 0u;
    u8 coarse_step = 2u;
    u8 fine_step = 2u;
    u8 state = 0u;
    u8 last_candidate = 0u;
    u8 repeat_count = 0u;

    InitializeCalibrationSearchHardware(parameter_code);
    baseline = ReadCalibrationSearchU16(FW_NEAR_UNKNOWN_FC05);
    wide_upper = (u16)(baseline + 15u);
    wide_lower = (u16)(baseline - 15u);
    narrow_upper = (u16)(baseline + 3u);
    narrow_lower = (u16)(baseline - 3u);
    if(parameter_code == 0x2cu)
    {
        wide_upper = (u16)(wide_upper + 4u);
        wide_lower = (u16)(wide_lower - 4u);
    }
    else if(parameter_code == 0x3cu)
    {
        wide_upper = (u16)(wide_upper + 42u);
        wide_lower = (u16)(wide_lower - 42u);
    }
    ConfigureCalibrationSearchParameter(parameter_code);

    measurement = CaptureCalibrationSearchMeasurement(2000u);
    if(measurement >= wide_lower && measurement <= wide_upper)
    {
        state = 1u;
        if(measurement >= narrow_lower && measurement <= narrow_upper)
            state = 2u;
        else
            candidate = AdjustCalibrationSearchCandidate(parameter_code, candidate, measurement, baseline, fine_step);
    }
    else
    {
        candidate = AdjustCalibrationSearchCandidate(parameter_code, candidate, measurement, baseline, coarse_step);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 100u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) >= 0 && state != 2u)
    {
        WriteCalibratedControllerParameterForCode(parameter_code, candidate);
        if(IsCalibrationSearchCandidateOutOfRange(candidate) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 0xffu);
            break;
        }

        measurement = CaptureCalibrationSearchMeasurement(400u);
        if(state == 1u)
        {
            u8 step;

            if(measurement >= narrow_lower && measurement <= narrow_upper)
                break;
            step = (u8)(coarse_step >> 1);
            if(step <= fine_step)
                step = fine_step;
            candidate = AdjustCalibrationSearchCandidate(parameter_code, candidate, measurement, baseline, step);

            if(measurement > baseline)
            {
                if(last_candidate == candidate && repeat_count <= 2u)
                {
                    ++repeat_count;
                    previous_measurement = measurement;
                }
                last_candidate = candidate;
            }
            else if(repeat_count == 2u)
            {
                u16 previous_delta = (u16)(previous_measurement - baseline);
                u16 current_delta = (u16)(baseline - measurement);

                if(current_delta <= previous_delta)
                    candidate = last_candidate;
                WriteCalibratedControllerParameterForCode(parameter_code, candidate);
                break;
            }
        }
        else if(measurement >= wide_lower && measurement <= wide_upper)
        {
            u8 step = (u8)(coarse_step >> 1);

            if(step <= fine_step)
                step = fine_step;
            candidate = AdjustCalibrationSearchCandidate(parameter_code, candidate, measurement, baseline, step);
            state = 1u;
        }
        else
        {
            candidate = AdjustCalibrationSearchCandidate(parameter_code, candidate, measurement, baseline, coarse_step);
        }
    }

    CommitCalibrationSearchResult(parameter_code, candidate);
    return (u8)((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) >= 0);
}
