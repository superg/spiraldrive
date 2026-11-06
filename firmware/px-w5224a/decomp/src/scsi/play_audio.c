#include "play_audio.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/play_audio_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ReadBigEndianS32ForPlayAudio(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

/* RAM:fdcb43 */
void StorePlaybackRangeStartFramePosition(s32 frame_position)
{
    u32 value = (u32)frame_position;

    FirmwareWriteNearU32(FW_NEAR_UNKNOWN_0FC6, value);
}

/* RAM:fd4162 */
void ValidateAndStartPlayAudioMsfRangeBody(fw_addr_t end_tuple_address, fw_addr_t start_tuple_address, u8 playback_mode)
{
    s32 start_frame_position;
    s32 end_frame_position;
    u8 resume_current_position = (u8)((FirmwareRead8(start_tuple_address) & FirmwareRead8(start_tuple_address + 1u) & FirmwareRead8(start_tuple_address + 2u)) == 0xffu);

    if(resume_current_position != 0u)
    {
        if(DispatchValidatePlayAudioMsfTuple(end_tuple_address) == 0u)
        {
            DispatchRejectPlayAudioMsfRange();
            return;
        }
        start_frame_position = ReadBigEndianS32ForPlayAudio(FW_NEAR_UNKNOWN_1235);
        end_frame_position = DispatchConvertPlayAudioMsfTuple(end_tuple_address);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 1u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) >> 4) & 1u) == 0u)
        {
            if(playback_mode != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 8u));
            DispatchResumePlaybackRangeAtExclusiveEnd(end_frame_position);
            DispatchSetNoSenseForPlayAudioFrameRange();
            return;
        }
    }
    else
    {
        if(DispatchValidatePlayAudioMsfTuple(start_tuple_address) == 0u)
        {
            DispatchRejectPlayAudioMsfRange();
            return;
        }
        if(DispatchValidatePlayAudioMsfTuple(end_tuple_address) == 0u)
        {
            DispatchRejectPlayAudioMsfRange();
            return;
        }
        start_frame_position = DispatchConvertPlayAudioMsfTuple(start_tuple_address);
        end_frame_position = DispatchConvertPlayAudioMsfTuple(end_tuple_address);
    }

    if(DispatchValidatePlayAudioFrameRange(start_frame_position, end_frame_position) != 0u)
        DispatchStartPlayAudioFrameRange(start_frame_position, end_frame_position, playback_mode);
}

/* RAM:fd458e */
void ValidateAndStartPlayAudioFrameRangeBody(s32 start_frame_position, s32 frame_count, u8 playback_mode)
{
    s32 end_frame_position;
    u8 resume_current_position = (u8)(start_frame_position == -1l);

    if(resume_current_position != 0u)
        start_frame_position = ReadBigEndianS32ForPlayAudio(FW_NEAR_UNKNOWN_1235);
    end_frame_position = (s32)((u32)start_frame_position + (u32)frame_count);

    if(DispatchValidatePlayAudioFrameRange(start_frame_position, end_frame_position) == 0u)
        return;

    if(resume_current_position != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 1u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) >> 4) & 1u) == 0u)
    {
        if(playback_mode != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 8u));
        DispatchResumePlaybackRangeAtExclusiveEnd(end_frame_position);
        DispatchSetNoSenseForPlayAudioFrameRange();
        return;
    }

    DispatchStartPlayAudioFrameRange(start_frame_position, end_frame_position, playback_mode);
}
