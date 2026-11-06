#include "play_audio_track_index.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/play_audio_track_index_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static s32 ReadPlayAudioTrackIndexFrame(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static void WritePlayAudioTrackIndexFrame(fw_addr_t address, s32 value)
{
    u32 bits = (u32)value;

    FirmwareWrite8(address, (u8)(bits >> 24));
    FirmwareWrite8(address + 1u, (u8)(bits >> 16));
    FirmwareWrite8(address + 2u, (u8)(bits >> 8));
    FirmwareWrite8(address + 3u, (u8)bits);
}

static fw_addr_t PlayAudioTrackTupleAddress(u16 base, u8 track_number)
{
    return (fw_addr_t)(u16)(base + (u16)track_number * 6u);
}

/* RAM:fd4284 */
void HandlePlayAudioTrackIndexCommand(void)
{
    u8 start_track;
    u8 start_index;
    u8 end_track;
    u8 end_index;
    u8 last_track;
    s32 target_frame;
    s32 end_frame;

    if(VALIDATE_PLAY_AUDIO_TRACK_INDEX_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D65)) == 0u)
    {
        REJECT_PLAY_AUDIO_TRACK_INDEX_COMMAND();
        return;
    }

    start_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E);
    start_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F);
    end_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B11);
    end_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B12);

    if(start_track == 0xffu && start_index == 0xffu)
    {
        start_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1233);
        start_index = 1u;
        if(VALIDATE_PLAY_AUDIO_TRACK_INDEX_REQUEST(start_track, end_track, start_index, end_index) == 0u)
            return;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
            {
                last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
                if(end_track < last_track)
                    last_track = end_track;
                end_frame = CONVERT_PLAY_AUDIO_TRACK_INDEX_MSF(PlayAudioTrackTupleAddress(0xf2bfu, last_track));
            }
            else
            {
                last_track = GET_PLAY_AUDIO_TRACK_INDEX_LAST_TRACK();
                if(end_track <= last_track)
                {
                    LOAD_PLAY_AUDIO_TRACK_INDEX_TRACK(end_track);
                    end_frame = ReadPlayAudioTrackIndexFrame(FW_NEAR_UNKNOWN_DD3D);
                }
                else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
                {
                    LOAD_PLAY_AUDIO_TRACK_INDEX_TRACK(last_track);
                    end_frame = ReadPlayAudioTrackIndexFrame(FW_NEAR_UNKNOWN_DD41);
                }
                else
                {
                    end_frame = CONVERT_PLAY_AUDIO_TRACK_INDEX_MSF(PlayAudioTrackTupleAddress(0xf2bfu, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0)));
                }
            }
            end_frame = (s32)((u32)end_frame - 1ul);
            STORE_PLAY_AUDIO_TRACK_INDEX_RESUME_END(end_frame);
            SET_NO_SENSE_FOR_PLAY_AUDIO_TRACK_INDEX();
            return;
        }
    }
    else if(VALIDATE_PLAY_AUDIO_TRACK_INDEX_REQUEST(start_track, end_track, start_index, end_index) == 0u)
    {
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8D, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        target_frame = CONVERT_PLAY_AUDIO_TRACK_INDEX_MSF(PlayAudioTrackTupleAddress(0xf2b9u, start_track));
        last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
        if(end_track < last_track)
            last_track = end_track;
        end_frame = CONVERT_PLAY_AUDIO_TRACK_INDEX_MSF(PlayAudioTrackTupleAddress(0xf2bfu, last_track));
    }
    else
    {
        last_track = GET_PLAY_AUDIO_TRACK_INDEX_LAST_TRACK();
        LOAD_PLAY_AUDIO_TRACK_INDEX_TRACK(start_track);
        target_frame = ReadPlayAudioTrackIndexFrame(FW_NEAR_UNKNOWN_DD3D);
        if(end_track < last_track)
            last_track = end_track;
        LOAD_PLAY_AUDIO_TRACK_INDEX_TRACK(last_track);
        end_frame = ReadPlayAudioTrackIndexFrame(FW_NEAR_UNKNOWN_DD41);
    }

    end_frame = (s32)((u32)end_frame - 1ul);
    WritePlayAudioTrackIndexFrame(FW_NEAR_UNKNOWN_0C8E, target_frame);
    WritePlayAudioTrackIndexFrame(FW_NEAR_UNKNOWN_0C92, end_frame);
    WritePlayAudioTrackIndexFrame(FW_NEAR_UNKNOWN_0F36, (s32)((u32)end_frame - (u32)target_frame));
    CLEAR_PLAY_AUDIO_TRACK_INDEX_BIT0018();
    START_PLAY_AUDIO_TRACK_INDEX_READ(target_frame);
}
