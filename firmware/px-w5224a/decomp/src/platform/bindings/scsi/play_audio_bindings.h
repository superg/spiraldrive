#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidatePlayAudioMsfTuple(fw_addr_t tuple_address);
extern s32 ObserveConvertPlayAudioMsfTuple(fw_addr_t tuple_address);
extern void ObserveRejectPlayAudioMsfRange(void);
extern u8 ObserveValidatePlayAudioFrameRange(s32 start_frame_position, s32 end_frame_position);
extern void ObserveResumePlaybackRangeAtExclusiveEnd(s32 end_frame_position);
extern void ObserveSetNoSenseForPlayAudioFrameRange(void);
extern void ObserveStartPlayAudioFrameRange(s32 start_frame_position, s32 end_frame_position, u8 playback_mode);

#define DispatchValidatePlayAudioFrameRange(start, end) ObserveValidatePlayAudioFrameRange((start), (end))
#define DispatchValidatePlayAudioMsfTuple(address) ObserveValidatePlayAudioMsfTuple(address)
#define DispatchConvertPlayAudioMsfTuple(address) ObserveConvertPlayAudioMsfTuple(address)
#define DispatchRejectPlayAudioMsfRange() ObserveRejectPlayAudioMsfRange()
#define DispatchResumePlaybackRangeAtExclusiveEnd(end) ObserveResumePlaybackRangeAtExclusiveEnd(end)
#define DispatchSetNoSenseForPlayAudioFrameRange() ObserveSetNoSenseForPlayAudioFrameRange()
#define DispatchStartPlayAudioFrameRange(start, end, mode) ObserveStartPlayAudioFrameRange((start), (end), (mode))
#define ValidateAndStartPlayAudioFrameRangeBody ValidateAndStartPlayAudioFrameRangeForTest
#define ValidateAndStartPlayAudioMsfRangeBody ValidateAndStartPlayAudioMsfRangeFromCdbTuplesForTest
#else
extern u8 IsBinaryMsfTupleValid(fw_addr_t tuple_address);
extern s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address);
extern void HandleUnsupportedScsiCommand(void);
extern u8 PLEXTOR_NEAR_FUNCTION ValidatePlayAudioFrameRange(s32 start_frame_position, s32 end_frame_position);
extern void SetNoSenseData(void);
extern void StartPlayAudioFrameRange(s32 start_frame_position, s32 end_frame_position, u8 playback_mode);

#define DispatchValidatePlayAudioFrameRange(start, end) ValidatePlayAudioFrameRange((start), (end))
#define DispatchValidatePlayAudioMsfTuple(address) IsBinaryMsfTupleValid(address)
#define DispatchConvertPlayAudioMsfTuple(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define DispatchRejectPlayAudioMsfRange() HandleUnsupportedScsiCommand()
#define DispatchResumePlaybackRangeAtExclusiveEnd(end) StorePlaybackRangeStartFramePosition(end)
#define DispatchSetNoSenseForPlayAudioFrameRange() SetNoSenseData()
#define DispatchStartPlayAudioFrameRange(start, end, mode) StartPlayAudioFrameRange((start), (end), (mode))
#define ValidateAndStartPlayAudioFrameRangeBody ValidateAndStartPlayAudioFrameRange
#define ValidateAndStartPlayAudioMsfRangeBody ValidateAndStartPlayAudioMsfRangeFromCdbTuples
#endif
#endif
