#include "controller_measurement_sequence.h"
#include "../media/controller_measurement_dispatch.h"
#include "controller_current_profile_configuration.h"
#include "controller_operating_state_initialization.h"
#include "controller_operating_window.h"
#include "controller_parameters.h"
#include "drive_control_reset.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

extern u8 AcquireDiscReadPosition(void);

static u32 ReadMeasurementSequenceDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteMeasurementSequenceDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static s32 ScaledMeasurementPositionOffset(u8 operating_speed, u8 multiplier, float scale)
{
    s16 product = (s16)((u16)operating_speed * (u16)multiplier);
    float scaled = (float)product;

    scaled = (float)(scaled * scale);
    return (s32)scaled;
}

static void PublishMeasurementSequenceFailure(u8 status)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, status);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, status);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
}

/* RAM:f94fc3 */
void PLEXTOR_NEAR_FUNCTION ExecuteControllerMeasurementSequenceAtComputedTarget(void)
{
    u8 restart_timeout = 1u;
    float position_scale = 1.0f;
    u8 attempt = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 2u);
    ComputeMediaState6MeasurementTargetFromLeadInOffset();

    for(;;)
    {
        u8 operating_speed;
        u8 acquisition_result;
        s32 measurement_target;
        s32 lower_target_bound;

        if(attempt >= 3u)
        {
            PublishMeasurementSequenceFailure(5u);
            return;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 0x10u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            operating_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 0x10u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0x10u);
            operating_speed = 0x10u;
        }

        measurement_target = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - (u32)((u16)operating_speed * 75u));
        WriteMeasurementSequenceDword(FW_NEAR_UNKNOWN_0E3B, (u32)measurement_target);
        lower_target_bound = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_114F) - 0x0b2cul);
        if(measurement_target <= lower_target_bound)
        {
            s32 delta;
            s16 denominator;

            WriteMeasurementSequenceDword(FW_NEAR_UNKNOWN_0E3B, (u32)lower_target_bound);
            delta = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - (u32)lower_target_bound);
            denominator = (s16)((u16)operating_speed * (u16)75u);
            position_scale = (float)((float)delta / (float)denominator);
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 2u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0x40u) != 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 8u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 4u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
                operating_speed = 4u;
                measurement_target = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - 300ul);
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 8u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 8u);
                operating_speed = 8u;
                measurement_target = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - 600ul);
            }
            WriteMeasurementSequenceDword(FW_NEAR_UNKNOWN_0E3B, (u32)measurement_target);
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        acquisition_result = AcquireDiscReadPosition();
        if(acquisition_result == 0u)
        {
            ResetDriveControlStateToEight();
            if(InitializeControllerOperatingStateAndRunCalibration() == 0u)
            {
                PublishMeasurementSequenceFailure(7u);
                return;
            }
            SetControllerOperatingEnableState(1u);
            ++attempt;
            continue;
        }
        if(acquisition_result == 1u && CheckControllerMeasurementToleranceOrTimeout(restart_timeout) == 0u)
        {
            restart_timeout = 0u;
            continue;
        }
        if(acquisition_result == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);

        ApplyOperatingCodeAndFixedControllerChannels0Through2();
        measurement_target = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - (u32)ScaledMeasurementPositionOffset(operating_speed, 65u, position_scale));
        if(WaitForDecodedFramePositionPastTarget(measurement_target) == 0u)
        {
            ++attempt;
            continue;
        }

        ConfigureControllerPortsAndParametersForCurrentOperatingCode(1u);
        ApplyOperatingCodeAndFixedControllerChannels0Through2();
        ResetControllerDataPathRegisters();
        ConfigureControllerRegisterProfileForCurrentMediaState();
        measurement_target = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - (u32)ScaledMeasurementPositionOffset(operating_speed, 45u, position_scale));
        if(WaitForDecodedFramePositionPastTarget(measurement_target) == 0u || RunControllerReadPathReadinessProbe() == 0u)
        {
            ++attempt;
            continue;
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 2u) != 0u)
        {
            measurement_target = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - (u32)ScaledMeasurementPositionOffset(operating_speed, 20u, position_scale));
            if(WaitForDecodedFramePositionPastTarget(measurement_target) == 0u)
            {
                ++attempt;
                continue;
            }
            if(IsControllerReadPathWithinOperatingWindow(1u) == 0u)
            {
                RestoreControllerReadPathConfiguration(1u);
                ++attempt;
                continue;
            }
        }
        else
        {
            measurement_target = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - 30ul);
            if(WaitForDecodedFramePositionPastTarget(measurement_target) == 0u)
            {
                ++attempt;
                continue;
            }
            if(IsControllerReadPathWithinOperatingWindow(1u) == 0u)
            {
                RestoreControllerReadPathConfiguration(0u);
                if(RunControllerReadPathReadinessProbe() == 0u)
                {
                    ++attempt;
                    continue;
                }
            }
            measurement_target = (s32)(ReadMeasurementSequenceDword(FW_NEAR_UNKNOWN_103D) - 15ul);
            if(WaitForDecodedFramePositionPastTarget(measurement_target) == 0u)
            {
                ++attempt;
                continue;
            }
            if(IsControllerReadPathWithinOperatingWindow(1u) == 0u)
            {
                RestoreControllerReadPathConfiguration(1u);
                ++attempt;
                continue;
            }
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x40u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
        WriteControllerParametersD4AndE2ForCode(0xffu);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 2u) != 0u)
            CaptureControllerMeasurementPacketInTargetWindow();
        else
            (void)CaptureControllerMeasurementPacketAtComputedTarget();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
        WriteControllerParametersD412AndE2Value58();
        RestoreControllerReadPathConfiguration(1u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 1u)
            return;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 3u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, 0x1eu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            return;
        }
        ++attempt;
    }
}
