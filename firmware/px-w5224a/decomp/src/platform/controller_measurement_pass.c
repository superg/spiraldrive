#include "controller_measurement_pass.h"
#include "controller_mapping.h"
#include "controller_measurement_recovery.h"
#include "controller_measurement_sequence.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "media_state6_lead_in_reference.h"
#include "status_mapping.h"

/* RAM:f94b91 */
u8 ExecuteControllerMeasurementPassAndClassifyResult(void)
{
    u8 combined_status;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0xf1u) | 0x0cu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) | 1u));
    SetControllerOperatingEnableState(1u);
    AcquireMediaState6LeadInMeasurementReference();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 4u)
        return 2u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF74) >= 0x28u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) == 0xb9u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, 0x60u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF74) >= 0x60u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 8u) != 0u)
    {
        if(ExecuteReadAcquisitionCalibrationForMode2() == 0u)
            return 2u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xf7u));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) | 2u));
    SelectControllerMeasurementPassByEf72Bit5();
    ExecuteControllerMeasurementSequenceAtComputedTarget();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF79, FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 4u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, 5u);
        return 2u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 1u)
    {
        RetryControllerMeasurementEvaluationWithReadPositionRecovery();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7A, FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F));
    }

    if(IsControllerMeasurementSeriesRetryEligible() == 1u)
    {
        CopyControllerMeasurementSeries00Through05To07Through12();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) | 4u));
        InitializeControllerMeasurementSeries00And01FromCenteredSlopes();
        ExecuteControllerMeasurementSequenceAtComputedTarget();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7B, FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 1u)
        {
            RetryControllerMeasurementEvaluationWithReadPositionRecovery();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7C, FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F));
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xfdu));
    ExecuteControllerMeasurementSequenceAtComputedTarget();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7D, FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 4u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF74) + 2u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF74) + 1u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF74) >= 0x28u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) == 0xb9u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, 0x60u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 4u) != 0u)
        combined_status = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7A) | FirmwareReadNear8(FW_NEAR_UNKNOWN_EF79) | FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7B) | FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7C)
                               | FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7D));
    else
        combined_status = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7A) | FirmwareReadNear8(FW_NEAR_UNKNOWN_EF79) | FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7D));

    if(combined_status != 1u)
        return 2u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF80) == 1u)
        return 1u;
    return 3u;
}
