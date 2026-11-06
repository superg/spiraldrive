#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCloseTrackCaptureInheritedB(void);
extern u8 ObserveCloseTrackValidateCdb(fw_addr_t mask);
extern void ObserveCloseTrackNormalizeSpeed(void);
extern u8 ObserveCloseTrackFinalizeActiveWrite(void);
extern void ObserveCloseTrackFinalizeExistingState(void);
extern void ObserveCloseTrackCompleteNoSense(void);
extern void ObserveCloseTrackResetRawRead(void);
extern void ObserveCloseTrackInitializeCoroutine2(u32 entry_point);
extern void ObserveCloseTrackYield(void);
extern u8 ObserveCloseTrackPrepareControllerState(void);
extern void ObserveCloseTrackUpdateFailure(void);
extern void ObserveCloseTrackUnsupported(void);
extern u8 ObserveCloseTrackValidateMedia(void);
extern u8 ObserveCloseTrackValidateSessionState(void);
extern void ObserveCloseTrackUpdateTailBounds(void);
extern void ObserveCloseTrackClearWorking(void);
extern u8 ObserveCloseTrackLoadTrack(u8 track_number, u8 inherited_b);
extern void ObserveCloseTrackNoSense(void);
extern void ObserveCloseTrackSequenceError(void);
extern u8 ObserveCloseTrackValidateWriteState(void);
extern u8 ObserveCloseTrackValidatePma(void);
extern void ObserveCloseTrackCompleteSession(void);
extern void ObserveCloseTrackExecuteOperation(void);
#define CLOSE_CAPTURE_B() ObserveCloseTrackCaptureInheritedB()
#define CLOSE_VALIDATE_CDB(mask) ObserveCloseTrackValidateCdb(mask)
#define CLOSE_NORMALIZE_SPEED() ObserveCloseTrackNormalizeSpeed()
#define CLOSE_FINALIZE_ACTIVE() ObserveCloseTrackFinalizeActiveWrite()
#define CLOSE_FINALIZE_EXISTING() ObserveCloseTrackFinalizeExistingState()
#define CLOSE_COMPLETE_NO_SENSE() ObserveCloseTrackCompleteNoSense()
#define CLOSE_RESET_RAW_READ() ObserveCloseTrackResetRawRead()
#define CLOSE_INIT_COROUTINE2(entry) ObserveCloseTrackInitializeCoroutine2(entry)
#define CLOSE_YIELD() ObserveCloseTrackYield()
#define CLOSE_PREPARE_CONTROLLER() ObserveCloseTrackPrepareControllerState()
#define CLOSE_UPDATE_FAILURE() ObserveCloseTrackUpdateFailure()
#define CLOSE_UNSUPPORTED() ObserveCloseTrackUnsupported()
#define CLOSE_VALIDATE_MEDIA() ObserveCloseTrackValidateMedia()
#define CLOSE_VALIDATE_SESSION() ObserveCloseTrackValidateSessionState()
#define CLOSE_UPDATE_TAIL_BOUNDS() ObserveCloseTrackUpdateTailBounds()
#define CLOSE_CLEAR_WORKING() ObserveCloseTrackClearWorking()
#define CLOSE_LOAD_TRACK(number, inherited_b) ObserveCloseTrackLoadTrack((number), (inherited_b))
#define CLOSE_NO_SENSE() ObserveCloseTrackNoSense()
#define CLOSE_SEQUENCE_ERROR() ObserveCloseTrackSequenceError()
#define CLOSE_VALIDATE_WRITE_STATE() ObserveCloseTrackValidateWriteState()
#define CLOSE_VALIDATE_PMA() ObserveCloseTrackValidatePma()
#define CLOSE_COMPLETE_SESSION() ObserveCloseTrackCompleteSession()
#define CLOSE_EXECUTE_OPERATION() ObserveCloseTrackExecuteOperation()
#else
extern u8 CaptureInheritedBForCloseTrackSession(void);
extern void YieldCoroutine0(void);
#define CLOSE_CAPTURE_B() CaptureInheritedBForCloseTrackSession()
#define CLOSE_VALIDATE_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define CLOSE_NORMALIZE_SPEED() NormalizeWriteSpeedForCurrentMedia()
#define CLOSE_FINALIZE_ACTIVE() FinalizeActiveWritePipelineAndTrackState()
#define CLOSE_FINALIZE_EXISTING() FinalizeCloseTrackSessionState()
#define CLOSE_COMPLETE_NO_SENSE() CompleteScsiCommandWithNoSense()
#define CLOSE_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define CLOSE_INIT_COROUTINE2(entry) InitializeCoroutine2(entry)
#define CLOSE_YIELD() YieldCoroutine0()
#define CLOSE_PREPARE_CONTROLLER() PrepareWritePipelineControllerRecordState()
#define CLOSE_UPDATE_FAILURE() UpdateWorkingTrackAfterWriteFailure()
#define CLOSE_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define CLOSE_VALIDATE_MEDIA() ValidateRecordableMediaCommandEligibility()
#define CLOSE_VALIDATE_SESSION() ValidateCloseSessionTrackTableState()
#define CLOSE_UPDATE_TAIL_BOUNDS() UpdateSelectedTrackTableTailFrameBounds()
#define CLOSE_CLEAR_WORKING() ClearWorkingTrackInformation()
#define CLOSE_LOAD_TRACK(number, inherited_b) LoadTrackInformationByNumberFromInheritedB((number), (inherited_b))
#define CLOSE_NO_SENSE() SetNoSenseData()
#define CLOSE_SEQUENCE_ERROR() SetCommandSequenceErrorSense()
#define CLOSE_VALIDATE_WRITE_STATE() ValidateWriteOperationTrackState()
#define CLOSE_VALIDATE_PMA() ValidatePmaStartBeforeAtipLeadInForWrite()
#define CLOSE_COMPLETE_SESSION() CompleteCloseTrackSessionAfterOpc()
#define CLOSE_EXECUTE_OPERATION() ExecuteCloseTrackOrSynchronizeCacheOperation()
#endif
#endif
