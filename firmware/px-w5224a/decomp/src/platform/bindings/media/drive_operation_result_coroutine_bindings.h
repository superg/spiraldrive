#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDriveOperationResultExecute(u8 mode, u8 speed);
extern u8 ObserveDriveOperationResultIndexed(u8 state);
extern u8 ObserveDriveOperationResultConfiguration(fw_addr_t source, u16 destination_offset, u16 byte_count);
extern void ObserveDriveOperationResultState10(void);
extern void ObserveDriveOperationResultRestart(u32 entry_point);
extern void ObserveDriveOperationResultRecovery(void);
extern void ObserveDriveOperationResultYield(void);
extern void ObserveDriveOperationResultStatusUpdate(void);
extern void ObserveDriveOperationResultPattern(u8 restart_pattern);
extern void ObserveDriveOperationResultInitialize(u32 entry_point);
#define RESULT_EXECUTE(mode, speed) ObserveDriveOperationResultExecute((mode), (speed))
#define RESULT_INDEXED(state) ObserveDriveOperationResultIndexed(state)
#define RESULT_CONFIGURATION(source, offset, count) ObserveDriveOperationResultConfiguration((source), (offset), (count))
#define RESULT_STATE10() ObserveDriveOperationResultState10()
#define RESULT_RESTART(entry) ObserveDriveOperationResultRestart(entry)
#define RESULT_RECOVERY() ObserveDriveOperationResultRecovery()
#define RESULT_YIELD() ObserveDriveOperationResultYield()
#define RESULT_STATUS_UPDATE() ObserveDriveOperationResultStatusUpdate()
#define RESULT_PATTERN(value) ObserveDriveOperationResultPattern(value)
#define RESULT_INITIALIZE(entry) ObserveDriveOperationResultInitialize(entry)
#else
extern void YieldCoroutine0(void);
#define RESULT_EXECUTE(mode, speed) ExecuteRawReadOperationForModeAndSpeed((mode), (speed))
#define RESULT_INDEXED(state) IsIndexedDriveStateActive(state)
#define RESULT_CONFIGURATION(source, offset, count) UpdateAndProgramDriveConfigurationRange((source), (offset), (count))
#define RESULT_STATE10() RunDriveControlState10Sequence()
#define RESULT_RESTART(entry) RestartCoroutine1AndYield(entry)
#define RESULT_RECOVERY() RunFullMediaReadVerifyRecoveryCoroutine0()
#define RESULT_YIELD() YieldCoroutine0()
#define RESULT_STATUS_UPDATE() FUN_RAM_f8f08b()
#define RESULT_PATTERN(value) UpdateP3Bits0And1PatternState(value)
#define RESULT_INITIALIZE(entry) InitializeCoroutine1(entry)
#endif
#endif
