#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetAllInitialTocAcquisitionState(void);
extern u8 ObserveInitialTocDriveOperationCodeIsEligible(void);
extern void ObserveMarkInitialTocUnavailable(void);
extern u8 ObserveAcquireInitialTocDiscPosition(void);
extern void ObserveApplyInitialTocOperatingCode(void);
extern void ObserveRearmInitialTocQPacketCapture(void);
extern void ObserveArmInitialTocThreeBytePacketCapture(void);
extern u8 ObserveParseInitialTocThreeBytePacket(void);
extern u8 ObserveProcessInitialTocQPacket(void);
extern u8 ObserveResetInitialTocAcquisition(u8 reinitialize_controller, u8 first_point_index);
extern u8 ObserveAccumulateInitialTocQPacket(void);
extern u8 ObserveIsInitialTocComplete(void);
extern u8 ObserveTestInitialTocPulseThreshold(u8 sample_count);
extern void ObserveYieldInitialTocCoroutine(void);
extern void ObserveResetInitialTocDriveControlState(void);
#define RESET_ALL_INITIAL_TOC_STATE() ObserveResetAllInitialTocAcquisitionState()
#define INITIAL_TOC_OPERATION_CODE_IS_ELIGIBLE() ObserveInitialTocDriveOperationCodeIsEligible()
#define MARK_INITIAL_TOC_UNAVAILABLE() ObserveMarkInitialTocUnavailable()
#define ACQUIRE_INITIAL_TOC_POSITION() ObserveAcquireInitialTocDiscPosition()
#define APPLY_INITIAL_TOC_OPERATING_CODE() ObserveApplyInitialTocOperatingCode()
#define REARM_INITIAL_TOC_Q_CAPTURE() ObserveRearmInitialTocQPacketCapture()
#define ARM_INITIAL_TOC_PACKET_CAPTURE() ObserveArmInitialTocThreeBytePacketCapture()
#define PARSE_INITIAL_TOC_PACKET() ObserveParseInitialTocThreeBytePacket()
#define PROCESS_INITIAL_TOC_Q_PACKET() ObserveProcessInitialTocQPacket()
#define RESET_INITIAL_TOC_ACQUISITION(reinitialize_controller) ObserveResetInitialTocAcquisition((reinitialize_controller), 0u)
#define ACCUMULATE_INITIAL_TOC_Q_PACKET() ObserveAccumulateInitialTocQPacket()
#define INITIAL_TOC_IS_COMPLETE() ObserveIsInitialTocComplete()
#define TEST_INITIAL_TOC_PULSE_THRESHOLD(sample_count) ObserveTestInitialTocPulseThreshold(sample_count)
#define YIELD_INITIAL_TOC() ObserveYieldInitialTocCoroutine()
#define RESET_INITIAL_TOC_DRIVE_CONTROL() ObserveResetInitialTocDriveControlState()
extern void ObserveAppendFullTocSession(void);
extern void ObserveFinalizeFullTocLeadOut(void);
extern u8 ObserveAcquireNextFullTocSessionBoundary(void);
extern u8 ObserveAcquireNextFullTocSessionLeadIn(void);
extern u8 ObserveReacquireNextFullTocSessionBoundary(void);
extern void ObserveClearFailedFullTocSessionPoints(void);
extern u8 ObserveAccumulateCurrentSessionTocQPacket(void);
extern u8 ObserveIsCurrentSessionTocComplete(void);
#define APPEND_FULL_TOC_SESSION() ObserveAppendFullTocSession()
#define FINALIZE_FULL_TOC_LEAD_OUT() ObserveFinalizeFullTocLeadOut()
#define ACQUIRE_NEXT_FULL_TOC_BOUNDARY() ObserveAcquireNextFullTocSessionBoundary()
#define ACQUIRE_NEXT_FULL_TOC_LEAD_IN() ObserveAcquireNextFullTocSessionLeadIn()
#define REACQUIRE_NEXT_FULL_TOC_BOUNDARY() ObserveReacquireNextFullTocSessionBoundary()
#define CLEAR_FAILED_FULL_TOC_SESSION_POINTS() ObserveClearFailedFullTocSessionPoints()
#define ACCUMULATE_CURRENT_SESSION_TOC_Q_PACKET() ObserveAccumulateCurrentSessionTocQPacket()
#define CURRENT_SESSION_TOC_IS_COMPLETE() ObserveIsCurrentSessionTocComplete()
#else
extern void PLEXTOR_NEAR_FUNCTION ResetAllTocAcquisitionState(void);
extern u8 IsCurrentDriveOperationCodeInTocAcquisitionSet(void);
extern void PLEXTOR_NEAR_FUNCTION MarkInitialTocUnavailable(void);
extern u8 AcquireDiscReadPosition(void);
extern void ApplyCurrentOperatingCodeToControllerChannels0Through3(void);
extern void RearmControllerQPacketCaptureWithTimeout(void);
extern void ArmControllerThreeBytePacketCapture(void);
extern u8 ParseControllerThreeBytePacket(void);
extern u8 PLEXTOR_NEAR_FUNCTION ResetTocAcquisitionAndOptionallyReinitializeController(u8 reinitialize_controller, u8 first_point_index);
extern u8 PLEXTOR_NEAR_FUNCTION IsInitialDiscTocComplete(void);
extern u8 TestP1Cex0PulseSuccessThreshold(u8 sample_count);
extern void YieldCoroutine1(void);
extern void ResetDriveControlStateToEight(void);
#define RESET_ALL_INITIAL_TOC_STATE() ResetAllTocAcquisitionState()
#define INITIAL_TOC_OPERATION_CODE_IS_ELIGIBLE() IsCurrentDriveOperationCodeInTocAcquisitionSet()
#define MARK_INITIAL_TOC_UNAVAILABLE() MarkInitialTocUnavailable()
#define ACQUIRE_INITIAL_TOC_POSITION() AcquireDiscReadPosition()
#define APPLY_INITIAL_TOC_OPERATING_CODE() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define REARM_INITIAL_TOC_Q_CAPTURE() RearmControllerQPacketCaptureWithTimeout()
#define ARM_INITIAL_TOC_PACKET_CAPTURE() ArmControllerThreeBytePacketCapture()
#define PARSE_INITIAL_TOC_PACKET() ParseControllerThreeBytePacket()
#define PROCESS_INITIAL_TOC_Q_PACKET() ProcessCapturedControllerQPacket()
#define RESET_INITIAL_TOC_ACQUISITION(reinitialize_controller) ResetTocAcquisitionAndOptionallyReinitializeController((reinitialize_controller), 0u)
#define ACCUMULATE_INITIAL_TOC_Q_PACKET() AccumulateCurrentQPacketIntoInitialToc()
#define INITIAL_TOC_IS_COMPLETE() IsInitialDiscTocComplete()
#define TEST_INITIAL_TOC_PULSE_THRESHOLD(sample_count) TestP1Cex0PulseSuccessThreshold(sample_count)
#define YIELD_INITIAL_TOC() YieldCoroutine1()
#define RESET_INITIAL_TOC_DRIVE_CONTROL() ResetDriveControlStateToEight()
extern void PLEXTOR_NEAR_FUNCTION AppendFullTocSessionFromDecodedTocState(void);
extern void PLEXTOR_NEAR_FUNCTION FinalizeDecodedTocLeadOutRecord(void);
extern u8 PLEXTOR_NEAR_FUNCTION AcquireNextSessionBoundaryPosition(void);
extern u8 PLEXTOR_NEAR_FUNCTION AcquireNextSessionLeadInToc(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReacquireNextSessionBoundaryPosition(void);
extern void PLEXTOR_NEAR_FUNCTION ClearDecodedTocPointSummariesFromLastTrack(void);
extern u8 PLEXTOR_NEAR_FUNCTION AccumulateCurrentQPacketIntoSessionToc(void);
extern u8 PLEXTOR_NEAR_FUNCTION IsCurrentSessionTocComplete(void);
#define APPEND_FULL_TOC_SESSION() AppendFullTocSessionFromDecodedTocState()
#define FINALIZE_FULL_TOC_LEAD_OUT() FinalizeDecodedTocLeadOutRecord()
#define ACQUIRE_NEXT_FULL_TOC_BOUNDARY() AcquireNextSessionBoundaryPosition()
#define ACQUIRE_NEXT_FULL_TOC_LEAD_IN() AcquireNextSessionLeadInToc()
#define REACQUIRE_NEXT_FULL_TOC_BOUNDARY() ReacquireNextSessionBoundaryPosition()
#define CLEAR_FAILED_FULL_TOC_SESSION_POINTS() ClearDecodedTocPointSummariesFromLastTrack()
#define ACCUMULATE_CURRENT_SESSION_TOC_Q_PACKET() AccumulateCurrentQPacketIntoSessionToc()
#define CURRENT_SESSION_TOC_IS_COMPLETE() IsCurrentSessionTocComplete()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveOuterDiscIndexedStateActive(u8 state_code);
extern u8 ObserveOuterDiscAcquirePosition(void);
extern u8 ObserveOuterDiscResetController(void);
extern void ObserveOuterDiscSetOperatingEnable(u8 enabled);
extern u8 ObserveOuterDiscCheckTolerance(u8 restart_timeout);
extern void ObserveOuterDiscConfigureDisabled(void);
extern void ObserveOuterDiscConfigureEnabled(u8 mode_selector);
extern void ObserveOuterDiscApplyOperatingCode(void);
extern void ObserveOuterDiscRearmQCapture(void);
extern u8 ObserveOuterDiscProcessQPacket(void);
extern void ObserveOuterDiscArmPacketCapture(void);
extern u8 ObserveOuterDiscParsePacket(void);
extern void ObserveOuterDiscYield(void);
extern void ObserveOuterDiscWritePort(fw_addr_t port_address, u8 value);
extern u8 ObserveOuterDiscReadPort(fw_addr_t port_address);
#define OUTER_DISC_INDEXED_STATE_ACTIVE(state_code) ObserveOuterDiscIndexedStateActive(state_code)
#define OUTER_DISC_ACQUIRE_POSITION() ObserveOuterDiscAcquirePosition()
#define OUTER_DISC_RESET_CONTROLLER() ObserveOuterDiscResetController()
#define OUTER_DISC_SET_OPERATING_ENABLE(enabled) ObserveOuterDiscSetOperatingEnable(enabled)
#define OUTER_DISC_CHECK_TOLERANCE(restart_timeout) ObserveOuterDiscCheckTolerance(restart_timeout)
#define OUTER_DISC_CONFIGURE_DISABLED() ObserveOuterDiscConfigureDisabled()
#define OUTER_DISC_CONFIGURE_ENABLED(mode_selector) ObserveOuterDiscConfigureEnabled(mode_selector)
#define OUTER_DISC_APPLY_OPERATING_CODE() ObserveOuterDiscApplyOperatingCode()
#define OUTER_DISC_REARM_Q_CAPTURE() ObserveOuterDiscRearmQCapture()
#define OUTER_DISC_PROCESS_Q_PACKET() ObserveOuterDiscProcessQPacket()
#define OUTER_DISC_ARM_PACKET_CAPTURE() ObserveOuterDiscArmPacketCapture()
#define OUTER_DISC_PARSE_PACKET() ObserveOuterDiscParsePacket()
#define OUTER_DISC_YIELD() ObserveOuterDiscYield()
#define OUTER_DISC_WRITE_PORT(port_address, value) ObserveOuterDiscWritePort((port_address), (value))
#define OUTER_DISC_READ_PORT(port_address) ObserveOuterDiscReadPort(port_address)
#else
extern u8 IsIndexedDriveStateActive(u8 state_code);
extern u8 AcquireDiscReadPosition(void);
extern u8 ResetDriveControlAndInitializeControllerCalibration(void);
extern void SetControllerOperatingEnableState(u8 enabled);
extern u8 CheckControllerMeasurementToleranceOrTimeout(u8 restart_timeout);
extern void ConfigureControllerPortsForDisabledOperatingPath(void);
extern void ConfigureControllerPortsAndParametersForCurrentOperatingCode(u8 mode_selector);
extern void ApplyOperatingCodeAndFixedControllerChannels0Through2(void);
extern void RearmControllerQPacketCaptureWithTimeout(void);
extern void ArmControllerThreeBytePacketCapture(void);
extern u8 ParseControllerThreeBytePacket(void);
extern void YieldCoroutine1(void);
extern void WaitControllerPortReadyAndWriteByte(fw_addr_t port_address, u8 value);
extern u8 WaitControllerPortReadyAndReadByte(fw_addr_t port_address);
#define OUTER_DISC_INDEXED_STATE_ACTIVE(state_code) IsIndexedDriveStateActive(state_code)
#define OUTER_DISC_ACQUIRE_POSITION() AcquireDiscReadPosition()
#define OUTER_DISC_RESET_CONTROLLER() ResetDriveControlAndInitializeControllerCalibration()
#define OUTER_DISC_SET_OPERATING_ENABLE(enabled) SetControllerOperatingEnableState(enabled)
#define OUTER_DISC_CHECK_TOLERANCE(restart_timeout) CheckControllerMeasurementToleranceOrTimeout(restart_timeout)
#define OUTER_DISC_CONFIGURE_DISABLED() ConfigureControllerPortsForDisabledOperatingPath()
#define OUTER_DISC_CONFIGURE_ENABLED(mode_selector) ConfigureControllerPortsAndParametersForCurrentOperatingCode(mode_selector)
#define OUTER_DISC_APPLY_OPERATING_CODE() ApplyOperatingCodeAndFixedControllerChannels0Through2()
#define OUTER_DISC_REARM_Q_CAPTURE() RearmControllerQPacketCaptureWithTimeout()
#define OUTER_DISC_PROCESS_Q_PACKET() ProcessCapturedControllerQPacket()
#define OUTER_DISC_ARM_PACKET_CAPTURE() ArmControllerThreeBytePacketCapture()
#define OUTER_DISC_PARSE_PACKET() ParseControllerThreeBytePacket()
#define OUTER_DISC_YIELD() YieldCoroutine1()
#define OUTER_DISC_WRITE_PORT(port_address, value) WaitControllerPortReadyAndWriteByte((port_address), (value))
#define OUTER_DISC_READ_PORT(port_address) WaitControllerPortReadyAndReadByte(port_address)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateDiscRecognitionSnapshot(void);
extern u8 ObserveInitializeDiscRecognitionPostTocState(void);
extern u8 ObserveInitializeDiscRecognitionControllerAndAtip(void);
extern u8 ObserveAcquireDiscRecognitionInitialToc(void);
extern u8 ObserveCalibrateDiscRecognitionRadialModel(void);
extern u8 ObserveBuildDiscRecognitionFullToc(void);
extern u8 ObserveAcquireDiscRecognitionPma(void);
extern void ObserveProbeDiscRecognitionOuterDisc(void);
extern void ObserveFinalizeDiscRecognitionPostTocState(void);
extern void ObserveConstrainDiscRecognitionReadSpeed(void);
extern void ObserveApplyDiscRecognitionReadSpeed(void);
#define VALIDATE_DISC_SNAPSHOT() ObserveValidateDiscRecognitionSnapshot()
#define INITIALIZE_DISC_POST_TOC() ObserveInitializeDiscRecognitionPostTocState()
#define INITIALIZE_DISC_CONTROLLER_ATIP() ObserveInitializeDiscRecognitionControllerAndAtip()
#define ACQUIRE_DISC_INITIAL_TOC() ObserveAcquireDiscRecognitionInitialToc()
#define CALIBRATE_DISC_RADIAL_MODEL() ObserveCalibrateDiscRecognitionRadialModel()
#define BUILD_DISC_FULL_TOC() ObserveBuildDiscRecognitionFullToc()
#define ACQUIRE_DISC_PMA() ObserveAcquireDiscRecognitionPma()
#define PROBE_DISC_OUTER_LIMIT() ObserveProbeDiscRecognitionOuterDisc()
#define FINALIZE_DISC_POST_TOC() ObserveFinalizeDiscRecognitionPostTocState()
#define CONSTRAIN_DISC_READ_SPEED() ObserveConstrainDiscRecognitionReadSpeed()
#define APPLY_DISC_READ_SPEED() ObserveApplyDiscRecognitionReadSpeed()
#else
extern u8 ValidateRetainedWriteBufferSnapshotChecksum(void);
extern u8 PLEXTOR_NEAR_FUNCTION InitializePostTocControllerState(void);
extern u8 InitializeControllerAndAcquireAtipState(void);
extern u8 PLEXTOR_NEAR_FUNCTION AcquireInitialDiscTocState(void);
extern u8 CalibrateDiscRadialPositionModel(void);
extern u8 PLEXTOR_NEAR_FUNCTION BuildFullTocForAvailableSessions(void);
extern u8 AcquirePmaQRecordState(void);
extern void PLEXTOR_NEAR_FUNCTION ProbeOuterDiscAndDetermineReadSpeedLimit(void);
extern void PLEXTOR_NEAR_FUNCTION FinalizePostTocDriveState(void);
extern void ConstrainDiscReadSpeedLimitFromCalibration(void);
extern void ApplyRequestedAndDiscReadSpeedLimits(void);
#define VALIDATE_DISC_SNAPSHOT() ValidateRetainedWriteBufferSnapshotChecksum()
#define INITIALIZE_DISC_POST_TOC() InitializePostTocControllerState()
#define INITIALIZE_DISC_CONTROLLER_ATIP() InitializeControllerAndAcquireAtipState()
#define ACQUIRE_DISC_INITIAL_TOC() AcquireInitialDiscTocState()
#define CALIBRATE_DISC_RADIAL_MODEL() CalibrateDiscRadialPositionModel()
#define BUILD_DISC_FULL_TOC() BuildFullTocForAvailableSessions()
#define ACQUIRE_DISC_PMA() AcquirePmaQRecordState()
#define PROBE_DISC_OUTER_LIMIT() ProbeOuterDiscAndDetermineReadSpeedLimit()
#define FINALIZE_DISC_POST_TOC() FinalizePostTocDriveState()
#define CONSTRAIN_DISC_READ_SPEED() ConstrainDiscReadSpeedLimitFromCalibration()
#define APPLY_DISC_READ_SPEED() ApplyRequestedAndDiscReadSpeedLimits()
#endif
#endif
