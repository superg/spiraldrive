#include "playable_audio_track.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/playable_audio_track_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static s32 ReadPlayableAudioFramePosition(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

/* RAM:fd4c63 */
u8 PLEXTOR_NEAR_FUNCTION IsFramePositionOnPlayableAudioTrack(s32 frame_position)
{
    u8 track_number;
    u8 last_track;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
        while(track_number <= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) && CONVERT_PLAYABLE_AUDIO_MSF(FW_NEAR_UNKNOWN_F2BF + (fw_addr_t)((u16)track_number * 6u)) <= frame_position)
            track_number++;

        last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
        if(track_number > last_track)
            track_number = last_track;
        return (u8)((FirmwareRead8(FW_HUGE_UNKNOWN_00F2B7 + (fw_addr_t)((u16)track_number * 6u)) & 0x40u) == 0u);
    }

    track_number = GET_PLAYABLE_AUDIO_FIRST_TRACK();
    last_track = GET_PLAYABLE_AUDIO_LAST_TRACK();
    while(track_number <= last_track)
    {
        LOAD_PLAYABLE_AUDIO_TRACK((u8)(track_number + 1u), last_track);
        if(frame_position < (s32)FirmwareReadNearU32(FW_NEAR_UNKNOWN_DD3D))
            break;
        track_number++;
    }
    if(track_number > last_track)
        track_number = last_track;
    LOAD_PLAYABLE_AUDIO_TRACK(track_number, last_track);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u)
        return 0u;
    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    return (u8)(track_number == 4u || track_number == 5u || track_number == 6u || track_number == 7u);
}

/* RAM:fd4b11 */
u8 PLEXTOR_NEAR_FUNCTION ValidatePlayAudioFrameRange(s32 start_frame_position, s32 end_frame_position)
{
    s32 upper_bound;
    s32 probe_position;
    u8 last_track;

    if(end_frame_position < start_frame_position)
    {
        REJECT_PLAY_AUDIO_FRAME_RANGE();
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        upper_bound = (s32)((u32)ReadPlayableAudioFramePosition(FW_NEAR_UNKNOWN_12C1) + 75ul);
        if(start_frame_position > upper_bound || end_frame_position > upper_bound)
        {
            SET_PLAY_AUDIO_FRAME_RANGE_SENSE(5u, 0x2100u);
            return 0u;
        }
    }
    else
    {
        last_track = GET_PLAYABLE_AUDIO_LAST_TRACK();
        if(last_track == 0u)
        {
            SET_PLAY_AUDIO_FRAME_RANGE_SENSE(5u, 0x2100u);
            return 0u;
        }
        LOAD_PLAYABLE_AUDIO_TRACK(last_track, last_track);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u)
        {
            upper_bound = (s32)((u32)SELECT_PLAYABLE_AUDIO_WORKING_FRAME() + 75ul);
            if(start_frame_position > upper_bound || end_frame_position > upper_bound)
            {
                SET_PLAY_AUDIO_FRAME_RANGE_SENSE(5u, 0x2100u);
                return 0u;
            }
        }
    }

    if(PROBE_PLAYABLE_AUDIO_FRAME(start_frame_position) != 0u)
        return 1u;
    probe_position = (s32)((u32)start_frame_position - 75ul);
    if(probe_position < 0)
        probe_position = 0;
    if(PROBE_PLAYABLE_AUDIO_FRAME(probe_position) != 0u)
        return 1u;
    probe_position = (s32)((u32)start_frame_position + 75ul);
    if(PROBE_PLAYABLE_AUDIO_FRAME(probe_position) != 0u)
        return 1u;

    SET_PLAY_AUDIO_FRAME_RANGE_SENSE(5u, 0x6400u);
    return 0u;
}
