#include "controller_state6_calibration_finalization.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_state6_calibration_finalization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void PublishState6FinalizationFailure(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x04u));
}

static u8 WaitForState6CalibrationFlag(u8 countdown)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, countdown);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) > 0)
    {
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 1) & 1u) != 1u)
        {
            PublishState6FinalizationFailure();
            return 0u;
        }
    }
    return 1u;
}

/* RAM:fb1aef */
u8 PLEXTOR_NEAR_FUNCTION FinalizeControllerState6OperatingCalibration(void)
{
    u8 failed_pulse_count = 0u;

    if(WaitForState6CalibrationFlag(5u) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u && FINAL_PULSE(10u) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 0u);

    FINAL_LOAD_PROFILE(4u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        FINAL_CONFIGURE_PORT8006(1u);
    FINAL_APPLY_CHANNELS();
    FINAL_CONFIGURE_RANGE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 2u);

    if(WaitForState6CalibrationFlag(20u) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1209, 15u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) > 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, 30u);
        while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) >= 0)
        {
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 1) & 1u) != 1u)
            {
                PublishState6FinalizationFailure();
                return 0u;
            }
            if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802A) & 0x40u) != 0u)
                break;
            FINAL_YIELD();
        }

        if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01802A) & 0x40u) != 0u)
            break;
        if(FINAL_PULSE(10u) == 1u)
        {
            failed_pulse_count = 0u;
        }
        else
        {
            failed_pulse_count = (u8)(failed_pulse_count + 1u);
            if(failed_pulse_count >= 3u)
                break;
        }
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) > 0)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1008, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 1u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FINAL_STROBE(FW_CONTROLLER_UNKNOWN_01803B, 0x88u);
        FINAL_STROBE(FW_CONTROLLER_UNKNOWN_018079, 0x10u);
        FINAL_CONFIGURE_P0(0u);
        FINAL_WAIT_FOR_TOLERANCE(0u, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB98, 0u);
        PublishState6FinalizationFailure();
        return 0u;
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1008, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 0u);
    }

    FINAL_LOAD_PROFILE(4u);
    FINAL_CONFIGURE_PORT8006(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if(FINAL_IS_INDEXED(5u) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0xbfu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB6, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16));
            FINAL_UPDATE_OUTCOME(1u);
        }
    }
    else if(FINAL_IS_INDEXED(5u) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0xbfu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB6, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16));
        FINAL_UPDATE_OUTCOME(1u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1209, 5u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) > 0)
        if(FINAL_READ_P0_BIT7() != 0u)
            return 1u;

    PublishState6FinalizationFailure();
    return 0u;
}
