#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR7ForRepairTrack(void);
extern u8 ObserveValidateRepairTrackCdb(fw_addr_t mask_descriptor);
extern u8 ObserveValidateRepairTrackWriteParameters(void);
extern u8 ObserveValidateRepairTrackMediaEligibility(void);
extern void ObserveRejectRepairTrackCommand(void);
extern void ObserveClearRepairTrackWorkingInformation(void);
extern u8 ObserveLoadRepairTrackInformation(u8 track_number, u8 inherited_r7);
extern void ObserveSetRepairTrackNoSense(void);
extern void ObserveSetRepairTrackSense(u8 sense_key, u16 sense_code);
extern void ObserveFinalizeRepairTrack(void);
#define CAPTURE_INHERITED_R7() ObserveCaptureInheritedR7ForRepairTrack()
#define VALIDATE_CDB(mask) ObserveValidateRepairTrackCdb(mask)
#define VALIDATE_WRITE_PARAMETERS() ObserveValidateRepairTrackWriteParameters()
#define VALIDATE_MEDIA() ObserveValidateRepairTrackMediaEligibility()
#define REJECT_COMMAND() ObserveRejectRepairTrackCommand()
#define CLEAR_WORKING_TRACK() ObserveClearRepairTrackWorkingInformation()
#define LOAD_TRACK(track, inherited_r7) ObserveLoadRepairTrackInformation((track), (inherited_r7))
#define SET_NO_SENSE() ObserveSetRepairTrackNoSense()
#define SET_SENSE(key, code) ObserveSetRepairTrackSense((key), (code))
#define FINALIZE_REPAIR() ObserveFinalizeRepairTrack()
#else
extern u8 CaptureInheritedR7ForRepairTrack(void);
extern u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_descriptor);
extern u8 ValidateCurrentWriteParameterCombination(void);
extern u8 ValidateRecordableMediaCommandEligibility(void);
extern void HandleUnsupportedScsiCommand(void);
extern void ClearWorkingTrackInformation(void);
extern u8 LoadTrackInformationByNumberFromInheritedB(u8 track_number, u8 inherited_r7);
extern void SetNoSenseData(void);
extern void SetSenseData(u8 sense_key, u16 sense_code);
extern void FinalizeWorkingTrackAfterRepairOrCacheSync(void);
#define CAPTURE_INHERITED_R7() CaptureInheritedR7ForRepairTrack()
#define VALIDATE_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define VALIDATE_WRITE_PARAMETERS() ValidateCurrentWriteParameterCombination()
#define VALIDATE_MEDIA() ValidateRecordableMediaCommandEligibility()
#define REJECT_COMMAND() HandleUnsupportedScsiCommand()
#define CLEAR_WORKING_TRACK() ClearWorkingTrackInformation()
#define LOAD_TRACK(track, inherited_r7) LoadTrackInformationByNumberFromInheritedB((track), (inherited_r7))
#define SET_NO_SENSE() SetNoSenseData()
#define SET_SENSE(key, code) SetSenseData((key), (code))
#define FINALIZE_REPAIR() FinalizeWorkingTrackAfterRepairOrCacheSync()
#endif
#endif
