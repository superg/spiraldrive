#include "pickup_move_outputs.h"
#include "controller_measurement_transition.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/pickup_move_outputs_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static s32 ReadScaledRadialMoveS32(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

/* RAM:fd8753 */
void PLEXTOR_NEAR_FUNCTION ConfigureControllerStateForScaledRadialPickupMove(u8 inward_direction)
{
    u8 controller_state;
    u8 operating_code;
    s32 threshold_value;
    s32 target_position;

    WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_0180E1, 0xb0u);
    WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_0180E2, 0x52u);
    CLEAR_MOVE_EXTERNAL_INTERRUPT1();
    controller_state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xfbu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_state);
    CLEAR_MOVE_CONTROLLER_CHANNEL86();
    WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_0180A2, 0x92u);
    SUBMIT_MOVE_CONTROLLER_WORD(0u, 0x0251u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 2u);
    WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_0180A4, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        CLEAR_MOVE_PARAMETER_D1_BIT6();
    WRITE_MOVE_PARAMETERS_D2_D3();
    WRITE_MOVE_PARAMETER_WORD((u16)(0xe700u | ((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15) & 0x00bfu)));

    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    if(operating_code >= 0x18u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 0xffu);
        operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
        if(operating_code < 0x20u)
        {
            threshold_value = ReadScaledRadialMoveS32(FW_NEAR_UNKNOWN_F9EE);
            target_position = ReadScaledRadialMoveS32(FW_NEAR_UNKNOWN_0E3B);
            if(threshold_value < target_position)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 0x18u);
        }
        else
        {
            operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
            if(operating_code < 0x28u)
            {
                threshold_value = ReadScaledRadialMoveS32(FW_NEAR_UNKNOWN_F9F2);
                target_position = ReadScaledRadialMoveS32(FW_NEAR_UNKNOWN_0E3B);
                if(threshold_value < target_position)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 0x20u);
            }
            else
            {
                operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
                if(operating_code < 0x34u)
                {
                    threshold_value = ReadScaledRadialMoveS32(FW_NEAR_UNKNOWN_F9F6);
                    target_position = ReadScaledRadialMoveS32(FW_NEAR_UNKNOWN_0E3B);
                    if(threshold_value < target_position)
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 0x28u);
                }
            }
        }
    }

    APPLY_MOVE_TRANSITION_PROFILE(inward_direction);
}

/* RAM:fd8b16 */
u8 PLEXTOR_NEAR_FUNCTION ConfigureScaledRadialMovePulseOutputs(u8 inward_direction)
{
    u8 sample = (u8)(READ_MOVE_PORT(FW_CONTROLLER_UNKNOWN_01849F) >> 2);
    u8 displacement = 0u;

    if(sample == 0x20u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, inward_direction == 0u ? 0xffu : 0u);
    }
    else if(sample < 0x20u)
    {
        if(inward_direction == 0u)
        {
            displacement = (u8)(0x20u - sample);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0xffu);
        }
        else
        {
            displacement = (u8)(sample + 0x20u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, 0u);
        }
    }
    else if(inward_direction == 0u)
    {
        displacement = (u8)(0x60u - sample);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0xffu);
    }
    else
    {
        displacement = (u8)(sample - 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, 0u);
    }

    WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_018494, FirmwareReadNear8(FW_NEAR_UNKNOWN_10BE));
    WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_018495, FirmwareReadNear8(FW_NEAR_UNKNOWN_10BF));
    return displacement;
}

/* RAM:fb084e */
void PLEXTOR_NEAR_FUNCTION HomePickupAgainstP1Bit2AndResetEstimate(void)
{
    u8 saved_p1_bit1;

    HOME_PICKUP_WRITE_PORT(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
    saved_p1_bit1 = READ_HOME_PICKUP_P1_BIT1();
    WRITE_HOME_PICKUP_P1_BIT1(1u);

    HOME_PICKUP_CONFIGURE_DRIVE(3u, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 120u);
    for(;;)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED) & 4u) != 0u)
            goto stop_drive;
        HOME_PICKUP_YIELD();
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1216) < 0)
            break;
    }

    HOME_PICKUP_WRITE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 5u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1216) >= 0)
        HOME_PICKUP_YIELD();

    HOME_PICKUP_CONFIGURE_DRIVE(4u, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 120u);
    for(;;)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED) & 4u) != 0u)
            break;
        HOME_PICKUP_YIELD();
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1216) < 0)
        {
            HOME_PICKUP_WRITE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
            break;
        }
    }

stop_drive:
    HOME_PICKUP_WRITE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    HOME_PICKUP_CONFIGURE_DRIVE(4u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 9u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1216) >= 0)
        HOME_PICKUP_YIELD();

    HOME_PICKUP_WRITE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1031, 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1032, 0x55u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1030, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1216, 1u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1216) >= 0)
        HOME_PICKUP_YIELD();

    WRITE_HOME_PICKUP_P1_BIT1(saved_p1_bit1);
    HOME_PICKUP_RESET_CHANNEL2();
}
