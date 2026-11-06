#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveStartupStatusCoroutineResetDrive(void);
extern void ObserveStartupStatusCoroutineWriteD6(void);
extern void ObserveStartupStatusCoroutineWriteP1(u8 bit0, u8 bit1);
extern void ObserveStartupStatusCoroutineWriteP3(u8 bit0, u8 bit1);
extern void ObserveStartupStatusCoroutinePersist(void);
extern void ObserveStartupStatusCoroutinePattern(u8 restart_pattern);
extern void ObserveStartupStatusCoroutineApplyConfiguration(void);
extern void ObserveStartupStatusCoroutineYield(void);
#define STATUS_COROUTINE_RESET_DRIVE() ObserveStartupStatusCoroutineResetDrive()
#define STATUS_COROUTINE_WRITE_D6() ObserveStartupStatusCoroutineWriteD6()
#define STATUS_COROUTINE_WRITE_P1(bit0, bit1) ObserveStartupStatusCoroutineWriteP1((bit0), (bit1))
#define STATUS_COROUTINE_WRITE_P3(bit0, bit1) ObserveStartupStatusCoroutineWriteP3((bit0), (bit1))
#define STATUS_COROUTINE_PERSIST() ObserveStartupStatusCoroutinePersist()
#define STATUS_COROUTINE_PATTERN(value) ObserveStartupStatusCoroutinePattern(value)
#define STATUS_COROUTINE_APPLY_CONFIGURATION() ObserveStartupStatusCoroutineApplyConfiguration()
#define STATUS_COROUTINE_YIELD() ObserveStartupStatusCoroutineYield()
#else
#include "platform/controller_parameters.h"
#include "platform/drive_control_reset.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#include "platform/timer_debounce.h"
extern void YieldCoroutine1(void);
sbit StartupStatusCoroutineP1Bit0 = 0x90 ^ 0;
sbit StartupStatusCoroutineP1Bit1 = 0x90 ^ 1;
sbit StartupStatusCoroutineP3Bit0 = 0xb0 ^ 0;
sbit StartupStatusCoroutineP3Bit1 = 0xb0 ^ 1;
#define STATUS_COROUTINE_RESET_DRIVE() ResetDriveControlStateToEight()
#define STATUS_COROUTINE_WRITE_D6() WriteControllerParameterD6Value7E()
#define STATUS_COROUTINE_WRITE_P1(bit0, bit1)                                                                                                                                                          \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        StartupStatusCoroutineP1Bit0 = (bit0);                                                                                                                                                         \
        StartupStatusCoroutineP1Bit1 = (bit1);                                                                                                                                                         \
    } while(0)
#define STATUS_COROUTINE_WRITE_P3(bit0, bit1)                                                                                                                                                          \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        StartupStatusCoroutineP3Bit0 = (bit0);                                                                                                                                                         \
        StartupStatusCoroutineP3Bit1 = (bit1);                                                                                                                                                         \
    } while(0)
#define STATUS_COROUTINE_PERSIST() PersistControllerOperatingStatusByte()
#define STATUS_COROUTINE_PATTERN(value) UpdateP3Bits0And1PatternState(value)
#define STATUS_COROUTINE_APPLY_CONFIGURATION() ApplyConfigurationFlagE750Bit0ToState1303Bit0()
#define STATUS_COROUTINE_YIELD() YieldCoroutine1()
#endif
#endif
