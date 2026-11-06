#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveConvertPlayableAudioTrackMsf(fw_addr_t tuple_address);
extern u8 ObserveGetPlayableAudioFirstTrack(void);
extern u8 ObserveGetPlayableAudioLastTrack(void);
extern void ObserveLoadPlayableAudioTrack(u8 track_number, u8 inherited_b);
extern void ObserveRejectPlayAudioFrameRange(void);
extern void ObserveSetPlayAudioFrameRangeSense(u8 sense_key, u16 asc_ascq);
extern s32 ObserveSelectPlayableAudioWorkingTrackFrame(void);
extern u8 ObservePlayableAudioFrameProbe(s32 frame_position);
#define CONVERT_PLAYABLE_AUDIO_MSF(address) ObserveConvertPlayableAudioTrackMsf(address)
#define GET_PLAYABLE_AUDIO_FIRST_TRACK() ObserveGetPlayableAudioFirstTrack()
#define GET_PLAYABLE_AUDIO_LAST_TRACK() ObserveGetPlayableAudioLastTrack()
#define LOAD_PLAYABLE_AUDIO_TRACK(track, inherited_b) ObserveLoadPlayableAudioTrack((track), (inherited_b))
#define REJECT_PLAY_AUDIO_FRAME_RANGE() ObserveRejectPlayAudioFrameRange()
#define SET_PLAY_AUDIO_FRAME_RANGE_SENSE(key, code) ObserveSetPlayAudioFrameRangeSense((key), (code))
#define SELECT_PLAYABLE_AUDIO_WORKING_FRAME() ObserveSelectPlayableAudioWorkingTrackFrame()
#define PROBE_PLAYABLE_AUDIO_FRAME(frame) ObservePlayableAudioFrameProbe(frame)
#else
extern s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address);
extern u8 GetEffectiveFirstTrackNumber(void);
extern u8 GetEffectiveLastTrackNumber(void);
extern u8 LoadTrackInformationByNumberFromInheritedB(u8 track_number, u8 inherited_b);
extern void HandleUnsupportedScsiCommand(void);
extern void SetSenseData(u8 sense_key, u16 asc_ascq);
extern s32 SelectWorkingTrackFramePosition(void);
#define CONVERT_PLAYABLE_AUDIO_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define GET_PLAYABLE_AUDIO_FIRST_TRACK() GetEffectiveFirstTrackNumber()
#define GET_PLAYABLE_AUDIO_LAST_TRACK() GetEffectiveLastTrackNumber()
#define LOAD_PLAYABLE_AUDIO_TRACK(track, inherited_b) ((void)LoadTrackInformationByNumberFromInheritedB((track), (inherited_b)))
#define REJECT_PLAY_AUDIO_FRAME_RANGE() HandleUnsupportedScsiCommand()
#define SET_PLAY_AUDIO_FRAME_RANGE_SENSE(key, code) SetSenseData((key), (code))
#define SELECT_PLAYABLE_AUDIO_WORKING_FRAME() SelectWorkingTrackFramePosition()
#define PROBE_PLAYABLE_AUDIO_FRAME(frame) IsFramePositionOnPlayableAudioTrack(frame)
#endif
#endif
