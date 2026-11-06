#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWriteRangeNormalizeSpeed(void);
extern u8 ObserveWriteRangeValidateParameters(void);
extern u8 ObserveWriteRangeValidateMedia(void);
extern void ObserveWriteRangeSequenceError(void);
extern void ObserveWriteRangeCopyBytes16(u16 destination, u16 source, u8 count);
extern void ObserveWriteRangeSetControllerEnable(u8 enabled);
extern void ObserveWriteRangeClearWorkingTrack(void);
extern u8 ObserveWriteRangeValidateTrackTable(s32 frame_position);
extern u8 ObserveWriteRangeFindWritableTrack(s32 frame_position);
extern u8 ObserveWriteRangeLoadSnapshot(u8 track_number);
extern u8 ObserveWriteRangeInitializeHighTrack(void);
extern void ObserveWriteRangeUnsupported(void);
extern void ObserveWriteRangeSelectAlternateCursor(void);
extern void ObserveWriteRangeSelectCursor(void);
extern u8 ObserveWriteRangeValidateWriteType(void);
extern u8 ObserveWriteRangeValidateTrackState(void);
extern u8 ObserveWriteRangeValidatePma(void);
extern u32 ObserveWriteRangeMapAddress(u32 logical_address);
extern void ObserveWriteRangeResetRawRead(void);
extern void ObserveWriteRangeInitializeCoroutine2(u32 entry_point);
extern void ObserveWriteRangeYield(void);
extern void ObserveWriteRangeSetSense(u8 sense_key, u16 asc_ascq);
extern u8 ObserveWriteRangeEnsureOpc(void);
extern u8 ObserveWriteRangeBuildRawQ(void);
extern u8 ObserveWriteRangeAcquireZeroRange(u32 caller_target);
extern void ObserveWriteRangeNoSense(void);
extern u8 ObserveWriteRangeReconcileStart(s32 start_lba);
extern s32 ObserveWriteRangeSelectPacketProgress(void);
extern u8 ObserveWriteRangeValidateExtent(u8 continuation, u32 count);
extern u8 ObserveWriteRangeInitializePipeline(u8 continuation);
extern u8 ObserveWriteRangeSelectLowestSpeed(void);
extern void ObserveWriteRangeAdvanceType1(u32 count, u8 continuation);
extern void ObserveWriteRangeAdvanceAfterTransfer(s32 start_lba, u32 count, u8 continuation);
extern void ObserveWriteRangeUpdateFailure(void);
#define WRITE_NORMALIZE_SPEED() ObserveWriteRangeNormalizeSpeed()
#define WRITE_VALIDATE_PARAMETERS() ObserveWriteRangeValidateParameters()
#define WRITE_VALIDATE_MEDIA() ObserveWriteRangeValidateMedia()
#define WRITE_SEQUENCE_ERROR() ObserveWriteRangeSequenceError()
#define WRITE_COPY16(destination, source, count) ObserveWriteRangeCopyBytes16((destination), (source), (count))
#define WRITE_SET_CONTROLLER(value) ObserveWriteRangeSetControllerEnable(value)
#define WRITE_CLEAR_TRACK() ObserveWriteRangeClearWorkingTrack()
#define WRITE_VALIDATE_TRACK(position) ObserveWriteRangeValidateTrackTable(position)
#define WRITE_FIND_TRACK(position) ObserveWriteRangeFindWritableTrack(position)
#define WRITE_LOAD_SNAPSHOT(track) ObserveWriteRangeLoadSnapshot(track)
#define WRITE_INITIALIZE_HIGH_TRACK() ObserveWriteRangeInitializeHighTrack()
#define WRITE_UNSUPPORTED() ObserveWriteRangeUnsupported()
#define WRITE_SELECT_ALTERNATE_CURSOR() ObserveWriteRangeSelectAlternateCursor()
#define WRITE_SELECT_CURSOR() ObserveWriteRangeSelectCursor()
#define WRITE_VALIDATE_TYPE() ObserveWriteRangeValidateWriteType()
#define WRITE_VALIDATE_TRACK_STATE() ObserveWriteRangeValidateTrackState()
#define WRITE_VALIDATE_PMA() ObserveWriteRangeValidatePma()
#define WRITE_MAP_ADDRESS(value) ObserveWriteRangeMapAddress(value)
#define WRITE_RESET_RAW_READ() ObserveWriteRangeResetRawRead()
#define WRITE_INITIALIZE_COROUTINE(value) ObserveWriteRangeInitializeCoroutine2(value)
#define WRITE_YIELD() ObserveWriteRangeYield()
#define WRITE_SET_SENSE(key, value) ObserveWriteRangeSetSense((key), (value))
#define WRITE_ENSURE_OPC() ObserveWriteRangeEnsureOpc()
#define WRITE_BUILD_RAW_Q() ObserveWriteRangeBuildRawQ()
#define WRITE_ACQUIRE_ZERO_RANGE(value) ObserveWriteRangeAcquireZeroRange(value)
#define WRITE_NO_SENSE() ObserveWriteRangeNoSense()
#define WRITE_RECONCILE_START(value) ObserveWriteRangeReconcileStart(value)
#define WRITE_SELECT_PACKET_PROGRESS() ObserveWriteRangeSelectPacketProgress()
#define WRITE_VALIDATE_EXTENT(flag, count) ObserveWriteRangeValidateExtent((flag), (count))
#define WRITE_INITIALIZE_PIPELINE(flag) ObserveWriteRangeInitializePipeline(flag)
#define WRITE_SELECT_LOWEST_SPEED() ObserveWriteRangeSelectLowestSpeed()
#define WRITE_ADVANCE_TYPE1(count, flag) ObserveWriteRangeAdvanceType1((count), (flag))
#define WRITE_ADVANCE_AFTER(start, count, flag) ObserveWriteRangeAdvanceAfterTransfer((start), (count), (flag))
#define WRITE_UPDATE_FAILURE() ObserveWriteRangeUpdateFailure()
#else
extern u8 InitializeAndBuildRawReadQRecordQueue(void);
extern void YieldCoroutine0(void);
#define WRITE_NORMALIZE_SPEED() NormalizeWriteSpeedForCurrentMedia()
#define WRITE_VALIDATE_PARAMETERS() ValidateCurrentWriteParameterCombination()
#define WRITE_VALIDATE_MEDIA() ValidateRecordableMediaCommandEligibility()
#define WRITE_SEQUENCE_ERROR() SetCommandSequenceErrorSense()
#define WRITE_COPY16(destination, source, count) CopyBytes16((destination), (source), (count))
#define WRITE_SET_CONTROLLER(value) SetWritePipelineControllerEnable(value)
#define WRITE_CLEAR_TRACK() ClearWorkingTrackInformation()
#define WRITE_VALIDATE_TRACK(position) ValidateWriteFramePositionAgainstTrackTable(position)
#define WRITE_FIND_TRACK(position) FindWritableTrackForCandidateFramePosition(position)
#define WRITE_LOAD_SNAPSHOT(track) LoadTrackInformationAgainstSnapshotFromInheritedB((track), 0u)
#define WRITE_INITIALIZE_HIGH_TRACK() InitializeWorkingTrackFromTrackTableHighEntryFromRegisterInputs(0u, 0u)
#define WRITE_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define WRITE_SELECT_ALTERNATE_CURSOR() SelectAlternateWorkingTrackWriteCursorFramePosition()
#define WRITE_SELECT_CURSOR() SelectWorkingTrackWriteCursorFramePosition()
#define WRITE_VALIDATE_TYPE() ValidateWriteTypeAgainstWorkingTrackState()
#define WRITE_VALIDATE_TRACK_STATE() ValidateWriteOperationTrackState()
#define WRITE_VALIDATE_PMA() ValidatePmaStartBeforeAtipLeadInForWrite()
#define WRITE_MAP_ADDRESS(value) MapFixedPacketLogicalToPhysicalAddress(value)
#define WRITE_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define WRITE_INITIALIZE_COROUTINE(value) InitializeCoroutine2(value)
#define WRITE_YIELD() YieldCoroutine0()
#define WRITE_SET_SENSE(key, value) SetSenseData((key), (value))
#define WRITE_ENSURE_OPC() EnsureOpcForSelectedWriteSpeed()
#define WRITE_BUILD_RAW_Q() InitializeAndBuildRawReadQRecordQueue()
#define WRITE_ACQUIRE_ZERO_RANGE(value) FUN_RAM_fbcde5(value)
#define WRITE_NO_SENSE() SetNoSenseData()
#define WRITE_RECONCILE_START(value) ValidateAndReconcileWriteCommandStartLba(value)
#define WRITE_SELECT_PACKET_PROGRESS() FUN_RAM_fbc6b0()
#define WRITE_VALIDATE_EXTENT(flag, count) ValidateWriteTransferExtentAgainstTrackBoundary((flag), (count))
#define WRITE_INITIALIZE_PIPELINE(flag) InitializeOrContinueWriteCommandRawReadPipeline(flag)
#define WRITE_SELECT_LOWEST_SPEED() SelectStateEligibleLowestWriteSpeed()
#define WRITE_ADVANCE_TYPE1(count, flag) AdvanceWorkingTrackWriteCursorByCallerFlag((count), (flag))
#define WRITE_ADVANCE_AFTER(start, count, flag) AdvanceWorkingTrackWriteCursorAfterTransfer((start), (count), (flag))
#define WRITE_UPDATE_FAILURE() UpdateWorkingTrackAfterWriteFailure()
#endif
#endif
