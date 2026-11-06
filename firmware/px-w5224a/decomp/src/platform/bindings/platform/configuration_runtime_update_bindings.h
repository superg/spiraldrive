#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveStartupConfigurationByteRangeProgram(fw_addr_t source, u16 destination_offset, u16 byte_count);
extern u8 ObserveStartupConfigurationRangeUpdate(fw_addr_t source, u16 destination_offset, u16 byte_count);
extern void ObserveStartupDriveControlState10Sequence(void);
extern void ObserveStartupRestartCoroutine1AndYield(u32 entry_point);
extern void ObserveStartupRuntimeRange6CUpdate(void);
#define DispatchStartupConfigurationByteRangeProgram(source, offset, count) ObserveStartupConfigurationByteRangeProgram((source), (offset), (count))
#define DispatchStartupConfigurationRangeUpdate(source, offset, count) ObserveStartupConfigurationRangeUpdate((source), (offset), (count))
#define DispatchStartupDriveControlState10Sequence() ObserveStartupDriveControlState10Sequence()
#define DispatchStartupRestartCoroutine1AndYield(entry_point) ObserveStartupRestartCoroutine1AndYield(entry_point)
#define DispatchStartupRuntimeRange6CUpdate() ObserveStartupRuntimeRange6CUpdate()
#else
extern void RestartCoroutine1AndYield(u32 entry_point);
#define DispatchStartupConfigurationByteRangeProgram(source, offset, count) UpdateDriveConfigurationByteRangeAndProgram((source), (offset), (count))
#define DispatchStartupConfigurationRangeUpdate(source, offset, count) UpdateAndProgramDriveConfigurationRange((source), (offset), (count))
#define DispatchStartupDriveControlState10Sequence() RunDriveControlState10Sequence()
#define DispatchStartupRestartCoroutine1AndYield(entry_point) RestartCoroutine1AndYield(entry_point)
#define DispatchStartupRuntimeRange6CUpdate() UpdateDriveConfigurationRange6CFromRuntimeState()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDriveInitCoroutineStep(u8 step, u32 argument);
extern u8 ObserveDriveInitCoroutineSequenceResult(void);
#define ClearDriveInitBit000E() FirmwareWriteBit000E(0u)
#define DriveInitConfigureRing(value) ObserveDriveInitCoroutineStep(1u, value)
#define DriveInitRegisters() ObserveDriveInitCoroutineStep(2u, 0ul)
#define DriveInitClearPorts() ObserveDriveInitCoroutineStep(3u, 0ul)
#define DriveInitAbortController() ObserveDriveInitCoroutineStep(4u, 0ul)
#define DriveInitResetChannel2() ObserveDriveInitCoroutineStep(5u, 0ul)
#define DriveInitYield() ObserveDriveInitCoroutineStep(6u, 0ul)
#define DriveInitRestart(entry) ObserveDriveInitCoroutineStep(7u, entry)
#define DriveInitCoroutine3(entry) ObserveDriveInitCoroutineStep(8u, entry)
#define DriveInitSequenceResult() ObserveDriveInitCoroutineSequenceResult()
#else
extern void ConfigureMainDataRingGeometry(u8 geometry);
extern void InitializeReadAcquisitionControllerRegisters(void);
extern void ClearControllerPorts80E4Through80E6And849D(void);
extern void AbortControllerCommandAndClearWords032A033A(void);
extern void ResetControllerChannel2(void);
extern u8 InitializeDriveControlSequenceReturningAccumulator(void);
extern void YieldCoroutine1(void);
extern void InitializeCoroutine3(u32 entry_point);
extern void PLEXTOR_NEAR_FUNCTION ClearBit000EForReadRequestAtomic(void);
#define ClearDriveInitBit000E() ClearBit000EForReadRequestAtomic()
#define DriveInitConfigureRing(value) ConfigureMainDataRingGeometry(value)
#define DriveInitRegisters() InitializeReadAcquisitionControllerRegisters()
#define DriveInitClearPorts() ClearControllerPorts80E4Through80E6And849D()
#define DriveInitAbortController() AbortControllerCommandAndClearWords032A033A()
#define DriveInitResetChannel2() ResetControllerChannel2()
#define DriveInitYield() YieldCoroutine1()
#define DriveInitRestart(entry) RestartCoroutine1AndYield(entry)
#define DriveInitCoroutine3(entry) InitializeCoroutine3(entry)
#define DriveInitSequenceResult() InitializeDriveControlSequenceReturningAccumulator()

#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveStartupSequenceClearBit0017(void);
extern void ObserveStartupSequenceInitializeMediaState(void);
extern void ObserveStartupSequenceSetP3PinsHigh(void);
extern void ObserveStartupSequenceEnsureStateFlag11EF(void);
extern void ObserveStartupSequenceSetP1TimerPinsLow(void);
extern void ObserveStartupSequenceWriteControllerParameterD6Value7E(void);
extern void ObserveStartupSequenceDebouncedInputs(void);
extern void ObserveStartupSequenceSetP1TimerPinsHigh(void);
extern void ObserveStartupSequenceWriteControllerParameterD6Value00(void);
extern void ObserveStartupSequenceRestartCoroutine1AndYield(u32 entry_point);
#define StartupSequenceClearBit0017() ObserveStartupSequenceClearBit0017()
#define StartupSequenceInitializeMediaState() ObserveStartupSequenceInitializeMediaState()
#define StartupSequenceSetP3PinsHigh() ObserveStartupSequenceSetP3PinsHigh()
#define StartupSequenceEnsureStateFlag11EF() ObserveStartupSequenceEnsureStateFlag11EF()
#define StartupSequenceSetP1TimerPinsLow() ObserveStartupSequenceSetP1TimerPinsLow()
#define StartupSequenceWriteD6Value7E() ObserveStartupSequenceWriteControllerParameterD6Value7E()
#define StartupSequenceDebouncedInputs() ObserveStartupSequenceDebouncedInputs()
#define StartupSequenceSetP1TimerPinsHigh() ObserveStartupSequenceSetP1TimerPinsHigh()
#define StartupSequenceWriteD6Value00() ObserveStartupSequenceWriteControllerParameterD6Value00()
#define StartupSequenceRestart(entry_point) ObserveStartupSequenceRestartCoroutine1AndYield(entry_point)
#else
extern void PLEXTOR_NEAR_FUNCTION InitializeStartupMediaState(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0017ForStartupSequenceAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetP3PinsHighForStartupSequenceAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetP1TimerPinsLowForStartupSequenceAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetP1TimerPinsHighForStartupSequenceAtomic(void);
#define StartupSequenceClearBit0017() ClearBit0017ForStartupSequenceAtomic()
#define StartupSequenceInitializeMediaState() InitializeStartupMediaState()
#define StartupSequenceSetP3PinsHigh() SetP3PinsHighForStartupSequenceAtomic()
#define StartupSequenceEnsureStateFlag11EF() EnsureStateFlag11EFBit3Initialized()
#define StartupSequenceSetP1TimerPinsLow() SetP1TimerPinsLowForStartupSequenceAtomic()
#define StartupSequenceWriteD6Value7E() WriteControllerParameterD6Value7E()
#define StartupSequenceDebouncedInputs() RunStartupDebouncedInputSequence()
#define StartupSequenceSetP1TimerPinsHigh() SetP1TimerPinsHighForStartupSequenceAtomic()
#define StartupSequenceWriteD6Value00() WriteControllerParameterD6Value00()
#define StartupSequenceRestart(entry_point) RestartCoroutine1AndYield(entry_point)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveDriveInitCompletionState9Result(void);
extern void ObserveDriveInitCompletionSetP3PinsHigh(void);
extern u8 ObserveDriveInitCompletionIndexedStateActive(u8 state);
extern s8 ObserveDriveInitCompletionCountdown(void);
extern void ObserveDriveInitCompletionYieldCoroutine1(void);
extern void ObserveDriveInitCompletionRestartCoroutine1(u32 entry_point);
#define DriveInitCompletionState9Result() ObserveDriveInitCompletionState9Result()
#define DriveInitCompletionSetP3PinsHigh() ObserveDriveInitCompletionSetP3PinsHigh()
#define DriveInitCompletionIndexedStateActive(state) ObserveDriveInitCompletionIndexedStateActive(state)
#define DriveInitCompletionCountdown() ObserveDriveInitCompletionCountdown()
#define DriveInitCompletionYield() ObserveDriveInitCompletionYieldCoroutine1()
#define DriveInitCompletionRestart(entry_point) ObserveDriveInitCompletionRestartCoroutine1(entry_point)
#else
extern u8 RunDriveControlState9SequenceWrapper(void);
extern u8 IsIndexedDriveStateActive(u8 state);
extern void YieldCoroutine1(void);
extern void PLEXTOR_NEAR_FUNCTION SetP3PinsHighForDriveInitializationCompletionAtomic(void);
#define DriveInitCompletionState9Result() RunDriveControlState9SequenceWrapper()
#define DriveInitCompletionSetP3PinsHigh() SetP3PinsHighForDriveInitializationCompletionAtomic()
#define DriveInitCompletionIndexedStateActive(state) IsIndexedDriveStateActive(state)
#define DriveInitCompletionCountdown() ((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A))
#define DriveInitCompletionYield() YieldCoroutine1()
#define DriveInitCompletionRestart(entry_point) RestartCoroutine1AndYield(entry_point)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveProgramDriveConfigurationRange6C(const u8 *image, u16 byte_count);

static void ReadConfigurationRange6C(u8 *image)
{
    u8 index;

    for(index = 0u; index < 18u; ++index)
        image[index] = FirmwareRead8(PLEXTOR_RUNTIME_MONITOR_ADDRESS(0x00ffa06cul) + index);
}

#define ProgramConfigurationRange6C(image) ObserveProgramDriveConfigurationRange6C((image), 18u)
#else
#define ReadConfigurationRange6C(image) ReadDriveConfigurationBytes(0x006cul, (fw_addr_t)(image), 18u)
#define ProgramConfigurationRange6C(image) UpdateDriveConfigurationByteRangeAndProgram((fw_addr_t)(image), 0x006cu, 18u)
#endif
#endif
