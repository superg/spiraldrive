#include "controller_state6_calibration_positioning.h"
#include "../media/disc_read_position.h"
#include "controller_operating_state_initialization.h"
#include "controller_parameters.h"
#include "drive_control_reset.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_state6_calibration_positioning_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadState6PositioningU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadState6PositioningU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteState6PositioningU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void SelectState6Mode2OperatingCode(void)
{
    u8 flags_high = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF5);
    u8 operating_code;

    if((flags_high & 0x10u) == 0u)
        operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 16u ? 1u : 16u;
    else if((flags_high & 0x08u) != 0u)
        operating_code = 40u;
    else if((flags_high & 0x04u) != 0u)
        operating_code = 32u;
    else if((ReadState6PositioningU16(FW_NEAR_UNKNOWN_0FF5) & 0x0200u) != 0u)
        operating_code = 24u;
    else if((ReadState6PositioningU16(FW_NEAR_UNKNOWN_0FF5) & 0x0100u) != 0u)
        operating_code = 20u;
    else if((ReadState6PositioningU16(FW_NEAR_UNKNOWN_0FF5) & 0x0080u) != 0u)
        operating_code = 16u;
    else
        operating_code = 16u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, operating_code);
}

static void SelectState6CalibrationTarget(u8 target_mode)
{
    u16 scaled_speed;
    u32 target;

    if(target_mode == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 16u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 16u);
        target = ReadState6PositioningU32(FW_NEAR_UNKNOWN_0FE9) - 0x047eul;
        WriteState6PositioningU32(FW_NEAR_UNKNOWN_0E3B, target);
    }
    else if(target_mode == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 16u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 16u);
        target = ReadState6PositioningU32(FW_NEAR_UNKNOWN_0FE9) - 0x0494ul - (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) * 15u);
        WriteState6PositioningU32(FW_NEAR_UNKNOWN_0E3B, target);
    }
    else if(target_mode == 2u)
    {
        u8 bounded_speed;

        SelectState6Mode2OperatingCode();
        target = ReadState6PositioningU32(FW_NEAR_UNKNOWN_0FE9) - 0x0488ul - (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) * 15u);
        bounded_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 16u)
            bounded_speed = 16u;
        scaled_speed = (u16)((u16)bounded_speed * 75u);
        if(bounded_speed >= 12u)
            scaled_speed = (u16)(scaled_speed + (u16)((u16)bounded_speed * 37u));
        if(scaled_speed >= 0x0708u)
            scaled_speed = 0x0708u;
        target -= scaled_speed;
        WriteState6PositioningU32(FW_NEAR_UNKNOWN_0E3B, target);
    }
    else if(target_mode == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        target = ReadState6PositioningU32(FW_NEAR_UNKNOWN_0FE9) - 0x0488ul - (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) * 15u) - (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) * 18u);
        WriteState6PositioningU32(FW_NEAR_UNKNOWN_0E3B, target);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 8u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 8u);
            target = ReadState6PositioningU32(FW_NEAR_UNKNOWN_0FE9) - 0x05b4ul - (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) * 15u);
            WriteState6PositioningU32(FW_NEAR_UNKNOWN_0E3B, target);
        }
    }
    else if(target_mode == 4u)
    {
        target = ReadState6PositioningU32(FW_NEAR_UNKNOWN_0FE9) - 0x0406ul - FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 8u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            target -= (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) * 75u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 8u);
            target -= 600ul;
        }
        WriteState6PositioningU32(FW_NEAR_UNKNOWN_0E3B, target);
    }
}

static void TrapUnexpectedState6PositioningState(void)
{
    for(;;)
    {
    }
}

/* RAM:f881fd */
u8 PLEXTOR_NEAR_FUNCTION PositionReadAcquisitionForCalibrationMode(u8 target_mode, u8 require_tolerance)
{
    u8 attempt_count = 0u;
    u8 retry_flags = 3u;

    for(;;)
    {
        u8 acquisition_result;

        if(attempt_count > 2u)
        {
            if(target_mode != 4u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            return 0u;
        }

        SelectState6CalibrationTarget(target_mode);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        acquisition_result = POSITION_ACQUIRE();

        if(acquisition_result == 0u)
        {
            POSITION_RESET();
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3C) & 1u) != 0u)
                attempt_count = 2u;
            if(POSITION_INITIALIZE() == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 7u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
                return 0u;
            }
            POSITION_ENABLE(1u);
            ++attempt_count;
            continue;
        }
        if(acquisition_result == 0xffu)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 6u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u)
                TrapUnexpectedState6PositioningState();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            continue;
        }
        if(acquisition_result != 1u)
            TrapUnexpectedState6PositioningState();

        if(require_tolerance != 0u && POSITION_TOLERANCE((u8)(retry_flags & 1u)) != 1u)
        {
            retry_flags &= 0xfeu;
            continue;
        }
        if((ReadState6PositioningU16(FW_NEAR_UNKNOWN_0FF5) & 0x1000u) == 0u || target_mode != 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            return 1u;
        }
        if((retry_flags & 2u) != 0u)
        {
            retry_flags &= 0xfdu;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1217, 100u);
        }
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1217) <= 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            return 1u;
        }
    }
}
