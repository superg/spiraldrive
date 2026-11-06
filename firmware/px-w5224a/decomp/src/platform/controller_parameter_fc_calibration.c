#include "controller_parameter_fc_calibration.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

static u8 MeasureControllerParameterFcTrial(void)
{
    u16 sum = 0u;
    u8 sample_index;

    for(sample_index = 0u; sample_index < 8u; ++sample_index)
    {
        sum = (u16)(sum + MeasureAveragedSfrD8MinusD9WithOffsets());
        WaitTimer1ScaledDuration(200u, 0u);
    }
    return (u8)(sum >> 3);
}

static u8 IsControllerParameterFcMeasurementAccepted(u8 measurement)
{
    return (u8)(measurement >= 77u && measurement <= 85u);
}

static u8 CommitControllerParameterFcCalibration(u8 candidate)
{
    u8 parameter_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);

    if(parameter_state == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC0, candidate);
    }
    else if(parameter_state == 1u)
    {
        if(candidate >= 0xb0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC0, (u8)(candidate + 0x40u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC1, candidate);
    }
    WriteControllerParametersD412AndE2Value58();
    return 1u;
}

/* RAM:fc9f94 */
u8 CalibrateControllerParameterFC(void)
{
    u8 candidate = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u ? 0xa0u : 0x78u;
    u8 measurement;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, candidate);
    WriteControllerParametersD4AndE2ForCode(0x34u);
    WriteControllerParameterFCValue(candidate);
    WaitTimer1ScaledDuration(7000u, 1u);
    measurement = MeasureControllerParameterFcTrial();
    if(IsControllerParameterFcMeasurementAccepted(measurement) != 0u)
        return CommitControllerParameterFcCalibration(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F));

    if(measurement > 85u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) - 4u));
    else if(measurement < 77u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) + 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120A, 10u);

    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) >= 0)
    {
        WriteControllerParameterFCValue(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) == 0xfcu || FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) == 0u)
            break;

        WaitTimer1ScaledDuration(400u, 1u);
        measurement = MeasureControllerParameterFcTrial();
        if(IsControllerParameterFcMeasurementAccepted(measurement) != 0u)
            break;
        if(measurement > 85u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) - 4u));
        else if(measurement < 77u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) + 4u));
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) < 0)
        return 0u;
    return CommitControllerParameterFcCalibration(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F));
}
