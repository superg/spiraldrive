#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveControllerProgressTransitionInvocation(u8 profile_index, u8 progress_bucket, u8 transition_mode);
extern void ObserveControllerProgressTransitionWriteProfile(u8 profile_index);
extern void ObserveControllerProgressTransitionYield(void);
extern void ObserveControllerProgressTransitionWait(u16 duration_units, u8 yield_while_waiting);
#define BYPASS_TRANSITION(profile, bucket, mode) ObserveControllerProgressTransitionInvocation((profile), (bucket), (mode))
#define WRITE_PROFILE(profile) ObserveControllerProgressTransitionWriteProfile(profile)
#define YIELD_TRANSITION() ObserveControllerProgressTransitionYield()
#define WAIT_TRANSITION(duration, yield) ObserveControllerProgressTransitionWait((duration), (yield))
#else
extern void WriteControllerPorts8090Through8097FromProfile(u8 profile_index);
extern void YieldCoroutine1(void);
extern void WaitTimer1ScaledDuration(u16 duration_units, u8 yield_while_waiting);
#define BYPASS_TRANSITION(profile, bucket, mode) 0u
#define WRITE_PROFILE(profile) WriteControllerPorts8090Through8097FromProfile(profile)
#define YIELD_TRANSITION() YieldCoroutine1()
#define WAIT_TRANSITION(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#endif
#endif
