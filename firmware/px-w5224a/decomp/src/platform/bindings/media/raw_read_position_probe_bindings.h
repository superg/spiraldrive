#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePositionProbeClearBit0014(void);
extern void ObservePositionProbeClearBit0015(void);
extern void ObservePositionProbeClearBit0007(void);
extern u8 ObservePositionProbeReadBit0013(void);
extern u8 ObservePositionProbeReadBit001C(void);
extern u8 ObservePositionProbeReadBit001D(void);
extern void ObservePositionProbeEnableExternalInterrupt0(void);
extern void ObserveRawSectorCaptureClearBit0009(void);
extern void ObservePositionProbeCoroutineInitializeBits(void);
extern u8 ObservePositionProbeCoroutineReadTerminalBit(void);
extern u8 ObservePositionProbeCoroutineStep(void);
extern void ObservePositionProbeCoroutineYield(void);
extern void ObservePositionProbeCoroutineFinalize(void);
extern void ObserveAlternateRawReadCoroutineClearTerminal(void);
extern void ObserveAlternateRawReadCoroutineInitializeSpeed(void);
extern u8 ObserveAlternateRawReadCoroutineStep(void);
extern void ObserveAlternateRawReadCoroutineYield(void);
extern u8 ObserveAlternateRawReadCoroutineReadTerminal(void);
extern void ObserveAlternateRawReadCoroutineFinalize(void);
extern s8 ObserveAlternateRawReadStepCaptureCountdown(void);
extern void ObserveAlternateRawReadStepClearTerminal(void);
extern u8 ObserveAlternateRawReadStepReadTerminal(void);
extern void ObserveAlternateRawReadStepThrottle(void);
extern u8 ObserveAlternateRawReadStepReadSpecialMode(void);
extern u8 ObserveAlternateRawReadStepRequest(s32 base_target, s32 signed_adjustment);
extern void ObserveAlternateRawReadStepInitializeController(void);
extern void ObserveAlternateRawReadStepServiceCountdown(void);
extern void ObserveAlternateRawReadStepYield(void);
extern u8 ObserveAlternateRawReadStepCheckP1(void);
extern void ObserveAlternateRawReadStepBuildDescriptor(u16 descriptor_status, u8 descriptor_mode);
extern void ObserveAlternateRawReadStepEnqueueDescriptor(void);
extern void ObserveAlternateRawReadStepSetTerminal(void);
#define CLEAR_POSITION_PROBE_BIT0014() ObservePositionProbeClearBit0014()
#define CLEAR_POSITION_PROBE_BIT0015() ObservePositionProbeClearBit0015()
#define CLEAR_POSITION_PROBE_BIT0007() ObservePositionProbeClearBit0007()
#define READ_POSITION_PROBE_BIT0013() ObservePositionProbeReadBit0013()
#define READ_POSITION_PROBE_BIT001C() ObservePositionProbeReadBit001C()
#define READ_POSITION_PROBE_BIT001D() ObservePositionProbeReadBit001D()
#define ENABLE_POSITION_PROBE_EXTERNAL_INTERRUPT0() ObservePositionProbeEnableExternalInterrupt0()
#define CLEAR_RAW_SECTOR_CAPTURE_BIT0009() ObserveRawSectorCaptureClearBit0009()
#define INITIALIZE_POSITION_PROBE_COROUTINE_BITS() ObservePositionProbeCoroutineInitializeBits()
#define READ_POSITION_PROBE_COROUTINE_TERMINAL() ObservePositionProbeCoroutineReadTerminalBit()
#define RUN_POSITION_PROBE_COROUTINE_STEP() ObservePositionProbeCoroutineStep()
#define YIELD_POSITION_PROBE_COROUTINE2() ObservePositionProbeCoroutineYield()
#define FINALIZE_POSITION_PROBE_COROUTINE2() ObservePositionProbeCoroutineFinalize()
#define CLEAR_ALTERNATE_RAW_READ_TERMINAL() ObserveAlternateRawReadCoroutineClearTerminal()
#define INITIALIZE_ALTERNATE_RAW_READ_SPEED() ObserveAlternateRawReadCoroutineInitializeSpeed()
#define RUN_ALTERNATE_RAW_READ_STEP() ObserveAlternateRawReadCoroutineStep()
#define YIELD_ALTERNATE_RAW_READ_COROUTINE2() ObserveAlternateRawReadCoroutineYield()
#define READ_ALTERNATE_RAW_READ_TERMINAL() ObserveAlternateRawReadCoroutineReadTerminal()
#define FINALIZE_ALTERNATE_RAW_READ_COROUTINE2() ObserveAlternateRawReadCoroutineFinalize()
#define CAPTURE_ALTERNATE_RAW_READ_STEP_COUNTDOWN() ObserveAlternateRawReadStepCaptureCountdown()
#define CLEAR_ALTERNATE_RAW_READ_STEP_TERMINAL() ObserveAlternateRawReadStepClearTerminal()
#define READ_ALTERNATE_RAW_READ_STEP_TERMINAL() ObserveAlternateRawReadStepReadTerminal()
#define THROTTLE_ALTERNATE_RAW_READ_STEP() ObserveAlternateRawReadStepThrottle()
#define READ_ALTERNATE_RAW_READ_STEP_SPECIAL_MODE() ObserveAlternateRawReadStepReadSpecialMode()
#define REQUEST_ALTERNATE_RAW_READ_STEP(target, adjustment) ObserveAlternateRawReadStepRequest((target), (adjustment))
#define INITIALIZE_ALTERNATE_RAW_READ_STEP_CONTROLLER() ObserveAlternateRawReadStepInitializeController()
#define SERVICE_ALTERNATE_RAW_READ_STEP_COUNTDOWN() ObserveAlternateRawReadStepServiceCountdown()
#define YIELD_ALTERNATE_RAW_READ_STEP() ObserveAlternateRawReadStepYield()
#define CHECK_ALTERNATE_RAW_READ_STEP_P1() ObserveAlternateRawReadStepCheckP1()
#define BUILD_ALTERNATE_RAW_READ_STEP_DESCRIPTOR(status, mode) ObserveAlternateRawReadStepBuildDescriptor((status), (mode))
#define ENQUEUE_ALTERNATE_RAW_READ_STEP_DESCRIPTOR() ObserveAlternateRawReadStepEnqueueDescriptor()
#define SET_ALTERNATE_RAW_READ_STEP_TERMINAL() ObserveAlternateRawReadStepSetTerminal()
#else
extern void PLEXTOR_NEAR_FUNCTION ClearBit0014Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0015Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0007Atomic(void);
extern u8 FirmwareReadBit0013(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit001CAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit001DAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearExternalInterrupt0PendingAndEnableAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0009Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeRawReadPositionProbeBitsAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0009Atomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ProcessRawReadPositionProbeStep(void);
extern void YieldCoroutine2(void);
extern void PLEXTOR_NEAR_FUNCTION FinalizeRawReadPipelineAndIdleCoroutine2(void);
extern void InitializeRawReadSpeedForCurrentDriveState(void);
extern u8 PLEXTOR_NEAR_FUNCTION ProcessAlternateRawReadPipelineStep(void);
extern s8 PLEXTOR_NEAR_FUNCTION CaptureInheritedR14ForAlternateRawReadStep(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0009Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ThrottleRawReadProducerForQueueCapacity(void);
extern u8 RequestReadAcquisitionAndWait(s32 base_target, s32 signed_adjustment);
extern void PLEXTOR_NEAR_FUNCTION InitializeAlternateRawReadControllerCaptureState(void);
extern void PLEXTOR_NEAR_FUNCTION ServiceParsedQDescriptorCountdownAndPositionAdvance(void);
extern u8 CheckP1Bit3StableHighForCurrentMediaState(void);
extern void EnqueueRawSectorDescriptor(void);
#define CLEAR_POSITION_PROBE_BIT0014() ClearBit0014Atomic()
#define CLEAR_POSITION_PROBE_BIT0015() ClearBit0015Atomic()
#define CLEAR_POSITION_PROBE_BIT0007() ClearBit0007Atomic()
#define READ_POSITION_PROBE_BIT0013() FirmwareReadBit0013()
#define READ_POSITION_PROBE_BIT001C() ReadBit001CAtomic()
#define READ_POSITION_PROBE_BIT001D() ReadBit001DAtomic()
#define ENABLE_POSITION_PROBE_EXTERNAL_INTERRUPT0() ClearExternalInterrupt0PendingAndEnableAtomic()
#define CLEAR_RAW_SECTOR_CAPTURE_BIT0009() ClearBit0009Atomic()
#define INITIALIZE_POSITION_PROBE_COROUTINE_BITS() InitializeRawReadPositionProbeBitsAtomic()
#define READ_POSITION_PROBE_COROUTINE_TERMINAL() ReadBit0009Atomic()
#define RUN_POSITION_PROBE_COROUTINE_STEP() ProcessRawReadPositionProbeStep()
#define YIELD_POSITION_PROBE_COROUTINE2() YieldCoroutine2()
#define FINALIZE_POSITION_PROBE_COROUTINE2() FinalizeRawReadPipelineAndIdleCoroutine2()
#define CLEAR_ALTERNATE_RAW_READ_TERMINAL() ClearBit0009Atomic()
#define INITIALIZE_ALTERNATE_RAW_READ_SPEED() InitializeRawReadSpeedForCurrentDriveState()
#define RUN_ALTERNATE_RAW_READ_STEP() ProcessAlternateRawReadPipelineStep()
#define YIELD_ALTERNATE_RAW_READ_COROUTINE2() YieldCoroutine2()
#define READ_ALTERNATE_RAW_READ_TERMINAL() ReadBit0009Atomic()
#define FINALIZE_ALTERNATE_RAW_READ_COROUTINE2() FinalizeRawReadPipelineAndIdleCoroutine2()
#define CAPTURE_ALTERNATE_RAW_READ_STEP_COUNTDOWN() CaptureInheritedR14ForAlternateRawReadStep()
#define CLEAR_ALTERNATE_RAW_READ_STEP_TERMINAL() ClearBit0009Atomic()
#define READ_ALTERNATE_RAW_READ_STEP_TERMINAL() ReadBit0009Atomic()
#define THROTTLE_ALTERNATE_RAW_READ_STEP() ThrottleRawReadProducerForQueueCapacity()
#define READ_ALTERNATE_RAW_READ_STEP_SPECIAL_MODE() FirmwareReadBit0013()
#define REQUEST_ALTERNATE_RAW_READ_STEP(target, adjustment) RequestReadAcquisitionAndWait((target), (adjustment))
#define INITIALIZE_ALTERNATE_RAW_READ_STEP_CONTROLLER() InitializeAlternateRawReadControllerCaptureState()
#define SERVICE_ALTERNATE_RAW_READ_STEP_COUNTDOWN() ServiceParsedQDescriptorCountdownAndPositionAdvance()
#define YIELD_ALTERNATE_RAW_READ_STEP() YieldCoroutine2()
#define CHECK_ALTERNATE_RAW_READ_STEP_P1() CheckP1Bit3StableHighForCurrentMediaState()
#define BUILD_ALTERNATE_RAW_READ_STEP_DESCRIPTOR(status, mode) BuildRawSectorDescriptorScratch((status), (mode))
#define ENQUEUE_ALTERNATE_RAW_READ_STEP_DESCRIPTOR() EnqueueRawSectorDescriptor()
#define SET_ALTERNATE_RAW_READ_STEP_TERMINAL() SetBit0009Atomic()
#endif
#endif
