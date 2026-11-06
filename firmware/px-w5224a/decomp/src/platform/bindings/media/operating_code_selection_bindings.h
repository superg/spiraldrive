#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveOperatingCodeBoundedSelection(u8 selector, u8 upper_bound);
extern u8 ObserveOperatingCodeTransitionConstraint(u8 inherited_seed);
extern void ObserveOperatingCodeWriteE8E9(u8 value);
extern void ObserveOperatingCodeConfigureController(u8 reload_profile);
extern void ObserveOperatingCodeWaitForSettling(void);
extern void ObserveOperatingCodeFinalizeTransition(s32 transition_value);

#define ApplyBoundedOperatingCode(selector, bound) ObserveOperatingCodeBoundedSelection((selector), (bound))
#define ApplyOperatingCodeConstraint(seed) ObserveOperatingCodeTransitionConstraint(seed)
#define ApplyOperatingCodeE8E9(value) ObserveOperatingCodeWriteE8E9(value)
#define ApplyOperatingCodeController(reload) ObserveOperatingCodeConfigureController(reload)
#define ApplyOperatingCodeSettlingWait() ObserveOperatingCodeWaitForSettling()
#define ApplyOperatingCodeFinalization(value) ObserveOperatingCodeFinalizeTransition(value)
#else
extern void SetCurrentOperatingCodeFromBoundedSelector(u8 selector, u8 upper_bound);
extern u8 ConstrainCurrentOperatingCodeForTransition(u8 inherited_seed);
extern void WriteControllerParametersE8E9ForValue(u8 value);
extern void ConfigureControllerForCurrentOperatingCode(u8 reload_profile);
extern void PLEXTOR_NEAR_FUNCTION WaitForOperatingCodeTransitionSettling(void);

#define ApplyBoundedOperatingCode(selector, bound) SetCurrentOperatingCodeFromBoundedSelector((selector), (bound))
#define ApplyOperatingCodeConstraint(seed) ConstrainCurrentOperatingCodeForTransition(seed)
#define ApplyOperatingCodeE8E9(value) WriteControllerParametersE8E9ForValue(value)
#define ApplyOperatingCodeController(reload) ConfigureControllerForCurrentOperatingCode(reload)
#define ApplyOperatingCodeSettlingWait() WaitForOperatingCodeTransitionSettling()
#define ApplyOperatingCodeFinalization(value) FinalizeControllerOperatingCodeTransition(value)
#endif
#endif
