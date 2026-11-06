#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveMediaState5CalibrationOrdering(void);
extern u8 ObserveMediaState5FourSampleSelection(void);
extern u8 ObserveMediaState5WindowSelection(void);
extern u8 ObserveMediaState5EncodeOffset(s8 value);
#define CLASSIFY_MEDIA_STATE5_ORDERING() ObserveMediaState5CalibrationOrdering()
#define SELECT_MEDIA_STATE5_FOUR_SAMPLES() ObserveMediaState5FourSampleSelection()
#define SELECT_MEDIA_STATE5_WINDOW() ObserveMediaState5WindowSelection()
#define ENCODE_MEDIA_STATE5_OFFSET(value) ObserveMediaState5EncodeOffset(value)
#else
#define CLASSIFY_MEDIA_STATE5_ORDERING() ClassifyMediaCalibrationSampleOrderingAndPublishCode()
#define SELECT_MEDIA_STATE5_FOUR_SAMPLES() SelectControllerCalibrationCandidateFromFourSamples()
#define SELECT_MEDIA_STATE5_WINDOW() SelectControllerCalibrationValueFromSampleWindow()
#define ENCODE_MEDIA_STATE5_OFFSET(value) EncodeSignedByteAsSignMagnitude(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRunControllerParameterCalibrationSet(void);
extern void ObserveControllerReferenceCaptureWrite(fw_addr_t address, u8 value);
extern u8 ObserveControllerReferenceCaptureWait10(void);
extern void ObserveControllerReferenceCaptureSubmit(u16 value, u16 selector);
extern u16 ObserveControllerReferenceCaptureRead(u16 selector);
#define RUN_CONTROLLER_PARAMETER_CALIBRATION_SET() ObserveRunControllerParameterCalibrationSet()
#define WRITE_CONTROLLER_REFERENCE_PORT(address, value) ObserveControllerReferenceCaptureWrite((address), (value))
#define WAIT_CONTROLLER_REFERENCE_READY_10() ObserveControllerReferenceCaptureWait10()
#define SUBMIT_CONTROLLER_REFERENCE_WORD(value, selector) ObserveControllerReferenceCaptureSubmit((value), (selector))
#define READ_CONTROLLER_REFERENCE_WORD(selector) ObserveControllerReferenceCaptureRead(selector)
#define ControllerReferenceCaptureFunction RunControllerParameterCalibrationAndReferenceCaptureForTest
#else
#define RUN_CONTROLLER_PARAMETER_CALIBRATION_SET() CalibrateControllerParameterSet()
#define WRITE_CONTROLLER_REFERENCE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define WAIT_CONTROLLER_REFERENCE_READY_10() WaitControllerPort80F8Bit7Clear10AndYield()
#define SUBMIT_CONTROLLER_REFERENCE_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define READ_CONTROLLER_REFERENCE_WORD(selector) ReadControllerWord(selector)
#define ControllerReferenceCaptureFunction RunControllerParameterCalibrationAndReferenceCapture
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern s8 ObserveDivideS8ByTwoForCalibrationSelector(s8 dividend);
#define DIVIDE_CALIBRATION_BYTE_BY_TWO(dividend) ObserveDivideS8ByTwoForCalibrationSelector(dividend)
#else
extern s8 PLEXTOR_NEAR_FUNCTION DivideS8ByTwoNative(s8 dividend);
#define DIVIDE_CALIBRATION_BYTE_BY_TWO(dividend) DivideS8ByTwoNative(dividend)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveIsIndexedDriveStateActiveForParameterFC(u8 state);
extern void ObserveWriteControllerPortForParameterFC(fw_addr_t port_address, u8 value);
extern u8 ObserveReadControllerPortForParameterFC(fw_addr_t port_address);
extern void ObserveResetIndexedControllerChannelForParameterFC(u8 channel);
extern void ObserveEnterControllerMeasurementForParameterFC(void);
extern u8 ObserveCalibrateControllerParameterFC(void);
extern void ObserveLeaveControllerMeasurementForParameterFC(void);
extern void ObserveWaitTimer1ForParameterFC(u16 duration_units, u8 yield_while_waiting);
extern void ObserveSelectAndWriteParameterFCForOperatingCode(u8 operating_code);
#define IS_INDEXED_STATE_ACTIVE(state) ObserveIsIndexedDriveStateActiveForParameterFC(state)
#define WRITE_CONTROLLER_PORT(address, value) ObserveWriteControllerPortForParameterFC((address), (value))
#define READ_CONTROLLER_PORT(address) ObserveReadControllerPortForParameterFC(address)
#define RESET_INDEXED_CHANNEL(channel) ObserveResetIndexedControllerChannelForParameterFC(channel)
#define ENTER_MEASUREMENT_CONFIGURATION() ObserveEnterControllerMeasurementForParameterFC()
#define CALIBRATE_PARAMETER_FC() ObserveCalibrateControllerParameterFC()
#define LEAVE_MEASUREMENT_CONFIGURATION() ObserveLeaveControllerMeasurementForParameterFC()
#define WAIT_TIMER1(duration, yield) ObserveWaitTimer1ForParameterFC((duration), (yield))
#define SELECT_AND_WRITE_PARAMETER_FC(code) ObserveSelectAndWriteParameterFCForOperatingCode(code)
#else
extern u8 CalibrateControllerParameterFC(void);
#define IS_INDEXED_STATE_ACTIVE(state) IsIndexedDriveStateActive(state)
#define WRITE_CONTROLLER_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define READ_CONTROLLER_PORT(address) WaitControllerPortReadyAndReadByte(address)
#define RESET_INDEXED_CHANNEL(channel) ResetIndexedControllerChannel(channel)
#define ENTER_MEASUREMENT_CONFIGURATION() EnterControllerMeasurementConfiguration()
#define CALIBRATE_PARAMETER_FC() CalibrateControllerParameterFC()
#define LEAVE_MEASUREMENT_CONFIGURATION() LeaveControllerMeasurementConfiguration()
#define WAIT_TIMER1(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define SELECT_AND_WRITE_PARAMETER_FC(code) SelectAndWriteControllerParameterFCForOperatingCode(code)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWriteControllerParametersForReadPathCalibration(void);
extern void ObserveApplyOperatingCodeForReadPathCalibration(void);
extern u8 ObserveWaitForReadPathCalibrationTarget(s32 target_frame_position);
extern void ObserveConfigureControllerForReadPathCalibration(u8 enabled);
extern void ObserveResetControllerDataPathForReadPathCalibration(void);
extern void ObserveConfigureControllerProfileForReadPathCalibration(void);
extern u8 ObserveReadPathCalibrationReadinessProbe(void);
extern u8 ObserveReadPathCalibrationOperatingWindow(u8 enabled);
extern void ObserveRestoreReadPathCalibrationConfiguration(u8 enabled);
#define WRITE_READ_PATH_PARAMETERS() ObserveWriteControllerParametersForReadPathCalibration()
#define APPLY_READ_PATH_OPERATING_CODE() ObserveApplyOperatingCodeForReadPathCalibration()
#define WAIT_FOR_READ_PATH_TARGET(target) ObserveWaitForReadPathCalibrationTarget(target)
#define CONFIGURE_READ_PATH(enabled) ObserveConfigureControllerForReadPathCalibration(enabled)
#define RESET_READ_PATH_DATA_REGISTERS() ObserveResetControllerDataPathForReadPathCalibration()
#define CONFIGURE_READ_PATH_PROFILE() ObserveConfigureControllerProfileForReadPathCalibration()
#define PROBE_READ_PATH_READINESS() ObserveReadPathCalibrationReadinessProbe()
#define TEST_READ_PATH_OPERATING_WINDOW(enabled) ObserveReadPathCalibrationOperatingWindow(enabled)
#define RESTORE_READ_PATH_CONFIGURATION(enabled) ObserveRestoreReadPathCalibrationConfiguration(enabled)
#else
extern void ConfigureControllerRegisterProfileForCurrentMediaState(void);
extern u8 IsControllerReadPathWithinOperatingWindow(u8 enabled);
#define WRITE_READ_PATH_PARAMETERS() WriteControllerParametersFCFDF6F7FromCurrentState()
#define APPLY_READ_PATH_OPERATING_CODE() ApplyOperatingCodeAndFixedControllerChannels0Through2()
#define WAIT_FOR_READ_PATH_TARGET(target) WaitForDecodedFramePositionPastTarget(target)
#define CONFIGURE_READ_PATH(enabled) ConfigureControllerPortsAndParametersForCurrentOperatingCode(enabled)
#define RESET_READ_PATH_DATA_REGISTERS() ResetControllerDataPathRegisters()
#define CONFIGURE_READ_PATH_PROFILE() ConfigureControllerRegisterProfileForCurrentMediaState()
#define PROBE_READ_PATH_READINESS() RunControllerReadPathReadinessProbe()
#define TEST_READ_PATH_OPERATING_WINDOW(enabled) IsControllerReadPathWithinOperatingWindow(enabled)
#define RESTORE_READ_PATH_CONFIGURATION(enabled) RestoreControllerReadPathConfiguration(enabled)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCurrentFrameProbeStrobe(fw_addr_t address, u8 value);
extern void ObserveCurrentFrameProbeWait(s32 target_frame_position);
extern u8 ObserveCurrentFrameProbeSample(void);
extern void ObserveCurrentFrameProbeYield(void);
#define CURRENT_FRAME_PROBE_STROBE(address, value) ObserveCurrentFrameProbeStrobe((address), (value))
#define CURRENT_FRAME_PROBE_WAIT(target) ObserveCurrentFrameProbeWait(target)
#define CURRENT_FRAME_PROBE_SAMPLE() ObserveCurrentFrameProbeSample()
#define CURRENT_FRAME_PROBE_YIELD() ObserveCurrentFrameProbeYield()
#else
extern void YieldCoroutine1(void);
#define CURRENT_FRAME_PROBE_STROBE(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define CURRENT_FRAME_PROBE_WAIT(target) WaitForDecodedFramePositionPastTarget(target)
#define CURRENT_FRAME_PROBE_SAMPLE() SampleControllerPort80C0Bit3WithHandshake()
#define CURRENT_FRAME_PROBE_YIELD() YieldCoroutine1()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveStartControllerParameterEaEbCalibrationState(void);
extern void ObserveStopControllerParameterEaEbCalibrationState(void);
extern void ObserveWriteControllerParameterEaEbCalibrationValue(u16 value);
extern void ObserveFinalizeControllerParameterEaEbCalibrationPhaseFive(void);
#define START_EA_EB_CALIBRATION_STATE() ObserveStartControllerParameterEaEbCalibrationState()
#define STOP_EA_EB_CALIBRATION_STATE() ObserveStopControllerParameterEaEbCalibrationState()
#define WRITE_EA_EB_CALIBRATION_VALUE(value) ObserveWriteControllerParameterEaEbCalibrationValue(value)
#define FINALIZE_EA_EB_CALIBRATION_PHASE_FIVE() ObserveFinalizeControllerParameterEaEbCalibrationPhaseFive()
#else
#define START_EA_EB_CALIBRATION_STATE() StartControllerParameterEaEbCalibrationCapture()
#define STOP_EA_EB_CALIBRATION_STATE() StopControllerParameterEaEbCalibrationCapture()
#define WRITE_EA_EB_CALIBRATION_VALUE(value) WriteControllerParametersEAEBFromScaledValue(value)
#define FINALIZE_EA_EB_CALIBRATION_PHASE_FIVE() UpdateControllerTimingValueUnlessFlag4Set()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 8
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveControllerParameterD1ModeBits(void);
extern void ObserveControllerPorts80D0D1D3Configuration(void);
extern void ObserveComputeAndRampControllerWord0255Target(void);
extern s16 DispatchSelectControllerWord0255TargetForRamp(void);
#define APPLY_CONTROLLER_D1_FOR_CALIBRATION ObserveControllerParameterD1ModeBits
#define CONFIGURE_CONTROLLER_D0_D1_D3_FOR_CALIBRATION ObserveControllerPorts80D0D1D3Configuration
#define COMPUTE_AND_RAMP_CONTROLLER_WORD0255 ObserveComputeAndRampControllerWord0255Target
#define SELECT_CONTROLLER_WORD0255_TARGET DispatchSelectControllerWord0255TargetForRamp
#else
#define APPLY_CONTROLLER_D1_FOR_CALIBRATION ApplyControllerParameterD1ModeBits
#define CONFIGURE_CONTROLLER_D0_D1_D3_FOR_CALIBRATION ConfigureControllerPorts80D0D1D3ForCurrentState
#define COMPUTE_AND_RAMP_CONTROLLER_WORD0255 ComputeAndRampControllerWord0255Target
#define SELECT_CONTROLLER_WORD0255_TARGET SelectControllerWord0255TargetForCurrentState
#endif
#endif
