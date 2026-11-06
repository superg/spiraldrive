#include "controller_current_state_calibration.h"
#include "controller_words0204_0319_calibration.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_current_state_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u8 FailCurrentStateCalibration(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x04u));
    return 0u;
}

static void PublishCurrentStateCalibrationReady(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 1u);
    CURRENT_CALIBRATION_SET_P3_HIGH();
}

/* RAM:fb1e1f */
u8 PLEXTOR_NEAR_FUNCTION RunControllerCalibrationForCurrentDriveState(void)
{
    u8 operating_enabled = FirmwareReadNear8(FW_NEAR_UNKNOWN_1006);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x01u) != 0x01u)
        return 1u;

    if(operating_enabled == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 5u)
            {
                if(CURRENT_CALIBRATION_RUN_FULL() != 1u)
                    return FailCurrentStateCalibration();
                PublishCurrentStateCalibrationReady();
            }
            else if(CURRENT_CALIBRATION_APPLY_STORED_WORD() == 0u)
            {
                return FailCurrentStateCalibration();
            }
        }
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x20u) == 0u)
        {
            if(CURRENT_CALIBRATION_APPLY_STORED_WORD() == 0u)
                return FailCurrentStateCalibration();
        }
        else if(CURRENT_CALIBRATION_RUN_FULL() != 1u)
        {
            return FailCurrentStateCalibration();
        }
    }
    else
    {
        CURRENT_CALIBRATION_YIELD();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 5u)
            {
                if(CURRENT_CALIBRATION_PULSE_TEST(10u) == 1u)
                {
                    if(CURRENT_CALIBRATION_RUN_FULL() != 1u)
                        return FailCurrentStateCalibration();
                }
                else
                {
                    if(CURRENT_CALIBRATION_APPLY_STORED_WORD() == 0u)
                        return FailCurrentStateCalibration();
                    PublishCurrentStateCalibrationReady();
                }
            }
            else if(CURRENT_CALIBRATION_APPLY_STORED_WORD() == 0u)
            {
                return FailCurrentStateCalibration();
            }
        }
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x04u) == 0u)
        {
            if(CURRENT_CALIBRATION_APPLY_STORED_WORD() == 0u)
                return FailCurrentStateCalibration();
        }
        else if(CURRENT_CALIBRATION_PULSE_TEST(10u) == 1u && CURRENT_CALIBRATION_RUN_FULL() != 1u)
        {
            return FailCurrentStateCalibration();
        }
    }

    if(CURRENT_CALIBRATION_CAPTURE_REFERENCES() == 0u)
        return FailCurrentStateCalibration();
    return 1u;
}
