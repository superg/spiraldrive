#include "controller_port80f8_calibration_attempt.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_port80f8_calibration_attempt_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_port80f8_calibration_attempt_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void ResetPort80F8AttemptChannel(void)
{
    PORT80F8_RESET_CHANNEL(0u);
    PORT80F8_SET_CHANNEL(0u, 0x99u);
}

/* RAM:fb1431 */
u8 PLEXTOR_NEAR_FUNCTION RunControllerPort80F8CalibrationAttempt(u8 calibration_mode)
{
    u8 error_count = 0u;
    u8 parameter_value = 0u;
    u8 last_count;

    if(calibration_mode == 1u)
    {
        PORT80F8_SET_FLAG(0u);
        PORT80F8_WRITE_PARAMETERS(parameter_value);
    }

    PORT80F8_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180AC, 1u);
    while((PORT80F8_READ_PORT(FW_CONTROLLER_UNKNOWN_0180AC) & 0x7fu) != 0u)
    {
    }
    PORT80F8_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B0, 0x40u);
    PORT80F8_WAIT_FOR_CLEAR();

    if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 2u) == 0u)
    {
        if(calibration_mode == 1u)
        {
            PORT80F8_SET_FLAG(1u);
            PORT80F8_WAIT_TIMER(20u, 0u);
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) >= 0)
            {
                parameter_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5);
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
                    parameter_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5);
            }
            PORT80F8_WRITE_PARAMETERS(parameter_value);
        }

        PORT80F8_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B0, 0x30u);
        PORT80F8_WAIT_FOR_CLEAR();
        if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 2u) == 0u)
        {
            if(calibration_mode == 1u)
            {
                PORT80F8_SET_FLAG(0u);
                PORT80F8_WRITE_PARAMETERS(0u);
            }
            return 0u;
        }
    }

    PORT80F8_SET_CHANNEL(0u, 0x9du);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 50u);
    last_count = 50u;
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) >= 0)
    {
        if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u
            && (s16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) + 2u) <= (s16)(u16)last_count)
        {
            last_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_1220);
            if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 1u) == 0u)
                error_count++;
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 1u) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u)
    {
        PORT80F8_WRITE_SHARED_COUNTDOWN(250l);
        while(PORT80F8_READ_SHARED_COUNTDOWN() > 0l)
        {
            PORT80F8_YIELD();
            if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 1u) == 0u)
            {
                ResetPort80F8AttemptChannel();
                return 0u;
            }
        }
        return 1u;
    }

    if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 1u) == 0u || error_count >= 4u)
    {
        ResetPort80F8AttemptChannel();
        return 0u;
    }
    return 1u;
}
