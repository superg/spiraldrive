#include "controller_state6_position_window.h"
#include "controller_operating_window.h"
#include "controller_parameters.h"
#include "controller_progress.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_state6_position_window_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadState6PositionWindowU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadState6PositionWindowU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteState6PositionWindowU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteState6PositionWindowU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void ClearState6PositionWindowEnable(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
}

static void FinishState6PositionWindow(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    ClearState6PositionWindowEnable();
}

static s32 ReadState6AlternatePositionBase(void)
{
    return (s32)(ReadState6PositionWindowU32(FW_NEAR_UNKNOWN_0FE9) - (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE));
}

static u8 WaitForState6PositionWindowPhase(u16 phase_mask)
{
    for(;;)
    {
        if(WINDOW_POLL() != 1u)
        {
            FinishState6PositionWindow();
            return 1u;
        }
        if((ReadState6PositionWindowU16(FW_NEAR_UNKNOWN_0FF5) & phase_mask) == 0u)
        {
            WINDOW_APPLY_PARAMETERS();
            FinishState6PositionWindow();
            return 1u;
        }
        WINDOW_YIELD();
    }
}

/* RAM:f8974a */
u8 PLEXTOR_NEAR_FUNCTION RunMediaState6ControllerCalibrationPositionWindow(void)
{
    s32 primary_target;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x40u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
    WINDOW_ARM();
    WriteState6PositionWindowU32(FW_NEAR_UNKNOWN_0FE5, 0x10000000ul);
    primary_target = (s32)(ReadState6PositionWindowU32(FW_NEAR_UNKNOWN_0FE9) - 0x48aul - (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) * 15u)
                           + (u32)(s32)(s16)((s16)(3 - (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEF)) * 5));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA04, 0x28u);
    WINDOW_WRITE_E8E9(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));

    for(;;)
    {
        s32 current_position;

        if(WINDOW_POLL() != 1u)
        {
            ClearState6PositionWindowEnable();
            return 0u;
        }
        current_position = (s32)ReadState6PositionWindowU32(FW_NEAR_UNKNOWN_1235);
        if((ReadState6PositionWindowU16(FW_NEAR_UNKNOWN_0FF5) & 1u) != 0u)
        {
            if(current_position == primary_target)
            {
                WriteState6PositionWindowU16(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadState6PositionWindowU16(FW_NEAR_UNKNOWN_0FF5) | 2u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FED, 0u);
                return WaitForState6PositionWindowPhase(2u);
            }
            if(current_position == (s32)((u32)primary_target + 1ul) || current_position >= (s32)((u32)primary_target + 8ul))
            {
                ClearState6PositionWindowEnable();
                return 0u;
            }
        }
        else
        {
            if(current_position == (s32)(ReadState6PositionWindowU32(FW_NEAR_UNKNOWN_0FE9) - 0x486ul) && WINDOW_OPERATING(1u) == 0u)
            {
                WINDOW_RESTORE(0u);
                if(WINDOW_READINESS() == 0u)
                {
                    ClearState6PositionWindowEnable();
                    return 0u;
                }
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x40u);
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
                continue;
            }
            if(current_position == (s32)((u32)ReadState6AlternatePositionBase() - 0x409ul) && WINDOW_OPERATING(1u) == 0u)
            {
                ClearState6PositionWindowEnable();
                return 0u;
            }
            if(current_position == (s32)((u32)ReadState6AlternatePositionBase() - 0x408ul))
            {
                WriteState6PositionWindowU16(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadState6PositionWindowU16(FW_NEAR_UNKNOWN_0FF5) | 4u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FED, 0u);
                return WaitForState6PositionWindowPhase(4u);
            }
            if(current_position == (s32)((u32)ReadState6AlternatePositionBase() - 0x407ul) || current_position >= (s32)((u32)ReadState6AlternatePositionBase() - 0x402ul))
            {
                ClearState6PositionWindowEnable();
                return 0u;
            }
        }
        WINDOW_YIELD();
    }
}
