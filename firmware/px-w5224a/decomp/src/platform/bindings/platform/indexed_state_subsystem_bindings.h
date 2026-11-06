#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveIndexedStateSubsystemResetController(void);
extern void ObserveIndexedStateSubsystemSetControllerFlag(u8 enabled);
extern void ObserveIndexedStateSubsystemWait(u16 duration, u8 yield);
extern void ObserveIndexedStateSubsystemWriteE8E9(u8 value);
extern void ObserveIndexedStateSubsystemInitializeMediaStrategy(void);
extern void ObserveIndexedStateSubsystemBuildProfiles(u16 profile_input);
extern void ObserveIndexedStateSubsystemConfigureProfile(void);
extern void ObserveIndexedStateSubsystemReadConfiguration(u8 *destination, u16 offset, u16 count);
extern void ObserveIndexedStateSubsystemWriteEAEB(u16 value);
extern void ObserveIndexedStateSubsystemYield(void);

#define ResetIndexedStateController() ObserveIndexedStateSubsystemResetController()
#define SetIndexedStateControllerFlag(value) ObserveIndexedStateSubsystemSetControllerFlag(value)
#define WaitIndexedStateDuration(duration, yield) ObserveIndexedStateSubsystemWait((duration), (yield))
#define WriteIndexedStateE8E9(value) ObserveIndexedStateSubsystemWriteE8E9(value)
#define InitializeIndexedStateMediaStrategy() ObserveIndexedStateSubsystemInitializeMediaStrategy()
#define BuildIndexedStateProfiles(value) ObserveIndexedStateSubsystemBuildProfiles(value)
#define ConfigureIndexedStateProfile() ObserveIndexedStateSubsystemConfigureProfile()
#define ReadIndexedStateConfiguration(destination) ObserveIndexedStateSubsystemReadConfiguration((destination), 0x0056u, 2u)
#define WriteIndexedStateEAEB(value) ObserveIndexedStateSubsystemWriteEAEB(value)
#define SetIndexedStateHandshakeBits()                                                                                                                                                                 \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteBit0580(1u);                                                                                                                                                                      \
        FirmwareWriteBit0581(0u);                                                                                                                                                                      \
    } while(0)
#define YieldIndexedStateCoroutine1() ObserveIndexedStateSubsystemYield()
#else
extern void SetP3Bit0Atomic(void);
extern void ClearP3Bit1Atomic(void);
extern void YieldCoroutine1(void);

#define ResetIndexedStateController() ResetControllerDataPathRegisters()
#define SetIndexedStateControllerFlag(value) SetControllerFlagFC07Bit2AndPort8098Bit6(value)
#define WaitIndexedStateDuration(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define WriteIndexedStateE8E9(value) WriteControllerParametersE8E9ForValue(value)
#define InitializeIndexedStateMediaStrategy() InitializeMediaStrategyProfileAndWriteSpeedSelection()
#define BuildIndexedStateProfiles(value) BuildFixedAndGeneratedControllerRegisterProfiles(value)
#define ConfigureIndexedStateProfile() ConfigureControllerRegisterProfileForCurrentMediaState()
#define ReadIndexedStateConfiguration(destination) ReadDriveConfigurationBytes(0x0056ul, (fw_addr_t)(destination), 2u)
#define WriteIndexedStateEAEB(value) WriteControllerParametersEAEBFromScaledValue(value)
#define SetIndexedStateHandshakeBits()                                                                                                                                                                 \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        SetP3Bit0Atomic();                                                                                                                                                                             \
        ClearP3Bit1Atomic();                                                                                                                                                                           \
    } while(0)
#define YieldIndexedStateCoroutine1() YieldCoroutine1()
#endif
#endif
