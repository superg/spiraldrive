#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidatePlayAudioTrackIndexCdb(fw_addr_t mask_address);
extern void ObserveRejectPlayAudioTrackIndexCommand(void);
extern u8 ObserveValidatePlayAudioTrackIndexRequest(u8 start_track, u8 end_track, u8 start_index, u8 end_index);
extern s32 ObserveConvertPlayAudioTrackIndexMsf(fw_addr_t tuple_address);
extern u8 ObserveGetPlayAudioTrackIndexLastTrack(void);
extern void ObserveLoadPlayAudioTrackIndexTrack(u8 track_number);
extern void ObserveStorePlayAudioTrackIndexResumeEnd(s32 frame_position);
extern void ObserveSetNoSenseForPlayAudioTrackIndex(void);
extern void ObserveClearPlayAudioTrackIndexBit0018(void);
extern void ObserveStartPlayAudioTrackIndexRead(s32 frame_position);
#define VALIDATE_PLAY_AUDIO_TRACK_INDEX_CDB(address) ObserveValidatePlayAudioTrackIndexCdb(address)
#define REJECT_PLAY_AUDIO_TRACK_INDEX_COMMAND() ObserveRejectPlayAudioTrackIndexCommand()
#define VALIDATE_PLAY_AUDIO_TRACK_INDEX_REQUEST(start_track, end_track, start_index, end_index) ObserveValidatePlayAudioTrackIndexRequest((start_track), (end_track), (start_index), (end_index))
#define CONVERT_PLAY_AUDIO_TRACK_INDEX_MSF(address) ObserveConvertPlayAudioTrackIndexMsf(address)
#define GET_PLAY_AUDIO_TRACK_INDEX_LAST_TRACK() ObserveGetPlayAudioTrackIndexLastTrack()
#define LOAD_PLAY_AUDIO_TRACK_INDEX_TRACK(track) ObserveLoadPlayAudioTrackIndexTrack(track)
#define STORE_PLAY_AUDIO_TRACK_INDEX_RESUME_END(frame) ObserveStorePlayAudioTrackIndexResumeEnd(frame)
#define SET_NO_SENSE_FOR_PLAY_AUDIO_TRACK_INDEX() ObserveSetNoSenseForPlayAudioTrackIndex()
#define CLEAR_PLAY_AUDIO_TRACK_INDEX_BIT0018() ObserveClearPlayAudioTrackIndexBit0018()
#define START_PLAY_AUDIO_TRACK_INDEX_READ(frame) ObserveStartPlayAudioTrackIndexRead(frame)
#else
extern u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_address);
extern void HandleUnsupportedScsiCommand(void);
extern u8 ValidatePlayAudioTrackIndexRange(u8 start_track, u8 end_track, u8 start_index, u8 end_index);
extern s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address);
extern u8 GetEffectiveLastTrackNumber(void);
extern void LoadTrackInformationByNumber(u8 track_number);
extern void StorePlaybackRangeStartFramePosition(s32 frame_position);
extern void SetNoSenseData(void);
extern void ClearBit0018Atomic(void);
extern void StartFormat2RawSectorReadAtFramePosition(s32 frame_position);
#define VALIDATE_PLAY_AUDIO_TRACK_INDEX_CDB(address) ValidateScsiCdbFieldMasks(address)
#define REJECT_PLAY_AUDIO_TRACK_INDEX_COMMAND() HandleUnsupportedScsiCommand()
#define VALIDATE_PLAY_AUDIO_TRACK_INDEX_REQUEST(start_track, end_track, start_index, end_index) ValidatePlayAudioTrackIndexRange((start_track), (end_track), (start_index), (end_index))
#define CONVERT_PLAY_AUDIO_TRACK_INDEX_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define GET_PLAY_AUDIO_TRACK_INDEX_LAST_TRACK() GetEffectiveLastTrackNumber()
#define LOAD_PLAY_AUDIO_TRACK_INDEX_TRACK(track) LoadTrackInformationByNumber(track)
#define STORE_PLAY_AUDIO_TRACK_INDEX_RESUME_END(frame) StorePlaybackRangeStartFramePosition(frame)
#define SET_NO_SENSE_FOR_PLAY_AUDIO_TRACK_INDEX() SetNoSenseData()
#define CLEAR_PLAY_AUDIO_TRACK_INDEX_BIT0018() ClearBit0018Atomic()
#define START_PLAY_AUDIO_TRACK_INDEX_READ(frame) StartFormat2RawSectorReadAtFramePosition(frame)
#endif
#endif
