#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveMediaReadTransitionResetDriveControl(void);
extern void ObserveMediaReadTransitionRestartCoroutine1(u32 entry_point);
extern void ObserveMediaReadTransitionUpdateP3(u8 restart_pattern);
extern void ObserveMediaReadTransitionYieldCoroutine1(void);
#define RESET_MEDIA_READ_TRANSITION_DRIVE_CONTROL() ObserveMediaReadTransitionResetDriveControl()
#define RESTART_MEDIA_READ_TRANSITION(entry_point) ObserveMediaReadTransitionRestartCoroutine1(entry_point)
#define UPDATE_MEDIA_READ_TRANSITION_P3(restart_pattern) ObserveMediaReadTransitionUpdateP3(restart_pattern)
#define YIELD_MEDIA_READ_TRANSITION() ObserveMediaReadTransitionYieldCoroutine1()
#else
extern void RestartCoroutine1AndYield(u32 entry_point);
extern void YieldCoroutine1(void);
#define RESET_MEDIA_READ_TRANSITION_DRIVE_CONTROL() ResetDriveControlStateToEight()
#define RESTART_MEDIA_READ_TRANSITION(entry_point) RestartCoroutine1AndYield(entry_point)
#define UPDATE_MEDIA_READ_TRANSITION_P3(restart_pattern) UpdateP3Bits0And1PatternState(restart_pattern)
#define YIELD_MEDIA_READ_TRANSITION() YieldCoroutine1()
#endif
#endif
