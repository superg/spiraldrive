#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveState6SequenceEnable(u8 enabled);
extern u8 ObserveState6SequencePosition(u8 mode, u8 require_tolerance);
extern void ObserveState6SequenceConfigure(u8 mode_selector);
extern void ObserveState6SequenceApply(void);
extern u8 ObserveState6SequenceAtipWindow(void);
extern u8 ObserveState6SequenceNearTarget(s32 target);
extern u8 ObserveState6SequenceReadPath(s32 target);
extern void ObserveState6SequenceStrobe(fw_addr_t address, u8 value);
extern void ObserveState6SequenceDeriveThreshold(void);
extern void ObserveState6SequenceWriteE2(u8 value);
extern u8 ObserveState6SequencePositionWindow(void);
extern void ObserveState6SequenceRestore(u8 refresh);
extern void ObserveState6SequenceWriteD4E2(u8 value);
extern void ObserveState6SequenceRaiseF4(void);
extern u8 ObserveState6SequenceAcquireSamples(void);
extern void ObserveState6SequenceRestoreD4E2(void);
extern void ObserveState6SequenceLowerF4(void);
extern u8 ObserveState6SequenceFixedSuccess(void);
extern u8 ObserveState6SequenceEvaluateResult(void);
#define SEQUENCE_ENABLE(value) ObserveState6SequenceEnable(value)
#define SEQUENCE_POSITION(mode, tolerance) ObserveState6SequencePosition((mode), (tolerance))
#define SEQUENCE_CONFIGURE(value) ObserveState6SequenceConfigure(value)
#define SEQUENCE_APPLY() ObserveState6SequenceApply()
#define SEQUENCE_ATIP_WINDOW() ObserveState6SequenceAtipWindow()
#define SEQUENCE_NEAR_TARGET(value) ObserveState6SequenceNearTarget(value)
#define SEQUENCE_READ_PATH(value) ObserveState6SequenceReadPath(value)
#define SEQUENCE_STROBE(address, value) ObserveState6SequenceStrobe((address), (value))
#define SEQUENCE_DERIVE_THRESHOLD() ObserveState6SequenceDeriveThreshold()
#define SEQUENCE_WRITE_E2(value) ObserveState6SequenceWriteE2(value)
#define SEQUENCE_POSITION_WINDOW() ObserveState6SequencePositionWindow()
#define SEQUENCE_RESTORE(value) ObserveState6SequenceRestore(value)
#define SEQUENCE_WRITE_D4_E2(value) ObserveState6SequenceWriteD4E2(value)
#define SEQUENCE_RAISE_F4() ObserveState6SequenceRaiseF4()
#define SEQUENCE_ACQUIRE_SAMPLES() ObserveState6SequenceAcquireSamples()
#define SEQUENCE_RESTORE_D4_E2() ObserveState6SequenceRestoreD4E2()
#define SEQUENCE_LOWER_F4() ObserveState6SequenceLowerF4()
#define SEQUENCE_FIXED_SUCCESS() ObserveState6SequenceFixedSuccess()
#define SEQUENCE_EVALUATE_RESULT() ObserveState6SequenceEvaluateResult()
#define SEQUENCE_SET_COMPLETION_BIT() FirmwareWriteBit0039(1u)
#else
#include "media/calibration_offset.h"
#include "platform/controller_calibration.h"
#include "platform/controller_parameters.h"
#include "platform/controller_position_sample_acquisition.h"
#include "platform/controller_state6_calibration_positioning.h"
#include "platform/controller_state6_position_window.h"
#include "platform/leaf_workers.h"
#include "platform/media_state6_calibration_threshold.h"
#include "platform/status_mapping.h"
extern void PLEXTOR_NEAR_FUNCTION SetBit0039Atomic(void);
#define SEQUENCE_ENABLE(value) SetControllerOperatingEnableState(value)
#define SEQUENCE_POSITION(mode, tolerance) PositionReadAcquisitionForCalibrationMode((mode), (tolerance))
#define SEQUENCE_CONFIGURE(value) ConfigureControllerPortsAndParametersForCurrentOperatingCode(value)
#define SEQUENCE_APPLY() ApplyOperatingCodeAndFixedControllerChannels0Through2()
#define SEQUENCE_ATIP_WINDOW() AcquireAndClassifyMediaState6AtipPositionWindow()
#define SEQUENCE_NEAR_TARGET(value) AcquireMediaState6PacketNearTargetAndUpdatePhase(value)
#define SEQUENCE_READ_PATH(value) RunControllerReadPathCalibrationAtTarget(value)
#define SEQUENCE_STROBE(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define SEQUENCE_DERIVE_THRESHOLD() DeriveMediaState6CalibrationThresholdState()
#define SEQUENCE_WRITE_E2(value) WriteControllerParameterE2ValueAndRefreshEE(value)
#define SEQUENCE_POSITION_WINDOW() RunMediaState6ControllerCalibrationPositionWindow()
#define SEQUENCE_RESTORE(value) RestoreControllerReadPathConfiguration(value)
#define SEQUENCE_WRITE_D4_E2(value) WriteControllerParametersD4AndE2ForCode(value)
#define SEQUENCE_RAISE_F4() WriteControllerParameterF4WithUpAdjustment()
#define SEQUENCE_ACQUIRE_SAMPLES() AcquireControllerPositionSamplesAndUpdateCalibration()
#define SEQUENCE_RESTORE_D4_E2() WriteControllerParametersD412AndE2Value58()
#define SEQUENCE_LOWER_F4() WriteControllerParameterF4WithDownAdjustment()
#define SEQUENCE_FIXED_SUCCESS() ReturnMediaState6CalibrationSequenceSuccess()
#define SEQUENCE_EVALUATE_RESULT() EvaluateMediaState6CalibrationResultAndRetry()
#define SEQUENCE_SET_COMPLETION_BIT() SetBit0039Atomic()
#endif
#endif
