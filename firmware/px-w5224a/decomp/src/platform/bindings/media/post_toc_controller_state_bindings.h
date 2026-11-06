#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePostTocControllerStage(u8 stage);
extern s32 ObservePostTocLogarithmicScale(u16 value);
extern u8 ObservePostTocControllerInitialization(void);

#define RESET_ALL_TOC_STATE() ObservePostTocControllerStage(1u)
#define RESET_ALL_PMA_STATE() ObservePostTocControllerStage(2u)
#define RESET_ATIP_STATE() ObservePostTocControllerStage(3u)
#define LOAD_CALIBRATION_STATE() ObservePostTocControllerStage(4u)
#define RESET_CONTROLLER_STATE() ObservePostTocControllerStage(5u)
#define RESET_CONTROLLER_HARDWARE() ObservePostTocControllerStage(6u)
#define RESTORE_READ_BUFFER_SNAPSHOT() ObservePostTocControllerStage(7u)
#define RESET_DRIVE_CONTROL() ObservePostTocControllerStage(8u)
#define CONFIGURE_PARAMETER_STATE2() ObservePostTocControllerStage(9u)
#define DECODE_ATIP_FIELDS() ObservePostTocControllerStage(10u)
#define APPLY_POST_TOC_PROFILE() ObservePostTocControllerStage(11u)
#define RECALCULATE_RADIAL_STATE() ObservePostTocControllerStage(12u)
#define COMPUTE_LOGARITHMIC_SCALE(value) ObservePostTocLogarithmicScale(value)
#define INITIALIZE_CONTROLLER() ObservePostTocControllerInitialization()
#else
#define RESET_ALL_TOC_STATE() ResetAllTocAcquisitionState()
#define RESET_ALL_PMA_STATE() ResetAllPmaQRecordTables()
#define RESET_ATIP_STATE() ResetAtipState()
#define LOAD_CALIBRATION_STATE() LoadControllerCalibrationConfigurationState()
#define RESET_CONTROLLER_STATE() ResetControllerOperatingState()
#define RESET_CONTROLLER_HARDWARE() ResetControllerHardwareRegisters()
#define RESTORE_READ_BUFFER_SNAPSHOT() RestoreReadBufferMode1Id2Snapshot()
#define RESET_DRIVE_CONTROL() ResetDriveControlStateToEight()
#define CONFIGURE_PARAMETER_STATE2() ConfigureControllerForParameterState2()
#define DECODE_ATIP_FIELDS() DecodeAtipTimeFieldsAndFramePositions()
#define APPLY_POST_TOC_PROFILE() ApplyPostTocControllerCalibrationProfile()
#define RECALCULATE_RADIAL_STATE() RecalculateRadialStepScaleAndDerivedState()
#define COMPUTE_LOGARITHMIC_SCALE(value) Compute2000TimesLog10OfWordOver32767(value)
#define INITIALIZE_CONTROLLER() InitializeControllerOperatingStateAndRunCalibration()
#endif
#endif
