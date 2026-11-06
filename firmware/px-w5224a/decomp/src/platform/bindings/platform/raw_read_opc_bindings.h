#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveIsOpcRequiredForRawRead(u8 write_speed_x);
extern void ObserveResetRawReadQueueForOpc(void);
extern void ObserveInitializeCoroutine2ForOpc(u32 entry_point);
extern void ObserveYieldCoroutine0ForOpc(void);
extern void ObserveSetSenseDataFromPendingOpcResult(void);
extern void ObserveUpdateOpcCalibrationSpeedClassForRawRead(u8 write_speed_x);
#define IS_OPC_REQUIRED(speed) ObserveIsOpcRequiredForRawRead(speed)
#define RESET_RAW_READ_QUEUE() ObserveResetRawReadQueueForOpc()
#define INITIALIZE_COROUTINE2(entry) ObserveInitializeCoroutine2ForOpc(entry)
#define YIELD_COROUTINE0() ObserveYieldCoroutine0ForOpc()
#define SET_SENSE_FROM_PENDING_RESULT() ObserveSetSenseDataFromPendingOpcResult()
#define UPDATE_OPC_SPEED_CLASS(speed) ObserveUpdateOpcCalibrationSpeedClassForRawRead(speed)
#else
extern u8 IsOpcRequiredForWriteSpeed(u8 write_speed_x);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(u32 entry_point);
extern void YieldCoroutine0(void);
extern void SetSenseDataFromPendingResultCode(void);
extern void UpdateOpcCalibrationSpeedClass(u8 write_speed_x);
#define IS_OPC_REQUIRED(speed) IsOpcRequiredForWriteSpeed(speed)
#define RESET_RAW_READ_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define YIELD_COROUTINE0() YieldCoroutine0()
#define SET_SENSE_FROM_PENDING_RESULT() SetSenseDataFromPendingResultCode()
#define UPDATE_OPC_SPEED_CLASS(speed) UpdateOpcCalibrationSpeedClass(speed)
#endif
#endif
