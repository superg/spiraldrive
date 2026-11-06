#include "scsi_opcode_ba_cd.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/scsi_opcode_ba_cd_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadOpcodeBaCdU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteOpcodeBaCdU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static fw_addr_t OpcodeBaCdTrackTupleAddress(u16 base, u8 track_number)
{
    return (fw_addr_t)(u16)(base + (u16)track_number * 6u);
}

/* RAM:fd487b */
void HandleScsiOpcodeBA_CD(void)
{
    u8 positioning_mode;
    u8 track_number;
    u8 last_track;
    u8 reverse_direction;
    s32 start_frame = 0l;
    s32 terminal_frame;

    if(VALIDATE_OPCODE_BA_CD_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E57)) == 0u)
    {
        REJECT_OPCODE_BA_CD();
        return;
    }

    positioning_mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13) >> 6);
    reverse_direction = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x10u) != 0u);
    if(positioning_mode == 0u)
    {
        start_frame = (s32)(ReadOpcodeBaCdU32(FW_NEAR_UNKNOWN_0B0C) + 150ul);
    }
    else if(positioning_mode == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) > 75u || VALIDATE_OPCODE_BA_CD_MSF(FW_NEAR_UNKNOWN_0B0D) == 0u)
        {
            REJECT_OPCODE_BA_CD();
            return;
        }
        start_frame = CONVERT_OPCODE_BA_CD_MSF(FW_NEAR_UNKNOWN_0B0D);
    }
    else if(positioning_mode == 2u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E) != 0u)
        {
            REJECT_OPCODE_BA_CD();
            return;
        }
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        {
            last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
            if(track_number == 0u || track_number > last_track)
            {
                REJECT_OPCODE_BA_CD();
                return;
            }
            start_frame = CONVERT_OPCODE_BA_CD_MSF(OpcodeBaCdTrackTupleAddress(reverse_direction != 0u ? 0xf2bfu : 0xf2b9u, track_number));
        }
        else
        {
            last_track = GET_OPCODE_BA_CD_LAST_TRACK();
            if(track_number == 0u || track_number > last_track)
            {
                REJECT_OPCODE_BA_CD();
                return;
            }
            LOAD_OPCODE_BA_CD_TRACK(track_number);
            start_frame = (s32)ReadOpcodeBaCdU32(FW_NEAR_UNKNOWN_DD3D);
            if(reverse_direction != 0u)
                start_frame = SELECT_OPCODE_BA_CD_TRACK_BOUNDARY();
        }
    }
    else
    {
        REJECT_OPCODE_BA_CD();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        terminal_frame = (s32)ReadOpcodeBaCdU32(FW_NEAR_UNKNOWN_12C1);
        if(start_frame > (s32)((u32)terminal_frame + 75ul))
        {
            REJECT_OPCODE_BA_CD();
            return;
        }
    }
    else
    {
        last_track = GET_OPCODE_BA_CD_LAST_TRACK();
        if(last_track == 0u)
        {
            SET_OPCODE_BA_CD_SENSE(5u, 0x2100u);
            return;
        }
        LOAD_OPCODE_BA_CD_TRACK(last_track);
        terminal_frame = SELECT_OPCODE_BA_CD_TRACK_BOUNDARY();
    }
    WriteOpcodeBaCdU32(FW_NEAR_UNKNOWN_0C92, (u32)terminal_frame);

    if((u32)start_frame > (u32)terminal_frame + 75ul)
    {
        REJECT_OPCODE_BA_CD();
        return;
    }

    WRITE_OPCODE_BA_CD_BIT0018(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 4u));
    WRITE_OPCODE_BA_CD_BIT0019(reverse_direction);
    if(reverse_direction == 0u)
    {
        WriteOpcodeBaCdU32(FW_NEAR_UNKNOWN_0F36, (u32)terminal_frame - (u32)start_frame);
    }
    else
    {
        WriteOpcodeBaCdU32(FW_NEAR_UNKNOWN_0F36, (u32)start_frame - 150ul);
        start_frame = (s32)((u32)start_frame - 18ul);
    }
    START_OPCODE_BA_CD_READ(start_frame);
}
