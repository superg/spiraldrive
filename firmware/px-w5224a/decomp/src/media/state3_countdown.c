#include "state3_countdown.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/state3_countdown_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. */
static u32 ReadState3CountdownDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:f8eb9b */
void ServiceMediaState3CountdownAndTransition(void)
{
    u8 countdown;
    u8 track_number;
    s32 track_position;
    s32 lead_out_distance;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E1) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E1, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB0) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            RESTART_STATE3_COUNTDOWN_COROUTINE1();
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F977) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E1) == 0xffu)
        {
            WriteBigEndianU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadBigEndianU16(FW_NEAR_UNKNOWN_FAAC) & 0xfffbu));
            ReadDriveConfigurationBytes(0x002ful, FW_NEAR_UNKNOWN_FAC2, 1u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC2) == 0xffu)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC2, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8) + 0x80u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC2, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC2) + 4u));
        }
    }

    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
    track_position = CONVERT_STATE3_COUNTDOWN_TRACK_POSITION(FW_HUGE_UNKNOWN_2BB03E + (fw_addr_t)((u16)track_number * 10u));
    lead_out_distance = (s32)(ReadState3CountdownDword(FW_NEAR_CURRENT_FRAME_POSITION) - (u32)track_position);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu && FirmwareReadNear8(FW_NEAR_UNKNOWN_F977) == 3u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) >= 94u || lead_out_distance <= 126000l))
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        RESTART_STATE3_COUNTDOWN_COROUTINE1();
    }

    countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E1);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E1, (u8)(countdown - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x04u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x08u));
}
