#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveIndexedDriveStateOperationInTocSet(void);
extern void ObserveIndexedDriveStateReset(void);
extern void ObserveIndexedDriveStateSignal(void);
extern void ObserveIndexedDriveStateReplaceWriteSpeed(u8 value);
extern void ObserveIndexedDriveStatePersistCalibration(void);
extern void ObserveIndexedDriveStateUpdateCalibration(u8 stage);
extern void ObserveIndexedDriveStateRestart(u32 entry_point);
#define INDEXED_OPERATION_IN_TOC_SET() ObserveIndexedDriveStateOperationInTocSet()
#define INDEXED_RESET_DRIVE() ObserveIndexedDriveStateReset()
#define INDEXED_SIGNAL_PATTERN() ObserveIndexedDriveStateSignal()
#define INDEXED_REPLACE_WRITE_SPEED(value) ObserveIndexedDriveStateReplaceWriteSpeed(value)
#define INDEXED_PERSIST_CALIBRATION() ObserveIndexedDriveStatePersistCalibration()
#define INDEXED_UPDATE_CALIBRATION(stage) ObserveIndexedDriveStateUpdateCalibration(stage)
#define INDEXED_RESTART(entry) ObserveIndexedDriveStateRestart(entry)
#else
#include "platform/controller_calibration_outcome.h"
#include "platform/drive_control_reset.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void RestartCoroutine1AndYield(u32 entry_point);
#define INDEXED_OPERATION_IN_TOC_SET() IsCurrentDriveOperationCodeInTocAcquisitionSet()
#define INDEXED_RESET_DRIVE() ResetDriveControlStateToEight()
#define INDEXED_SIGNAL_PATTERN() SignalDriveStatusPatternForever()
#define INDEXED_REPLACE_WRITE_SPEED(value) ReplaceWriteSpeedSelectionFlagForValue(value)
#define INDEXED_PERSIST_CALIBRATION() PersistControllerCalibrationStateBlock()
#define INDEXED_UPDATE_CALIBRATION(stage) UpdateCalibrationOutcomeStateForStage(stage)
#define INDEXED_RESTART(entry) RestartCoroutine1AndYield(entry)
#endif
#endif
