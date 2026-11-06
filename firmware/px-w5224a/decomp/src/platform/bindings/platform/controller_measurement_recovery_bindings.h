#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveMeasurementRecoveryAcquirePosition(void);
extern void ObserveMeasurementRecoveryResetDriveControl(void);
extern u8 ObserveMeasurementRecoveryInitializeController(void);
extern void ObserveMeasurementRecoverySetOperatingEnable(u8 enabled);
extern u8 ObserveMeasurementRecoveryCheckTolerance(u8 restart_timeout);
extern void ObserveMeasurementRecoveryConfigurePorts(u8 mode_selector);
extern void ObserveMeasurementRecoveryApplyChannels(void);
extern void ObserveMeasurementRecoveryWriteD4E2(u8 code);
extern void ObserveMeasurementRecoveryRaiseF4(void);
extern void ObserveMeasurementRecoveryPollPacket(void);
extern void ObserveMeasurementRecoveryRestoreD4E2(void);
extern void ObserveMeasurementRecoveryLowerF4(void);
extern void ObserveMeasurementRecoveryEvaluateTail(void);
extern u8 ObserveMeasurementRecoveryEvaluateSeries(u8 fallback);
#define RECOVERY_ACQUIRE_POSITION() ObserveMeasurementRecoveryAcquirePosition()
#define RECOVERY_RESET_DRIVE_CONTROL() ObserveMeasurementRecoveryResetDriveControl()
#define RECOVERY_INITIALIZE_CONTROLLER() ObserveMeasurementRecoveryInitializeController()
#define RECOVERY_SET_OPERATING_ENABLE(value) ObserveMeasurementRecoverySetOperatingEnable(value)
#define RECOVERY_CHECK_TOLERANCE(value) ObserveMeasurementRecoveryCheckTolerance(value)
#define RECOVERY_CONFIGURE_PORTS(value) ObserveMeasurementRecoveryConfigurePorts(value)
#define RECOVERY_APPLY_CHANNELS() ObserveMeasurementRecoveryApplyChannels()
#define RECOVERY_WRITE_D4_E2(value) ObserveMeasurementRecoveryWriteD4E2(value)
#define RECOVERY_RAISE_F4() ObserveMeasurementRecoveryRaiseF4()
#define RECOVERY_POLL_PACKET() ObserveMeasurementRecoveryPollPacket()
#define RECOVERY_RESTORE_D4_E2() ObserveMeasurementRecoveryRestoreD4E2()
#define RECOVERY_LOWER_F4() ObserveMeasurementRecoveryLowerF4()
#define RECOVERY_EVALUATE_TAIL() ObserveMeasurementRecoveryEvaluateTail()
#define RECOVERY_EVALUATE_SERIES(value) ObserveMeasurementRecoveryEvaluateSeries(value)
#else
#include "platform/controller_parameters.h"
#include "platform/drive_control_reset.h"
extern u8 AcquireDiscReadPosition(void);
extern u8 InitializeControllerOperatingStateAndRunCalibration(void);
extern u8 CheckControllerMeasurementToleranceOrTimeout(u8 restart_timeout);
extern void ConfigureControllerPortsAndParametersForCurrentOperatingCode(u8 mode_selector);
extern void PLEXTOR_NEAR_FUNCTION RetryControllerMeasurementEvaluation(void);
extern void EvaluateControllerMeasurementTailResiduals(void);
extern u8 EvaluateControllerMeasurementSeriesForCurrentModeWithFallback(u8 fallback);
#define RECOVERY_ACQUIRE_POSITION() AcquireDiscReadPosition()
#define RECOVERY_RESET_DRIVE_CONTROL() ResetDriveControlStateToEight()
#define RECOVERY_INITIALIZE_CONTROLLER() InitializeControllerOperatingStateAndRunCalibration()
#define RECOVERY_SET_OPERATING_ENABLE(value) SetControllerOperatingEnableState(value)
#define RECOVERY_CHECK_TOLERANCE(value) CheckControllerMeasurementToleranceOrTimeout(value)
#define RECOVERY_CONFIGURE_PORTS(value) ConfigureControllerPortsAndParametersForCurrentOperatingCode(value)
#define RECOVERY_APPLY_CHANNELS() ApplyOperatingCodeAndFixedControllerChannels0Through2()
#define RECOVERY_WRITE_D4_E2(value) WriteControllerParametersD4AndE2ForCode(value)
#define RECOVERY_RAISE_F4() WriteControllerParameterF4WithUpAdjustment()
#define RECOVERY_POLL_PACKET() RetryControllerMeasurementEvaluation()
#define RECOVERY_RESTORE_D4_E2() WriteControllerParametersD412AndE2Value58()
#define RECOVERY_LOWER_F4() WriteControllerParameterF4WithDownAdjustment()
#define RECOVERY_EVALUATE_TAIL() EvaluateControllerMeasurementTailResiduals()
#define RECOVERY_EVALUATE_SERIES(value) EvaluateControllerMeasurementSeriesForCurrentModeWithFallback(value)
#endif
#endif
