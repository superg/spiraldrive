#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveFormatValidateCdb(fw_addr_t mask);
extern void ObserveFormatUnsupported(void);
extern void ObserveFormatSetSense(u8 key, u16 code);
extern void ObserveFormatStatusSense(u8 status);
extern void ObserveFormatSequenceError(void);
extern void ObserveFormatInitializeDefaults(void);
extern u8 ObserveFormatValidateControllerCode(void);
extern u8 ObserveFormatValidateControllerFlags(void);
extern void ObserveFormatNormalizeSpeed(void);
extern u8 ObserveFormatReceiveParameters(fw_addr_t destination, u32 count);
extern u8 ObserveFormatParseCode11(void);
extern void ObserveFormatInitializeTrackFlags(void);
extern void ObserveFormatResetRawRead(void);
extern void ObserveFormatInitializeCoroutine2(u32 entry_point);
extern void ObserveFormatYield(void);
extern u8 ObserveFormatEnsureOpc(void);
extern void ObserveFormatExecuteRawReadPlan(void);
extern void ObserveFormatUpdateTrackState(void);
extern void ObserveFormatResetTrackRecords(u8 first_index);
extern void ObserveFormatLoadTrack(u8 track_number);
extern u8 ObserveFormatGetLastTrack(void);
extern u8 ObserveFormatTailUninitialized(void);
extern void ObserveFormatUpdateTrackTable(void);
extern void ObserveFormatCompleteNoSense(void);
extern void ObserveFormatStartCode11(u32 variant);
extern void ObserveFormatRunCode11Scan(void);
extern void ObserveFormatExecuteMedia(void);
extern u8 ObserveFormatExactPacketMultiple(u32 block_count);
#define FORMAT_VALIDATE(mask) ObserveFormatValidateCdb(mask)
#define FORMAT_UNSUPPORTED() ObserveFormatUnsupported()
#define FORMAT_SENSE(key, code) ObserveFormatSetSense((key), (code))
#define FORMAT_STATUS_SENSE(status) ObserveFormatStatusSense(status)
#define FORMAT_SEQUENCE_ERROR() ObserveFormatSequenceError()
#define FORMAT_INITIALIZE_DEFAULTS() ObserveFormatInitializeDefaults()
#define FORMAT_VALIDATE_CODE() ObserveFormatValidateControllerCode()
#define FORMAT_VALIDATE_FLAGS() ObserveFormatValidateControllerFlags()
#define FORMAT_NORMALIZE_SPEED() ObserveFormatNormalizeSpeed()
#define FORMAT_RECEIVE(destination, count) ObserveFormatReceiveParameters((destination), (count))
#define FORMAT_PARSE_CODE11() ObserveFormatParseCode11()
#define FORMAT_INITIALIZE_TRACK_FLAGS() ObserveFormatInitializeTrackFlags()
#define FORMAT_RESET_RAW_READ() ObserveFormatResetRawRead()
#define FORMAT_INITIALIZE_COROUTINE(entry) ObserveFormatInitializeCoroutine2(entry)
#define FORMAT_YIELD() ObserveFormatYield()
#define FORMAT_ENSURE_OPC() ObserveFormatEnsureOpc()
#define FORMAT_EXECUTE_RAW_READ_PLAN() ObserveFormatExecuteRawReadPlan()
#define FORMAT_UPDATE_TRACK_STATE() ObserveFormatUpdateTrackState()
#define FORMAT_RESET_TRACK_RECORDS(index) ObserveFormatResetTrackRecords(index)
#define FORMAT_LOAD_TRACK(number) ObserveFormatLoadTrack(number)
#define FORMAT_GET_LAST_TRACK() ObserveFormatGetLastTrack()
#define FORMAT_TAIL_UNINITIALIZED() ObserveFormatTailUninitialized()
#define FORMAT_UPDATE_TRACK_TABLE() ObserveFormatUpdateTrackTable()
#define FORMAT_COMPLETE_NO_SENSE() ObserveFormatCompleteNoSense()
#define FORMAT_START_CODE11(variant) ObserveFormatStartCode11(variant)
#define FORMAT_RUN_CODE11_SCAN() ObserveFormatRunCode11Scan()
#define FORMAT_EXECUTE_MEDIA() ObserveFormatExecuteMedia()
#define FORMAT_EXACT_MULTIPLE(count) ObserveFormatExactPacketMultiple(count)
#define FORMAT_DIVIDE(dividend, divisor) ((dividend) / (divisor))
#define FORMAT_MULTIPLY(left, right) ((left) * (right))
#else
#include "platform/arithmetic.h"
#include "platform/coroutine_initialization.h"
#include "platform/encoding.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#include "platform/track_state.h"
#include "scsi/format_unit_code11.h"
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
extern void YieldCoroutine0(void);
extern void LoadTrackInformationByNumber(u8 track_number);
extern void UpdateTrackStateAfterBlankOrFormatOperation(void);
extern void StartFormatUnitCode11Operation(u32 format_variant);
extern void RunFormatUnitCode11MediaScan(void);
#define FORMAT_VALIDATE(mask) ValidateScsiCdbFieldMasks(mask)
#define FORMAT_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define FORMAT_SENSE(key, code) SetSenseData((key), (code))
#define FORMAT_STATUS_SENSE(status) SetSenseDataFromStatusCode(status)
#define FORMAT_SEQUENCE_ERROR() SetCommandSequenceErrorSense()
#define FORMAT_INITIALIZE_DEFAULTS() InitializeWriteModePage05AndTrackStateDefaults()
#define FORMAT_VALIDATE_CODE() ValidateMediaState2ControllerCodeForWrite()
#define FORMAT_VALIDATE_FLAGS() ValidateMediaState2ControllerFlagsForWrite()
#define FORMAT_NORMALIZE_SPEED() NormalizeWriteSpeedForCurrentMedia()
#define FORMAT_RECEIVE(destination, count) ExecuteDataOutAndCopyFromControllerBuffer((destination), (count))
#define FORMAT_PARSE_CODE11() ParseFormatUnitCode11ParameterList()
#define FORMAT_INITIALIZE_TRACK_FLAGS() InitializeWorkingTrackFlagsFromSelectedTrackMode()
#define FORMAT_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define FORMAT_INITIALIZE_COROUTINE(entry) InitializeCoroutine2(entry)
#define FORMAT_YIELD() YieldCoroutine0()
#define FORMAT_ENSURE_OPC() EnsureOpcForSelectedWriteSpeed()
#define FORMAT_EXECUTE_RAW_READ_PLAN() ExecuteBlankOrFormatRawReadPlan()
#define FORMAT_UPDATE_TRACK_STATE() UpdateTrackStateAfterBlankOrFormatOperation()
#define FORMAT_RESET_TRACK_RECORDS(index) ResetTrackInformationRecordsFromIndex(index)
#define FORMAT_LOAD_TRACK(number) LoadTrackInformationByNumber(number)
#define FORMAT_GET_LAST_TRACK() GetEffectiveLastTrackNumber()
#define FORMAT_TAIL_UNINITIALIZED() IsTrackTableTailEntryUninitialized()
#define FORMAT_UPDATE_TRACK_TABLE() UpdateTrackTableFromWorkingTrack()
#define FORMAT_COMPLETE_NO_SENSE() CompleteScsiCommandWithNoSense()
#define FORMAT_START_CODE11(variant) StartFormatUnitCode11Operation(variant)
#define FORMAT_RUN_CODE11_SCAN() RunFormatUnitCode11MediaScan()
#define FORMAT_EXECUTE_MEDIA() ExecuteFormatUnitMediaOperation()
#define FORMAT_EXACT_MULTIPLE(count) IsWritePacketSizeExactMultiple(count)
#define FORMAT_DIVIDE(dividend, divisor) DivideU32Quotient((dividend), (divisor))
#define FORMAT_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u8 minute = FirmwareRead8(descriptor + 4u);
u8 second = FirmwareRead8(descriptor + 5u);
u8 frame = FirmwareRead8(descriptor + 6u);
reference = (u32)(((u32)minute * 60ul + second) * 75ul + frame);
#else
reference = (u32)ConvertBinaryMsfValuesToAdjustedFramePosition(FirmwareRead8(descriptor + 4u), FirmwareRead8(descriptor + 5u), FirmwareRead8(descriptor + 6u));
#endif
#endif
