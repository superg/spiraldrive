#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidatePlayTrackRelativeCdb(fw_addr_t mask_address);
extern void ObserveRejectPlayTrackRelativeCommand(void);
extern u8 ObserveValidatePlayTrackRelativeTrack(u8 start_track, u8 end_track, u8 start_index, u8 end_index);
extern s32 ObserveConvertPlayTrackRelativeMsf(fw_addr_t tuple_address);
extern void ObserveLoadPlayTrackRelativeTrack(u8 track_number);
extern u8 ObserveGetPlayTrackRelativeLastTrack(void);
extern s32 ObserveSelectPlayTrackRelativeBoundary(void);
extern void ObserveClearPlayTrackRelativeBit0018(void);
extern void ObserveStartPlayTrackRelativeRead(s32 frame_position);
extern u8 ObserveDispatchPlayTrackRelativeAcquisition(void);
#define VALIDATE_PLAY_TRACK_RELATIVE_CDB(address) ObserveValidatePlayTrackRelativeCdb(address)
#define REJECT_PLAY_TRACK_RELATIVE_COMMAND() ObserveRejectPlayTrackRelativeCommand()
#define VALIDATE_PLAY_TRACK_RELATIVE_TRACK(start_track, end_track, start_index, end_index) ObserveValidatePlayTrackRelativeTrack((start_track), (end_track), (start_index), (end_index))
#define CONVERT_PLAY_TRACK_RELATIVE_MSF(address) ObserveConvertPlayTrackRelativeMsf(address)
#define LOAD_PLAY_TRACK_RELATIVE_TRACK(track) ObserveLoadPlayTrackRelativeTrack(track)
#define GET_PLAY_TRACK_RELATIVE_LAST_TRACK() ObserveGetPlayTrackRelativeLastTrack()
#define SELECT_PLAY_TRACK_RELATIVE_BOUNDARY() ObserveSelectPlayTrackRelativeBoundary()
#define CLEAR_PLAY_TRACK_RELATIVE_BIT0018() ObserveClearPlayTrackRelativeBit0018()
#define START_PLAY_TRACK_RELATIVE_READ(frame) ObserveStartPlayTrackRelativeRead(frame)
#define DISPATCH_PLAY_TRACK_RELATIVE_ACQUISITION() ObserveDispatchPlayTrackRelativeAcquisition()
#define HandlePlayTrackRelative12CommandBody HandlePlayTrackRelative12CommandForTest
#else
extern u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_address);
extern void HandleUnsupportedScsiCommand(void);
extern u8 ValidatePlayAudioTrackIndexRange(u8 start_track, u8 end_track, u8 start_index, u8 end_index);
extern s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t tuple_address);
extern void LoadTrackInformationByNumber(u8 track_number);
extern u8 GetEffectiveLastTrackNumber(void);
extern s32 SelectWorkingTrackFramePosition(void);
extern void ClearBit0018Atomic(void);
extern void StartFormat2RawSectorReadAtFramePosition(s32 frame_position);
extern u8 DispatchAndWaitForReadAcquisitionResult(void);
#define VALIDATE_PLAY_TRACK_RELATIVE_CDB(address) ValidateScsiCdbFieldMasks(address)
#define REJECT_PLAY_TRACK_RELATIVE_COMMAND() HandleUnsupportedScsiCommand()
#define VALIDATE_PLAY_TRACK_RELATIVE_TRACK(start_track, end_track, start_index, end_index) ValidatePlayAudioTrackIndexRange((start_track), (end_track), (start_index), (end_index))
#define CONVERT_PLAY_TRACK_RELATIVE_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define LOAD_PLAY_TRACK_RELATIVE_TRACK(track) LoadTrackInformationByNumber(track)
#define GET_PLAY_TRACK_RELATIVE_LAST_TRACK() GetEffectiveLastTrackNumber()
#define SELECT_PLAY_TRACK_RELATIVE_BOUNDARY() SelectWorkingTrackFramePosition()
#define CLEAR_PLAY_TRACK_RELATIVE_BIT0018() ClearBit0018Atomic()
#define START_PLAY_TRACK_RELATIVE_READ(frame) StartFormat2RawSectorReadAtFramePosition(frame)
#define DISPATCH_PLAY_TRACK_RELATIVE_ACQUISITION() DispatchAndWaitForReadAcquisitionResult()
#define HandlePlayTrackRelative12CommandBody HandlePlayTrackRelative12Command
#endif
#endif
