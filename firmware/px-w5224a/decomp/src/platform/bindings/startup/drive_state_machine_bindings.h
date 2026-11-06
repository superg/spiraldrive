#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDriveStateMachineWriteP3(u8 bit0, u8 bit1);
extern void ObserveDriveStateMachineClearCompletionBit(void);
extern void ObserveDriveStateMachineLoadCalibration(void);
extern void ObserveDriveStateMachineLoadOffsets(void);
extern void ObserveDriveStateMachineYield(void);
extern u8 ObserveDriveStateMachineRecognize(void);
extern void ObserveDriveStateMachineIndexedTransitions(void);
extern void ObserveDriveStateMachineRestart(u32 entry_point);
extern void ObserveDriveStateMachineConfigureMediaRead(void);
extern void ObserveDriveStateMachineBuildInterpolation(void);
extern void ObserveDriveStateMachineInitializeMediaRead(void);
extern u8 ObserveDriveStateMachineIndexedState(u8 state_code);
#define DRIVE_STATE_WRITE_P3(bit0, bit1) ObserveDriveStateMachineWriteP3((bit0), (bit1))
#define DRIVE_STATE_CLEAR_COMPLETION() ObserveDriveStateMachineClearCompletionBit()
#define DRIVE_STATE_LOAD_CALIBRATION() ObserveDriveStateMachineLoadCalibration()
#define DRIVE_STATE_LOAD_OFFSETS() ObserveDriveStateMachineLoadOffsets()
#define DRIVE_STATE_YIELD() ObserveDriveStateMachineYield()
#define DRIVE_STATE_RECOGNIZE() ObserveDriveStateMachineRecognize()
#define DRIVE_STATE_INDEXED_TRANSITIONS() ObserveDriveStateMachineIndexedTransitions()
#define DRIVE_STATE_RESTART(value) ObserveDriveStateMachineRestart(value)
#define DRIVE_STATE_CONFIGURE_MEDIA_READ() ObserveDriveStateMachineConfigureMediaRead()
#define DRIVE_STATE_BUILD_INTERPOLATION() ObserveDriveStateMachineBuildInterpolation()
#define DRIVE_STATE_INITIALIZE_MEDIA_READ() ObserveDriveStateMachineInitializeMediaRead()
#define DRIVE_STATE_INDEXED_STATE(value) ObserveDriveStateMachineIndexedState(value)
#else
#include "media/disc_recognition.h"
#include "media/raw_read_runtime.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void RestartCoroutine1AndYield(u32 entry_point);
extern void YieldCoroutine1(void);
u8 bdata DriveStateMachineBitByte27 _at_ 0x27;
sbit DriveStateMachineCompletionBit = DriveStateMachineBitByte27 ^ 1;
sbit DriveStateMachineP3Bit0 = 0xb0 ^ 0;
sbit DriveStateMachineP3Bit1 = 0xb0 ^ 1;
#define DRIVE_STATE_WRITE_P3(bit0, bit1)                                                                                                                                                               \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        DriveStateMachineP3Bit0 = (bit0);                                                                                                                                                              \
        DriveStateMachineP3Bit1 = (bit1);                                                                                                                                                              \
    } while(0)
#define DRIVE_STATE_CLEAR_COMPLETION() (DriveStateMachineCompletionBit = 0)
#define DRIVE_STATE_LOAD_CALIBRATION() LoadControllerCalibrationConfigurationState()
#define DRIVE_STATE_LOAD_OFFSETS() LoadMediaCalibrationOffsetsAndControllerParameterFc()
#define DRIVE_STATE_YIELD() YieldCoroutine1()
#define DRIVE_STATE_RECOGNIZE() RunDiscRecognitionAndTocInitialization()
#define DRIVE_STATE_INDEXED_TRANSITIONS() ServiceActiveIndexedDriveStateTransitions()
#define DRIVE_STATE_RESTART(value) RestartCoroutine1AndYield(value)
#define DRIVE_STATE_CONFIGURE_MEDIA_READ() ConfigureMediaReadStateForIndexedDriveState()
#define DRIVE_STATE_BUILD_INTERPOLATION() BuildControllerInterpolationTablesForMeasurementModes2Or3()
#define DRIVE_STATE_INITIALIZE_MEDIA_READ() InitializeMediaReadControlState()
#define DRIVE_STATE_INDEXED_STATE(value) IsIndexedDriveStateActive(value)
#endif
#endif
