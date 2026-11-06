#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFirmwareMainEvent(u8 event, u32 argument);
extern u8 ObserveFirmwareMainIndexedState(u8 state);
extern u8 ObserveFirmwareMainConfigurationUpdate(fw_addr_t source, u16 offset, u16 count);
extern u8 ObserveFirmwareMainReadBit0029(void);
extern u8 ObserveFirmwareMainReadBit0046(void);
extern void ObserveFirmwareMainLoopEnd(void);
extern u8 ObserveInitialCoroutineCountdown(void);
extern void ObserveConfigureInitialCoroutine0(u32 entry_point);
extern void ObserveConfigureInitialCoroutine1(u32 entry_point);
extern void ObserveConfigureInitialCoroutine2(u32 entry_point);
extern void ObserveConfigureInitialCoroutine3(u32 entry_point);
extern void ObserveConfigureInitialResumeCoroutine0(void);
extern void ObserveConfigureInitialResumeCoroutine1(void);
#define INITIAL_COROUTINE_COUNTDOWN() ObserveInitialCoroutineCountdown()
#define CONFIGURE_INITIAL_COROUTINE0(entry) ObserveConfigureInitialCoroutine0(entry)
#define CONFIGURE_INITIAL_COROUTINE1(entry) ObserveConfigureInitialCoroutine1(entry)
#define CONFIGURE_INITIAL_COROUTINE2(entry) ObserveConfigureInitialCoroutine2(entry)
#define CONFIGURE_INITIAL_COROUTINE3(entry) ObserveConfigureInitialCoroutine3(entry)
#define CONFIGURE_INITIAL_RESUME0() ObserveConfigureInitialResumeCoroutine0()
#define CONFIGURE_INITIAL_RESUME1() ObserveConfigureInitialResumeCoroutine1()
#define FirmwareMainInitializeDrive() ObserveFirmwareMainEvent(1u, 0ul)
#define FirmwareMainEnableInterrupts() ObserveFirmwareMainEvent(2u, 0ul)
#define FirmwareMainClearP3Bits() ObserveFirmwareMainEvent(3u, 0ul)
#define FirmwareMainConfigureCoroutines() ObserveFirmwareMainEvent(4u, 0ul)
#define FirmwareMainIsIndexedState(state) ObserveFirmwareMainIndexedState(state)
#define FirmwareMainUpdateConfiguration(source, offset, count) ObserveFirmwareMainConfigurationUpdate((source), (offset), (count))
#define FirmwareMainInitializeCoroutine0(entry) ObserveFirmwareMainEvent(5u, entry)
#define FirmwareMainInitializeCoroutine1(entry) ObserveFirmwareMainEvent(6u, entry)
#define FirmwareMainApplyPendingRestart() ObserveFirmwareMainEvent(7u, 0ul)
#define FirmwareMainServiceTransition() ObserveFirmwareMainEvent(8u, 0ul)
#define FirmwareMainDispatchReadRequest() ObserveFirmwareMainEvent(9u, 0ul)
#define FirmwareMainServiceP1Transition() ObserveFirmwareMainEvent(10u, 0ul)
#define FirmwareMainResume0() ObserveFirmwareMainEvent(11u, 0ul)
#define FirmwareMainResume1() ObserveFirmwareMainEvent(12u, 0ul)
#define FirmwareMainResume2() ObserveFirmwareMainEvent(13u, 0ul)
#define FirmwareMainResume3() ObserveFirmwareMainEvent(14u, 0ul)
#define FirmwareMainReadBit0029() ObserveFirmwareMainReadBit0029()
#define FirmwareMainReadBit0046() ObserveFirmwareMainReadBit0046()
#define FirmwareMainLoopEnd() ObserveFirmwareMainLoopEnd()
#else
extern void ResumeCoroutine0(void);
extern void ResumeCoroutine3(void);
extern void EnableFirmwareInterruptSources(void);
extern void ClearP3Bit0Atomic(void);
extern void ClearP3Bit1Atomic(void);
extern u8 IsIndexedDriveStateActive(u8 state);
extern void ApplyPendingDriveInitializationRestart(void);
extern void ServiceP1Bit5AndBit7DriveStateTransition(void);
extern u8 ReadFirmwareMainBit0029(void);
extern u8 ReadFirmwareMainBit0046(void);
#define INITIAL_COROUTINE_COUNTDOWN() FirmwareReadNear8(FW_NEAR_UNKNOWN_1214)
#define CONFIGURE_INITIAL_COROUTINE0(entry) InitializeCoroutine0(entry)
#define CONFIGURE_INITIAL_COROUTINE1(entry) InitializeCoroutine1(entry)
#define CONFIGURE_INITIAL_COROUTINE2(entry) InitializeCoroutine2(entry)
#define CONFIGURE_INITIAL_COROUTINE3(entry) InitializeCoroutine3(entry)
#define CONFIGURE_INITIAL_RESUME0() ResumeCoroutine0()
#define CONFIGURE_INITIAL_RESUME1() ResumeCoroutine1()
#define FirmwareMainInitializeDrive() InitializeDriveFirmware()
#define FirmwareMainEnableInterrupts() EnableFirmwareInterruptSources()
#define FirmwareMainClearP3Bits()                                                                                                                                                                      \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ClearP3Bit0Atomic();                                                                                                                                                                           \
        ClearP3Bit1Atomic();                                                                                                                                                                           \
    } while(0)
#define FirmwareMainConfigureCoroutines() ConfigureInitialCoroutines()
#define FirmwareMainIsIndexedState(state) IsIndexedDriveStateActive(state)
#define FirmwareMainUpdateConfiguration(source, offset, count) UpdateAndProgramDriveConfigurationRange((source), (offset), (count))
#define FirmwareMainInitializeCoroutine0(entry) InitializeCoroutine0(entry)
#define FirmwareMainInitializeCoroutine1(entry) InitializeCoroutine1(entry)
#define FirmwareMainApplyPendingRestart() ApplyPendingDriveInitializationRestart()
#define FirmwareMainServiceTransition() ServiceDriveInitializationStateTransition()
#define FirmwareMainDispatchReadRequest() DispatchReadAcquisitionRequestFlags()
#define FirmwareMainServiceP1Transition() ServiceP1Bit5AndBit7DriveStateTransition()
#define FirmwareMainResume0() ResumeCoroutine0()
#define FirmwareMainResume1() ResumeCoroutine1()
#define FirmwareMainResume2() ResumeCoroutine2()
#define FirmwareMainResume3() ResumeCoroutine3()
#define FirmwareMainReadBit0029() ReadFirmwareMainBit0029()
#define FirmwareMainReadBit0046() ReadFirmwareMainBit0046()
#define FirmwareMainLoopEnd()                                                                                                                                                                          \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
    } while(0)
#endif
#endif
