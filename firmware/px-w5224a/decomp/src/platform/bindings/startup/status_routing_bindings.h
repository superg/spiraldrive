#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveStartupStatusYieldCoroutine1(void);
extern void ObserveStartupStatusConfigurationMaintenance(void);
extern u8 ObserveStartupStatusIndexedStateActive(u8 state_code);
extern void ObserveStartupStatusSignalPattern(void);
extern void ObserveStartupStatusInitializeCoroutine0(u32 entry_point);
#define RouteStatusYieldCoroutine1() ObserveStartupStatusYieldCoroutine1()
#define RouteStatusConfigurationMaintenance() ObserveStartupStatusConfigurationMaintenance()
#define RouteStatusIndexedStateActive(state_code) ObserveStartupStatusIndexedStateActive(state_code)
#define RouteStatusSignalPattern() ObserveStartupStatusSignalPattern()
#define RouteStatusInitializeCoroutine0(entry_point) ObserveStartupStatusInitializeCoroutine0(entry_point)
#else
extern void YieldCoroutine1(void);
extern void PLEXTOR_NEAR_FUNCTION ApplyStartupDriveConfigurationMaintenance(void);
extern u8 IsIndexedDriveStateActive(u8 state_code);
extern void SignalDriveStatusPatternForever(void);
extern void InitializeCoroutine0(u32 entry_point);
#define RouteStatusYieldCoroutine1() YieldCoroutine1()
#define RouteStatusConfigurationMaintenance() ApplyStartupDriveConfigurationMaintenance()
#define RouteStatusIndexedStateActive(state_code) IsIndexedDriveStateActive(state_code)
#define RouteStatusSignalPattern() SignalDriveStatusPatternForever()
#define RouteStatusInitializeCoroutine0(entry_point) InitializeCoroutine0(entry_point)
#endif
#endif
