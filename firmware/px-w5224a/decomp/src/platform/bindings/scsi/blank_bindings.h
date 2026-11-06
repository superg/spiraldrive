#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveBlankValidateCode(void);
extern u8 ObserveBlankValidateFlags(void);
extern u8 ObserveBlankValidateCdb(fw_addr_t mask);
extern void ObserveBlankUnsupported(void);
extern void ObserveBlankSense(u8 key, u16 code);
extern void ObserveBlankStatusSense(u8 status);
extern void ObserveBlankSequenceError(void);
extern void ObserveBlankNormalizeSpeed(void);
extern void ObserveBlankNoSense(void);
extern void ObserveBlankCompleteNoSense(void);
extern u8 ObserveBlankLastTrack(void);
extern u8 ObserveBlankTailUninitialized(void);
extern void ObserveBlankLoadTrack(u8 track);
extern u8 ObserveBlankFindTrack(s32 frame_position);
extern void ObserveBlankInitializeCoroutine1(u32 entry);
extern void ObserveBlankResetRawRead(void);
extern void ObserveBlankInitializeCoroutine2(u32 entry);
extern void ObserveBlankYield(void);
extern u8 ObserveBlankEnsureOpc(void);
extern u8 ObserveBlankExecutePlan(void);
extern void ObserveBlankUpdateTrackState(void);
extern void ObserveBlankResetType2Records(void);
extern s32 ObserveBlankConvertMsf(u8 minute, u8 second, u8 frame);
#define BLANK_VALIDATE_CODE() ObserveBlankValidateCode()
#define BLANK_VALIDATE_FLAGS() ObserveBlankValidateFlags()
#define BLANK_VALIDATE_CDB(mask) ObserveBlankValidateCdb(mask)
#define BLANK_UNSUPPORTED() ObserveBlankUnsupported()
#define BLANK_SENSE(key, code) ObserveBlankSense((key), (code))
#define BLANK_STATUS_SENSE(status) ObserveBlankStatusSense(status)
#define BLANK_SEQUENCE_ERROR() ObserveBlankSequenceError()
#define BLANK_NORMALIZE_SPEED() ObserveBlankNormalizeSpeed()
#define BLANK_NO_SENSE() ObserveBlankNoSense()
#define BLANK_COMPLETE_NO_SENSE() ObserveBlankCompleteNoSense()
#define BLANK_LAST_TRACK() ObserveBlankLastTrack()
#define BLANK_TAIL_UNINITIALIZED() ObserveBlankTailUninitialized()
#define BLANK_LOAD_TRACK(track) ObserveBlankLoadTrack(track)
#define BLANK_FIND_TRACK(frame) ObserveBlankFindTrack(frame)
#define BLANK_INITIALIZE_COROUTINE1(entry) ObserveBlankInitializeCoroutine1(entry)
#define BLANK_RESET_RAW_READ() ObserveBlankResetRawRead()
#define BLANK_INITIALIZE_COROUTINE2(entry) ObserveBlankInitializeCoroutine2(entry)
#define BLANK_YIELD() ObserveBlankYield()
#define BLANK_ENSURE_OPC() ObserveBlankEnsureOpc()
#define BLANK_EXECUTE_PLAN() ObserveBlankExecutePlan()
#define BLANK_UPDATE_TRACK_STATE() ObserveBlankUpdateTrackState()
#define BLANK_RESET_TYPE2_RECORDS() ObserveBlankResetType2Records()
#define BLANK_CONVERT_MSF(m, s, f) ObserveBlankConvertMsf((m), (s), (f))
#define BLANK_DIVIDE(dividend, divisor) ((dividend) / (divisor))
#define BLANK_MULTIPLY(left, right) ((left) * (right))
#define BLANK_REMAINDER(dividend, divisor) ((dividend) % (divisor))
#else
#include "platform/arithmetic.h"
#include "platform/coroutine_initialization.h"
#include "platform/encoding.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void YieldCoroutine0(void);
extern void LoadTrackInformationByNumber(u8 track_number);
extern void UpdateTrackStateAfterBlankOrFormatOperation(void);
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
extern u32 DivideU32RemainderNative(u32 dividend, u32 divisor);
#define BLANK_VALIDATE_CODE() ValidateMediaState2ControllerCodeForWrite()
#define BLANK_VALIDATE_FLAGS() ValidateMediaState2ControllerFlagsForWrite()
#define BLANK_VALIDATE_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define BLANK_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define BLANK_SENSE(key, code) SetSenseData((key), (code))
#define BLANK_STATUS_SENSE(status) SetSenseDataFromStatusCode(status)
#define BLANK_SEQUENCE_ERROR() SetCommandSequenceErrorSense()
#define BLANK_NORMALIZE_SPEED() NormalizeWriteSpeedForCurrentMedia()
#define BLANK_NO_SENSE() SetNoSenseData()
#define BLANK_COMPLETE_NO_SENSE() CompleteScsiCommandWithNoSense()
#define BLANK_LAST_TRACK() GetEffectiveLastTrackNumber()
#define BLANK_TAIL_UNINITIALIZED() IsTrackTableTailEntryUninitialized()
#define BLANK_LOAD_TRACK(track) LoadTrackInformationByNumber(track)
#define BLANK_FIND_TRACK(frame) FindTrackTableIndexForFramePosition(frame)
#define BLANK_INITIALIZE_COROUTINE1(entry) InitializeCoroutine1(entry)
#define BLANK_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define BLANK_INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define BLANK_YIELD() YieldCoroutine0()
#define BLANK_ENSURE_OPC() EnsureOpcForSelectedWriteSpeed()
#define BLANK_EXECUTE_PLAN() ExecuteBlankOrFormatRawReadPlan()
#define BLANK_UPDATE_TRACK_STATE() UpdateTrackStateAfterBlankOrFormatOperation()
#define BLANK_RESET_TYPE2_RECORDS() ResetType2ControllerRecordPipelineState()
#define BLANK_CONVERT_MSF(m, s, f) ConvertBinaryMsfValuesToAdjustedFramePosition((m), (s), (f))
#define BLANK_DIVIDE(dividend, divisor) DivideU32Quotient((dividend), (divisor))
#define BLANK_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define BLANK_REMAINDER(dividend, divisor) DivideU32RemainderNative((dividend), (divisor))
#endif
#endif
