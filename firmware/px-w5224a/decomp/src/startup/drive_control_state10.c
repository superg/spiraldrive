#include "drive_control_state10.h"
#include "platform/firmware_memory.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/drive_control_state10_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadDriveState10ConfigurationWord(void)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/startup/drive_control_state10_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteDriveState10ConfigurationWord(u16 value)
{
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/startup/drive_control_state10_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 ComputeDriveState10SecondTransitionOffset(u8 first_value)
{
    u16 scaled_value = (u16)((u16)first_value << 8);
    u8 offset = (u8)(s32)(((float)scaled_value * 12.0f) / 49152.0f);

    if((s8)first_value < (s8)0xb0)
        offset = (u8)(s32)(((float)scaled_value * 13.0f) / 49152.0f);
    return offset;
}

/* RAM:fb034e */
u8 RunDriveControlState10Sequence(void)
{
    const u8 first_value = 0x90u;
    const u8 second_value = 0xc8u;
    const u8 initial_value = 0xb0u;
    const u8 countdown_start = 0x5au;
    const u8 first_transition_offset = 3u;
    u8 second_transition_offset = 0u;
    u8 second_transition_enabled = 0u;
    u8 transition_pending = 0u;
    u8 phase_bit_set;
    u8 countdown;
    u8 result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 10u);
    FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x08u));

    if((s8)first_value < (s8)0xc0)
    {
        second_transition_enabled = 1u;
        second_transition_offset = ComputeDriveState10SecondTransitionOffset(first_value);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1208, countdown_start);
    phase_bit_set = DriveState10PhaseBitSet();
    DriveState10SubmitWord((u16)initial_value << 8);
    if(phase_bit_set != 0u)
        transition_pending = 1u;

    for(;;)
    {
        countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_1208);
        if((s8)countdown <= 0 || (DriveState10P1Bit7Low() != 1u && DriveState10P1Bit5Low() != 0u))
        {
            if((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1208) <= (u16)(countdown_start - first_transition_offset))
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1214, 15u);
                DriveState10SubmitWord((u16)(u8)(second_value + 0x12u) << 8);
                while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1214) > 0)
                    DriveState10Yield();
            }
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1208) <= 0)
            {
                result = 0u;
                DriveState10ClearPhaseBit();
            }
            else
            {
                result = 1u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfdu));
            }
            DriveState10ClearWord();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 8u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xdfu));
            return result;
        }

        if((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1208) <= (u16)(countdown_start - first_transition_offset) && transition_pending == 1u)
            DriveState10SubmitWord((u16)first_value << 8);
        if((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1208) <= (u16)(countdown_start - second_transition_offset) && second_transition_enabled != 0u && transition_pending != 0u)
        {
            DriveState10SubmitWord((u16)second_value << 8);
            transition_pending = 0u;
        }

        DriveState10Yield();
        countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_1208);
        if((u16)countdown <= (u16)(countdown_start - 30u) && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 2u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xbfu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 2u));
            DriveState10ClearWord();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 8u);
            return 2u;
        }
        if(DriveState10TakeStateFlag() == 1u)
        {
            DriveState10ClearWord();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 8u);
            return 2u;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F976) == 1u || (ReadDriveState10ConfigurationWord() & 0x4000u) != 0u) && (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1208) <= (u16)(countdown_start - 4u))
        {
            DriveState10ClearWord();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 8u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F976, 0u);
            WriteDriveState10ConfigurationWord((u16)(ReadDriveState10ConfigurationWord() & 0xbfffu));
            return 2u;
        }
    }
}
