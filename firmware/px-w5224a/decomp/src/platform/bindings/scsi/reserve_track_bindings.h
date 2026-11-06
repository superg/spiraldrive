#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateReserveTrackCdb(fw_addr_t mask_descriptor);
extern void ObserveRejectReserveTrackCommand(void);
extern void ObserveSetReserveTrackSequenceError(void);
extern void ObserveNormalizeReserveTrackWriteSpeed(void);
extern u8 ObserveValidateReserveTrackWriteParameters(void);
extern u8 ObserveValidateReserveTrackMedia(void);
extern void ObserveSetReserveTrackSense(u8 sense_key, u16 sense_code);
extern void ObserveClearReserveTrackWorkingInformation(void);
extern u8 ObserveLoadReserveTrackInformation(u8 track_number, u8 inherited_b);
extern u8 ObserveValidateReserveTrackPmaStart(void);
extern u8 ObserveValidateReservedTrackEndFrame(u32 requested_block_count);
extern void ObserveBuildReserveTrackAdr1Record(void);
extern void ObserveExecuteReserveTrackPreparation(void);
#define VALIDATE_CDB(mask) ObserveValidateReserveTrackCdb(mask)
#define REJECT_COMMAND() ObserveRejectReserveTrackCommand()
#define SET_SEQUENCE_ERROR() ObserveSetReserveTrackSequenceError()
#define NORMALIZE_WRITE_SPEED() ObserveNormalizeReserveTrackWriteSpeed()
#define VALIDATE_WRITE_PARAMETERS() ObserveValidateReserveTrackWriteParameters()
#define VALIDATE_MEDIA() ObserveValidateReserveTrackMedia()
#define SET_SENSE(key, code) ObserveSetReserveTrackSense((key), (code))
#define CLEAR_WORKING_TRACK() ObserveClearReserveTrackWorkingInformation()
#define LOAD_TRACK(track, inherited_b) ObserveLoadReserveTrackInformation((track), (inherited_b))
#define VALIDATE_PMA_START() ObserveValidateReserveTrackPmaStart()
#define VALIDATE_END_FRAME(count) ObserveValidateReservedTrackEndFrame(count)
#define BUILD_ADR1_RECORD() ObserveBuildReserveTrackAdr1Record()
#define EXECUTE_PREPARATION() ObserveExecuteReserveTrackPreparation()
#else
extern u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_descriptor);
extern void HandleUnsupportedScsiCommand(void);
extern void SetCommandSequenceErrorSense(void);
extern void NormalizeWriteSpeedForCurrentMedia(void);
extern u8 ValidateCurrentWriteParameterCombination(void);
extern u8 ValidateRecordableMediaCommandEligibility(void);
extern void SetSenseData(u8 sense_key, u16 sense_code);
extern void ClearWorkingTrackInformation(void);
extern u8 LoadTrackInformationByNumberFromInheritedB(u8 track_number, u8 inherited_b);
extern u8 ValidatePmaStartBeforeAtipLeadInForWrite(void);
extern u8 PLEXTOR_NEAR_FUNCTION ValidateAndSetReservedTrackEndFrame(u32 requested_block_count);
extern void BuildWorkingTrackAdr1RawQRecord(void);
extern void ExecuteReserveTrackRawReadPreparation(void);
#define VALIDATE_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define REJECT_COMMAND() HandleUnsupportedScsiCommand()
#define SET_SEQUENCE_ERROR() SetCommandSequenceErrorSense()
#define NORMALIZE_WRITE_SPEED() NormalizeWriteSpeedForCurrentMedia()
#define VALIDATE_WRITE_PARAMETERS() ValidateCurrentWriteParameterCombination()
#define VALIDATE_MEDIA() ValidateRecordableMediaCommandEligibility()
#define SET_SENSE(key, code) SetSenseData((key), (code))
#define CLEAR_WORKING_TRACK() ClearWorkingTrackInformation()
#define LOAD_TRACK(track, inherited_b) LoadTrackInformationByNumberFromInheritedB((track), (inherited_b))
#define VALIDATE_PMA_START() ValidatePmaStartBeforeAtipLeadInForWrite()
#define VALIDATE_END_FRAME(count) ValidateAndSetReservedTrackEndFrame(count)
#define BUILD_ADR1_RECORD() BuildWorkingTrackAdr1RawQRecord()
#define EXECUTE_PREPARATION() ExecuteReserveTrackRawReadPreparation()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetReserveTrackRawReadQueue(void);
extern void ObserveInitializeReserveTrackCoroutine2(void);
extern void ObserveYieldReserveTrackCoroutine0(void);
extern u8 ObserveEnsureReserveTrackOpc(void);
extern u8 ObservePrepareReserveTrackRawReadPipeline(void);
extern void ObserveUpdateReserveTrackTableFromWorking(void);
extern void ObserveSnapshotReserveTrackTailState(void);
extern void ObserveClearReserveTrackPreparationWorkingInformation(void);
#define RESET_RAW_READ_QUEUE() ObserveResetReserveTrackRawReadQueue()
#define INITIALIZE_COROUTINE2() ObserveInitializeReserveTrackCoroutine2()
#define YIELD_COROUTINE0() ObserveYieldReserveTrackCoroutine0()
#define ENSURE_OPC() ObserveEnsureReserveTrackOpc()
#define PREPARE_RAW_READ_PIPELINE() ObservePrepareReserveTrackRawReadPipeline()
#define UPDATE_TRACK_TABLE() ObserveUpdateReserveTrackTableFromWorking()
#define SNAPSHOT_TRACK_TAIL() ObserveSnapshotReserveTrackTailState()
#define CLEAR_PREPARATION_WORKING_TRACK() ObserveClearReserveTrackPreparationWorkingInformation()
#else
extern void ResetRawReadQueueAndTransferState(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF84A00ForC(void);
extern void YieldCoroutine0(void);
extern u8 EnsureOpcForSelectedWriteSpeed(void);
extern void UpdateTrackTableFromWorkingTrack(void);
extern void SnapshotTrackTableTailState(void);
#define RESET_RAW_READ_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_COROUTINE2() InitializeCoroutine2AtF84A00ForC()
#define YIELD_COROUTINE0() YieldCoroutine0()
#define ENSURE_OPC() EnsureOpcForSelectedWriteSpeed()
#define PREPARE_RAW_READ_PIPELINE() PrepareReserveTrackRawReadPipeline()
#define UPDATE_TRACK_TABLE() UpdateTrackTableFromWorkingTrack()
#define SNAPSHOT_TRACK_TAIL() SnapshotTrackTableTailState()
#define CLEAR_PREPARATION_WORKING_TRACK() ClearWorkingTrackInformation()
#endif
#endif
