#include "controller_state6_operating_path.h"
#include "controller_state6_calibration_finalization.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_state6_operating_path_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void SetDriveControlFailure(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x04u));
}

static void PrepareState6HomingCalibration(void)
{
    STATE6_HOME_PICKUP();
    STATE6_SET_P1_PINS();
    STATE6_PULSE_WORD(0x0050u);
    STATE6_SET_CHANNEL(3u, 0xaau);
}

/* RAM:fb0f85 */
u8 PLEXTOR_NEAR_FUNCTION EstablishControllerDriveState6OperatingPath(void)
{
    u8 retry_failed;

    STATE6_PROBE_CONTROLLER();
    if(STATE6_VALIDATE_PATH() == 0u)
        return 0u;
    if(STATE6_ENTER_STATE5() == 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u)
    {
        STATE6_HOME_PICKUP();
        STATE6_CONFIGURE_PROFILE(0u, 0x0400u);
        STATE6_WRITE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    }

    if(STATE6_RETRY_CALIBRATION(1u) != 1u)
    {
        retry_failed = 1u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u)
        {
            PrepareState6HomingCalibration();
            if(STATE6_RETRY_CALIBRATION(1u) == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBEC, 2u);
                retry_failed = 0u;
            }
        }
        if(retry_failed != 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB98) == 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB98, 2u);
            else
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB98, 0u);
            SetDriveControlFailure();
            return 0u;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u)
    {
        STATE6_RESET_CHANNEL(0u);
        PrepareState6HomingCalibration();
        if(STATE6_RETRY_CALIBRATION(1u) != 1u)
        {
            SetDriveControlFailure();
            return 0u;
        }
    }

    if(STATE6_IS_INDEXED_STATE(5u) == 0u)
        STATE6_CLEAR_P3_PINS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 6u);

    if(STATE6_CALIBRATE_PARAMETER_FB() != 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) | 0x40u));
        STATE6_SET_CONTROLLER_FLAG(0u);
        SetDriveControlFailure();
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        return STATE6_FINALIZE_CALIBRATION() == 1u ? 1u : 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 1u);
    STATE6_LOAD_PROFILE(4u);
    STATE6_CONFIGURE_PORT8006(1u);
    STATE6_APPLY_CHANNELS();
    STATE6_CONFIGURE_RANGE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1209, 5u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) > 0)
        if(STATE6_READ_P0_BIT7() != 0u)
            return 1u;

    SetDriveControlFailure();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB98, 1u);
    return 0u;
}
