#include "write_command_range.h"
#include "media/write_command_raw_read_pipeline.h"
#include "media/write_pipeline_q.h"
#include "mmc_write.h"
#include "platform/coroutine_initialization.h"
#include "platform/firmware_memory.h"
#include "platform/leaf_workers.h"
#include "platform/memory.h"
#include "platform/status_mapping.h"
#include "platform/track_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/write_command_range_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadWriteRangeU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteWriteRangeU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 ReadWriteRangeU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteWriteRangeU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void PublishWriteRangeSpeedState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7C, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7A, WRITE_SELECT_LOWEST_SPEED());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7B, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C7D));
}

static u8 InitializeNewWriteRange(s32 start_lba, u32 transfer_count)
{
    u8 write_type;
    u8 option_flags;
    u8 command_flags;
    u8 controller_was_enabled;
    u16 speed_flags;
    s32 target_frame = (s32)((u32)start_lba + 150ul);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
    WRITE_NORMALIZE_SPEED();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 3u && (FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0) & 1u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) | 0x80u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x7fu));
    if(WRITE_VALIDATE_PARAMETERS() == 0u || WRITE_VALIDATE_MEDIA() == 0u)
        return 0u;

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
    if(write_type == 0u && (option_flags & 1u) != 0u)
    {
        WRITE_SEQUENCE_ERROR();
        return 0u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_158D) == 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) | 2u));
        }
        else if(write_type == 1u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 2u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) | 2u));
            }
            else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 2u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfdu));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, FirmwareReadNear8(FW_NEAR_UNKNOWN_158C));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_158C));
            }
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 2u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158C, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_158D) == 2u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38) > 8u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, 8u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 8u);
                }
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, 4u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
            }
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xefu) | ((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 2u) != 0u ? 0x10u : 0u)));

    if(write_type == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x7fu) | ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) == 0u ? 0x80u : 0u)));
    else if(write_type == 3u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 0x80u));

    if(write_type == 2u || write_type == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C39, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1592) > 0x50u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38) > 0x28u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, 0x28u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x28u);
        }
    }
    else if((s32)ReadWriteRangeU32(FW_NEAR_CURRENT_FRAME_POSITION) >= 0x00058fd4l)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C39, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38) > 0x28u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, 0x28u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x28u);
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x40u) == 0u)
    {
        WRITE_COPY16(FW_NEAR_UNKNOWN_FAD3, 0xe4f2u, 8u);
    }
    else
    {
        WRITE_COPY16(0xe4f2u, 0xfad3u, 8u);
        speed_flags = ReadWriteRangeU16(FW_NEAR_UNKNOWN_FAD7);
        WriteWriteRangeU16(FW_NEAR_UNKNOWN_FAD7, speed_flags);
    }

    command_flags = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xbfu) | ((option_flags & 1u) != 0u ? 0x40u : 0u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_158F) & 3u) != 0u && (command_flags & 0x80u) != 0u)
    {
        speed_flags = ReadWriteRangeU16(FW_NEAR_UNKNOWN_FAD7);
        speed_flags |= 0x8000u;
        speed_flags |= (FirmwareReadNear8(FW_NEAR_UNKNOWN_158F) & 2u) != 0u ? 0x4000u : 0x0080u;
        WriteWriteRangeU16(FW_NEAR_UNKNOWN_FAD7, speed_flags);
    }

    controller_was_enabled = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, command_flags);
    if(write_type == 3u)
    {
        if(controller_was_enabled == 0u)
            WRITE_SET_CONTROLLER(1u);
    }
    else if(controller_was_enabled != 0u)
    {
        WRITE_SET_CONTROLLER(0u);
    }

    if(write_type == 0u || write_type == 1u)
    {
        WRITE_CLEAR_TRACK();
        if((option_flags & 1u) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            {
                if(WRITE_VALIDATE_TRACK(start_lba) == 0u)
                    return 0u;
            }
            else if(WRITE_FIND_TRACK(start_lba) == 0u)
            {
                return 0u;
            }
        }
        else
        {
            if(WRITE_LOAD_SNAPSHOT(FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC)) == 0u)
                return 0u;
            if(target_frame != (s32)ReadWriteRangeU32(FW_NEAR_UNKNOWN_DD3D))
            {
                if(WRITE_INITIALIZE_HIGH_TRACK() == 0u)
                    return 0u;
                if(target_frame != (s32)ReadWriteRangeU32(FW_NEAR_UNKNOWN_DD3D))
                {
                    WRITE_UNSUPPORTED();
                    return 0u;
                }
            }
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            WRITE_SELECT_ALTERNATE_CURSOR();
        else
            WRITE_SELECT_CURSOR();
    }
    else if(write_type == 2u)
    {
        if((option_flags & 1u) == 0u && target_frame != (s32)ReadWriteRangeU32(FW_NEAR_UNKNOWN_DD6D) && target_frame != (s32)ReadWriteRangeU32(FW_NEAR_UNKNOWN_DD69))
        {
            WRITE_UNSUPPORTED();
            return 0u;
        }
    }
    else if(write_type == 3u && target_frame != (s32)ReadWriteRangeU32(FW_NEAR_UNKNOWN_DD69))
    {
        WRITE_UNSUPPORTED();
        return 0u;
    }

    if(WRITE_VALIDATE_TYPE() == 0u || WRITE_VALIDATE_TRACK_STATE() == 0u)
        return 0u;
    if((s32)ReadWriteRangeU32(FW_NEAR_CURRENT_FRAME_POSITION) < target_frame)
    {
        WRITE_UNSUPPORTED();
        return 0u;
    }
    if(WRITE_VALIDATE_PMA() == 0u)
        return 0u;

    if(write_type == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD61, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u ? 0u : 2u);
    if(write_type == 3u)
        WriteWriteRangeU32(FW_NEAR_UNKNOWN_DD65, ReadWriteRangeU32(FW_NEAR_UNKNOWN_DD69));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x10u));
    WriteWriteRangeU32(FW_NEAR_UNKNOWN_DD5B, 0ul);
    WriteWriteRangeU32(FW_NEAR_UNKNOWN_0B02, WRITE_MAP_ADDRESS((u32)start_lba));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    WRITE_RESET_RAW_READ();
    WRITE_INITIALIZE_COROUTINE(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    WRITE_YIELD();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        u8 status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        WRITE_SET_SENSE(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + status), ReadWriteRangeU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (u16)status * 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        return 0u;
    }
    if((option_flags & 1u) == 0u && WRITE_ENSURE_OPC() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && write_type == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u && WRITE_BUILD_RAW_Q() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfbu));
        return 0u;
    }
    if(transfer_count == 0ul)
    {
        if(WRITE_ACQUIRE_ZERO_RANGE((u32)start_lba) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
            return 0u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        WRITE_NO_SENSE();
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB69, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) | 1u));
    return 1u;
}

/* RAM:fbbb0d */
void PLEXTOR_NEAR_FUNCTION ExecuteWriteCommandRange(s32 start_logical_block_address, u32 transfer_block_count)
{
    u8 continuation;
    u8 command_flags;
    u8 write_type;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xefu));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) == 0u)
    {
        if(InitializeNewWriteRange(start_logical_block_address, transfer_block_count) == 0u)
            return;
    }
    else
    {
        if(WRITE_RECONCILE_START(start_logical_block_address) == 0u)
            return;
        if(transfer_block_count == 0ul)
        {
            WRITE_NO_SENSE();
            return;
        }
        if(start_logical_block_address == 0l)
            WriteWriteRangeU32(FW_NEAR_UNKNOWN_0B02, (u32)WRITE_SELECT_PACKET_PROGRESS());
        else
            WriteWriteRangeU32(FW_NEAR_UNKNOWN_0B02, WRITE_MAP_ADDRESS((u32)start_logical_block_address));
    }

    continuation = (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x10u) != 0u ? 1u : 0u;
    if(WRITE_VALIDATE_EXTENT(continuation, transfer_block_count) == 0u)
    {
        command_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && continuation == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_128C) != 0u)
            command_flags |= 0x80u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, command_flags);
        WRITE_SET_SENSE(5u, 0x2400u);
        WriteWriteRangeU16(FW_NEAR_UNKNOWN_0C3A, 0x2400u);
        return;
    }

    WriteWriteRangeU32(FW_NEAR_UNKNOWN_0B06, transfer_block_count);
    if(WRITE_INITIALIZE_PIPELINE(continuation) == 0u)
    {
        PublishWriteRangeSpeedState();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfbu));
        WRITE_UPDATE_FAILURE();
        return;
    }

    PublishWriteRangeSpeedState();
    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type == 1u)
        WRITE_ADVANCE_TYPE1(transfer_block_count, continuation);
    else if(write_type == 3u)
        WriteWriteRangeU32(FW_NEAR_UNKNOWN_DD65, ReadWriteRangeU32(FW_NEAR_UNKNOWN_DD65) + transfer_block_count);
    else if(write_type == 0u)
        WRITE_ADVANCE_AFTER(start_logical_block_address, transfer_block_count, continuation);
}
