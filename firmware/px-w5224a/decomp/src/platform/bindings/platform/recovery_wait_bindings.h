#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveInitializeRecoveryCoroutine1(fw_addr_t entry_address);
extern void ObserveSetRecoveryHandshakePinsHigh(void);
extern void ObserveRecoveryDelayTimerUnits59(void);
extern void ObserveSetRecoveryBit0547(void);
extern void ObserveSetRecoveryBit0546(void);
extern void ObserveSetRecoverySfrA9Bit2(void);
extern void ObserveServicePendingHostCommandForRecovery(void);
extern void ObserveYieldRecoveryCoroutine0(void);
extern void ObserveRestartFirmwareForRecovery(void);
extern void ObserveControllerStartupDelayTimerUnits59(void);
extern void ObserveControllerStartupYieldCoroutine0(void);
extern void ObserveControllerStartupRestartFirmware(void);
#define INITIALIZE_RECOVERY_COROUTINE1(address) ObserveInitializeRecoveryCoroutine1(address)
#define SET_RECOVERY_HANDSHAKE_HIGH() ObserveSetRecoveryHandshakePinsHigh()
#define DELAY_RECOVERY_TIMER_UNITS_59() ObserveRecoveryDelayTimerUnits59()
#define SET_RECOVERY_BIT0547() ObserveSetRecoveryBit0547()
#define SET_RECOVERY_BIT0546() ObserveSetRecoveryBit0546()
#define SET_RECOVERY_SFR_A9_BIT2() ObserveSetRecoverySfrA9Bit2()
#define SERVICE_RECOVERY_HOST_COMMAND() ObserveServicePendingHostCommandForRecovery()
#define YIELD_RECOVERY_COROUTINE0() ObserveYieldRecoveryCoroutine0()
#define RESTART_RECOVERY_FIRMWARE() ObserveRestartFirmwareForRecovery()
#define DELAY_CONTROLLER_STARTUP() ObserveControllerStartupDelayTimerUnits59()
#define YIELD_CONTROLLER_STARTUP() ObserveControllerStartupYieldCoroutine0()
#define RESTART_CONTROLLER_STARTUP() ObserveControllerStartupRestartFirmware()
#else
extern void InitializeCoroutine1(fw_addr_t entry_address);
extern void SetP3Bits0And1HighForTimedOutputSequence(void);
extern void DelayTimerUnits59(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
extern void ServicePendingHostCommandIfAny(void);
extern void YieldCoroutine0(void);
extern void RestartFirmware(void);
#define INITIALIZE_RECOVERY_COROUTINE1(address) InitializeCoroutine1(address)
#define SET_RECOVERY_HANDSHAKE_HIGH() SetP3Bits0And1HighForTimedOutputSequence()
#define DELAY_RECOVERY_TIMER_UNITS_59() DelayTimerUnits59()
#define SET_RECOVERY_BIT0547() SetBit0547Atomic()
#define SET_RECOVERY_BIT0546() SetBit0546Atomic()
#define SET_RECOVERY_SFR_A9_BIT2() SetSfrA9Bit2Atomic()
#define SERVICE_RECOVERY_HOST_COMMAND() ServicePendingHostCommandIfAny()
#define YIELD_RECOVERY_COROUTINE0() YieldCoroutine0()
#define RESTART_RECOVERY_FIRMWARE() RestartFirmware()
#define DELAY_CONTROLLER_STARTUP() DelayTimerUnits59()
#if defined(PLEXTOR_LED_HOST_ROUTE_DIAGNOSTICS)
extern void StartupRouteHandshakeYield(void);
#define YIELD_CONTROLLER_STARTUP() StartupRouteHandshakeYield()
#else
#define YIELD_CONTROLLER_STARTUP() YieldCoroutine0()
#endif
#define RESTART_CONTROLLER_STARTUP() RestartFirmware()
#endif
#endif
