#include "scaled_radial_pickup_move_sequence.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "pickup_move_outputs.h"
#include "radial_pickup_parameter_sequence.h"
#include "radial_pickup_restore.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/scaled_radial_pickup_move_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern s16 DivideS16Quotient(s16 dividend, s16 divisor);

static u16 ReadScaledRadialMoveWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadScaledRadialMoveDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u8 ReadScaledRadialMoveProfile(u16 index)
{
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_10CA + index);
}

/* RAM:fd83d0 */
void PLEXTOR_NEAR_FUNCTION ExecuteScaledRadialPickupMoveSequence(u8 inward_direction, u16 scaled_magnitude)
{
    u8 waveform[109];
    u8 leading_magnitude;
    u8 pulse_count;
    u8 index;
    u8 output_control;
    u16 adjusted_magnitude;
    u16 residual_magnitude;
    u16 aligned_magnitude;
    u16 half_magnitude;
    s16 remaining_magnitude;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1130, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1131, 1u);

    if(scaled_magnitude < 0x60u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1130, 0u);
        CONFIGURE_MOVE(inward_direction);
        EXECUTE_MOVE(inward_direction, scaled_magnitude);
    }
    else
    {
        leading_magnitude = CONFIGURE_OUTPUTS(inward_direction);
        adjusted_magnitude = (u16)(scaled_magnitude - leading_magnitude);
        residual_magnitude = (u16)(adjusted_magnitude & 0x001fu);
        aligned_magnitude = (u16)(adjusted_magnitude - residual_magnitude);
        pulse_count = (u8)((u16)DivideS16Quotient((s16)aligned_magnitude, 0x20u) - 1u);
        half_magnitude = (u16)DivideS16Quotient((s16)aligned_magnitude, 2u);

        CONFIGURE_MOVE(inward_direction);
        if(leading_magnitude != 0u)
            EXECUTE_MOVE(inward_direction, leading_magnitude);

        WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
        WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_018496, 0x80u);
        WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_018497, 0x80u);

        index = 0u;
        remaining_magnitude = (s16)aligned_magnitude;
        if(half_magnitude > ReadScaledRadialMoveWord(FW_NEAR_UNKNOWN_10C8))
        {
            do
            {
                if((u32)index < ReadScaledRadialMoveDword(FW_NEAR_UNKNOWN_10C4))
                {
                    waveform[index] = ReadScaledRadialMoveProfile(index);
                }
                else if((u32)index < ReadScaledRadialMoveDword(FW_NEAR_UNKNOWN_10C4) || (u32)((u32)pulse_count - ReadScaledRadialMoveDword(FW_NEAR_UNKNOWN_10C4)) <= (u32)index)
                {
                    if((u32)((u32)pulse_count - ReadScaledRadialMoveDword(FW_NEAR_UNKNOWN_10C4)) <= (u32)index)
                        waveform[index] = ReadScaledRadialMoveProfile((u16)(99u + (u16)index - (u16)pulse_count));
                }
                else
                {
                    waveform[index] = ReadScaledRadialMoveProfile((u16)(ReadScaledRadialMoveDword(FW_NEAR_UNKNOWN_10C4) + 1ul));
                }
                ++index;
                remaining_magnitude = (s16)(remaining_magnitude - 0x20);
            } while(remaining_magnitude >= 0);
        }
        else
        {
            do
            {
                if(pulse_count < 2u || (s16)(u16)index < DivideS16Quotient((s16)(u16)pulse_count, 2u))
                    waveform[index] = ReadScaledRadialMoveProfile(index);
                else
                    waveform[index] = ReadScaledRadialMoveProfile((u16)(99u + (u16)index - (u16)pulse_count));
                ++index;
                remaining_magnitude = (s16)(remaining_magnitude - 0x20);
            } while(remaining_magnitude >= 0);
        }

        index = 0u;
        while(index < pulse_count)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, FirmwareReadNear8(FW_NEAR_UNKNOWN_10BE) == 0u ? 0xffu : 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, FirmwareReadNear8(FW_NEAR_UNKNOWN_10BF) == 0u ? 0xffu : 0u);
            WAIT_MOVE_DURATION((u16)waveform[index]);
            WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_018494, FirmwareReadNear8(FW_NEAR_UNKNOWN_10BE));
            WAIT_MOVE_DURATION((u16)waveform[index]);
            WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_018495, FirmwareReadNear8(FW_NEAR_UNKNOWN_10BF));
            ++index;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_10BE) == 0xffu)
        {
            if(inward_direction == 0u)
            {
                output_control = 0xc0u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0x80u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, 0xffu);
            }
            else
            {
                output_control = 0x40u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0x80u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, 0u);
            }
        }
        else if(inward_direction == 0u)
        {
            output_control = 0x40u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0x80u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, 0u);
        }
        else
        {
            output_control = 0xc0u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BE, 0x80u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BF, 0xffu);
        }

        WAIT_MOVE_DURATION((u16)waveform[index]);
        WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_018494, FirmwareReadNear8(FW_NEAR_UNKNOWN_10BE));
        WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
        WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_01849F, output_control);
        WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0x80u);

        residual_magnitude = (u16)(residual_magnitude + 0x10u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1131, 0u);
        if(residual_magnitude == 0u)
            WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_018496, 0xc9u);
        else
            EXECUTE_MOVE(inward_direction, residual_magnitude);
    }

    WRITE_MOVE_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    RESTORE_MOVE_STATE();
}
