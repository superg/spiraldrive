#include "play_track_relative.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/play_track_relative_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPlayTrackRelativeU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePlayTrackRelativeU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static fw_addr_t PlayTrackRelativeTupleAddress(u16 base, u8 track_number)
{
    return (fw_addr_t)(u16)(base + (u16)track_number * 6u);
}

/* RAM:fd4631 */
void HandlePlayTrackRelative12CommandBody(void)
{
    u8 track_number;
    s32 relative_start;
    u32 frame_count;
    u32 start_frame;
    u32 end_frame;
    u32 boundary_frame = 0ul;

    if(VALIDATE_PLAY_TRACK_RELATIVE_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D70)) == 0u)
    {
        REJECT_PLAY_TRACK_RELATIVE_COMMAND();
        return;
    }

    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14);
    relative_start = (s32)ReadPlayTrackRelativeU32(FW_NEAR_UNKNOWN_0B0C);
    if(relative_start <= 0l)
        relative_start = 0l;
    frame_count = ReadPlayTrackRelativeU32(FW_NEAR_UNKNOWN_0B10);

    if(VALIDATE_PLAY_TRACK_RELATIVE_TRACK(track_number, track_number, 1u, 1u) == 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        start_frame = (u32)CONVERT_PLAY_TRACK_RELATIVE_MSF(PlayTrackRelativeTupleAddress(0xf2b9u, track_number)) + (u32)relative_start;
        boundary_frame = (u32)CONVERT_PLAY_TRACK_RELATIVE_MSF(PlayTrackRelativeTupleAddress(0xf2bfu, track_number));
        if(start_frame >= boundary_frame)
        {
            REJECT_PLAY_TRACK_RELATIVE_COMMAND();
            return;
        }

        boundary_frame = ReadPlayTrackRelativeU32(FW_NEAR_UNKNOWN_12C1);
        if(frame_count >= boundary_frame)
            frame_count = boundary_frame;
        end_frame = start_frame + frame_count;
        if(end_frame >= boundary_frame)
            end_frame = boundary_frame - 1ul;
    }
    else
    {
        LOAD_PLAY_TRACK_RELATIVE_TRACK(track_number);
        start_frame = ReadPlayTrackRelativeU32(FW_NEAR_UNKNOWN_DD3D) + (u32)relative_start;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u && start_frame >= (u32)SELECT_PLAY_TRACK_RELATIVE_BOUNDARY())
        {
            REJECT_PLAY_TRACK_RELATIVE_COMMAND();
            return;
        }

        LOAD_PLAY_TRACK_RELATIVE_TRACK(GET_PLAY_TRACK_RELATIVE_LAST_TRACK());
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u)
        {
            boundary_frame = (u32)SELECT_PLAY_TRACK_RELATIVE_BOUNDARY();
            if(frame_count >= boundary_frame)
                frame_count = boundary_frame;
        }
        end_frame = start_frame + frame_count;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u && end_frame >= boundary_frame)
            end_frame = boundary_frame - 1ul;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 5u);
    if(frame_count == 0ul)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 3u);
        WritePlayTrackRelativeU32(FW_NEAR_UNKNOWN_0C8E, start_frame);
        (void)DISPATCH_PLAY_TRACK_RELATIVE_ACQUISITION();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8D, 0u);
    CLEAR_PLAY_TRACK_RELATIVE_BIT0018();
    WritePlayTrackRelativeU32(FW_NEAR_UNKNOWN_0F36, end_frame - start_frame);
    START_PLAY_TRACK_RELATIVE_READ((s32)start_frame);
}
