#include "controller_calibration_outcome.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_calibration_outcome_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. */
static s16 ReadCalibrationOutcomeWord(fw_addr_t address)
{
    u8 high = FirmwareRead8(address);
    u8 low = FirmwareRead8(address + 1u);

    return (s16)(((u16)high << 8) | low);
}

static void PublishFixedCalibrationOutcome(u8 outcome, u8 status)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F972, outcome);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, status);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F974, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F975, 6u);
}

/* RAM:f8ee55 */
void UpdateCalibrationOutcomeStateForStage(u8 calibration_stage)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F972, 0u);

    if(calibration_stage == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB6) < 0x60u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB6) > 0xa0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F973) != 1u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F974, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F973, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F972, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x11u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F974, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F974) + 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F975, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F975) + 1u));
        }
    }
    else if(calibration_stage == 2u)
    {
        if(ReadCalibrationOutcomeWord(FW_NEAR_UNKNOWN_FABA) <= (s16)-6144 || ReadCalibrationOutcomeWord(FW_NEAR_UNKNOWN_FABA) >= (s16)6144)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F973) != 2u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F974, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F973, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F972, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x12u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F974, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F974) + 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F975, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F975) + 1u));
        }
    }
    else if(calibration_stage == 3u)
    {
        if((ReadCalibrationOutcomeWord(FW_NEAR_UNKNOWN_FAB8) <= (s16)-8192 || ReadCalibrationOutcomeWord(FW_NEAR_UNKNOWN_FAB8) >= (s16)6072) && FirmwareReadNear8(FW_NEAR_UNKNOWN_F973) != 3u)
            PublishFixedCalibrationOutcome(3u, 0x13u);
    }
    else if(calibration_stage == 4u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB1) != 1u)
            PublishFixedCalibrationOutcome(4u, 0x14u);
    }
    else if(calibration_stage == 5u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) == 0u)
        {
            if(FirmwareRead8(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFC020)) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F976, 0u);
                RESTART_CALIBRATION_OUTCOME_AT_F84B0B();
            }
            else
            {
                PublishFixedCalibrationOutcome(5u, 0x15u);
            }
        }
        else
        {
            PublishFixedCalibrationOutcome(5u, 0x15u);
        }
    }
    else if(calibration_stage == 0u && (((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) < 0) || FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5) == 0xffu || FirmwareReadNear8(FW_NEAR_UNKNOWN_FABC) == 0u))
    {
        PublishFixedCalibrationOutcome(6u, 0x16u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F972) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F974) < 3u && FirmwareReadNear8(FW_NEAR_UNKNOWN_F975) < 6u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F976, 1u);
            RESTART_CALIBRATION_OUTCOME_AT_F84B0B();
        }
        else
        {
            RESOLVE_CALIBRATION_OUTCOME_DRIVE_CONTROL();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F976, 0u);
            RESTART_CALIBRATION_OUTCOME_AT_F8F07C();
        }
    }
}
