#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateSendOpcCdb(fw_addr_t mask_descriptor);
extern void ObserveRejectSendOpcCommand(void);
extern u8 ObserveValidateSendOpcMedia(void);
extern void ObserveNormalizeSendOpcWriteSpeed(void);
extern u8 ObserveExecuteSendOpcDataOut(fw_addr_t destination, u32 byte_count);
extern void ObserveSetSendOpcSense(u8 sense_key, u16 sense_code);
extern void ObserveResetSendOpcRawReadQueue(void);
extern void ObserveInitializeSendOpcCoroutine2(void);
extern void ObserveYieldSendOpcCoroutine0(void);
extern u8 ObserveRunOpcForCurrentWriteSpeed(void);
extern void ObserveSetSendOpcNoSense(void);
#define VALIDATE_CDB(mask) ObserveValidateSendOpcCdb(mask)
#define REJECT_COMMAND() ObserveRejectSendOpcCommand()
#define VALIDATE_MEDIA() ObserveValidateSendOpcMedia()
#define NORMALIZE_WRITE_SPEED() ObserveNormalizeSendOpcWriteSpeed()
#define EXECUTE_DATA_OUT(destination, count) ObserveExecuteSendOpcDataOut((destination), (count))
#define SET_SENSE(key, code) ObserveSetSendOpcSense((key), (code))
#define RESET_RAW_READ_QUEUE() ObserveResetSendOpcRawReadQueue()
#define INITIALIZE_COROUTINE2() ObserveInitializeSendOpcCoroutine2()
#define YIELD_COROUTINE0() ObserveYieldSendOpcCoroutine0()
#define RUN_OPC() ObserveRunOpcForCurrentWriteSpeed()
#define SET_NO_SENSE() ObserveSetSendOpcNoSense()
#else
extern u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_descriptor);
extern void HandleUnsupportedScsiCommand(void);
extern u8 ValidateRecordableMediaCommandEligibility(void);
extern void NormalizeWriteSpeedForCurrentMedia(void);
extern u8 ExecuteDataOutAndCopyFromControllerBuffer(fw_addr_t destination, u32 byte_count);
extern void SetSenseData(u8 sense_key, u16 sense_code);
extern void ResetRawReadQueueAndTransferState(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF84A00ForC(void);
extern void YieldCoroutine0(void);
extern u8 RunOpcForCurrentWriteSpeed(void);
extern void SetNoSenseData(void);
#define VALIDATE_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define REJECT_COMMAND() HandleUnsupportedScsiCommand()
#define VALIDATE_MEDIA() ValidateRecordableMediaCommandEligibility()
#define NORMALIZE_WRITE_SPEED() NormalizeWriteSpeedForCurrentMedia()
#define EXECUTE_DATA_OUT(destination, count) ExecuteDataOutAndCopyFromControllerBuffer((destination), (count))
#define SET_SENSE(key, code) SetSenseData((key), (code))
#define RESET_RAW_READ_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_COROUTINE2() InitializeCoroutine2AtF84A00ForC()
#define YIELD_COROUTINE0() YieldCoroutine0()
#define RUN_OPC() RunOpcForCurrentWriteSpeed()
#define SET_NO_SENSE() SetNoSenseData()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePrepareOpcHandshakePins(void);
extern void ObserveYieldOpcCoroutine0(void);
extern void ObserveServiceOpcPendingHostCommand(void);
extern void ObserveSetOpcSenseFromStatus(u8 status_code);
extern void ObserveSetOpcSenseFromPendingResult(void);
extern void ObserveUpdateOpcSpeedClass(u8 write_speed_x);
extern void ObserveSetOpcDirectSense(u8 sense_key, u16 sense_code);
#define PREPARE_HANDSHAKE_PINS() ObservePrepareOpcHandshakePins()
#define OPC_YIELD() ObserveYieldOpcCoroutine0()
#define SERVICE_PENDING_HOST() ObserveServiceOpcPendingHostCommand()
#define SET_SENSE_FROM_STATUS(code) ObserveSetOpcSenseFromStatus(code)
#define SET_SENSE_FROM_PENDING() ObserveSetOpcSenseFromPendingResult()
#define UPDATE_OPC_SPEED_CLASS(speed) ObserveUpdateOpcSpeedClass(speed)
#define SET_DIRECT_SENSE(key, code) ObserveSetOpcDirectSense((key), (code))
#else
extern void PrepareOpcHandshakePinsAtomic(void);
extern void ServicePendingHostCommandWhenOperationFlagsPermit(void);
extern void SetSenseDataFromStatusCode(u8 status_code);
extern void SetSenseDataFromPendingResultCode(void);
extern void UpdateOpcCalibrationSpeedClass(u8 write_speed_x);
#define PREPARE_HANDSHAKE_PINS() PrepareOpcHandshakePinsAtomic()
#define OPC_YIELD() YieldCoroutine0()
#define SERVICE_PENDING_HOST() ServicePendingHostCommandWhenOperationFlagsPermit()
#define SET_SENSE_FROM_STATUS(code) SetSenseDataFromStatusCode(code)
#define SET_SENSE_FROM_PENDING() SetSenseDataFromPendingResultCode()
#define UPDATE_OPC_SPEED_CLASS(speed) UpdateOpcCalibrationSpeedClass(speed)
#define SET_DIRECT_SENSE(key, code) SetSenseData((key), (code))
#endif
#endif
