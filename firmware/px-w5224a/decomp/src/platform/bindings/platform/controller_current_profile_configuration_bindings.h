#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCurrentProfileClearEa(void);
extern void ObserveCurrentProfileSetEa(void);
extern void ObserveCurrentProfileSetEc(void);
extern void ObserveCurrentProfileSetA9Bit2(void);
extern void ObserveCurrentProfileApplyFixed(u8 operating_code);
extern u32 ObserveCurrentProfileMultiply(u32 factor_a, u32 factor_b);
#define CLEAR_PROFILE_EA() ObserveCurrentProfileClearEa()
#define SET_PROFILE_EA() ObserveCurrentProfileSetEa()
#define SET_PROFILE_EC() ObserveCurrentProfileSetEc()
#define SET_PROFILE_A9_BIT2() ObserveCurrentProfileSetA9Bit2()
#define APPLY_FIXED_PROFILE(code) ObserveCurrentProfileApplyFixed(code)
#define PROFILE_MULTIPLY(left, right) ObserveCurrentProfileMultiply((left), (right))
#else
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
#define CLEAR_PROFILE_EA() ClearBit0547Atomic()
#define SET_PROFILE_EA() SetBit0547Atomic()
#define SET_PROFILE_EC() SetBit0546Atomic()
#define SET_PROFILE_A9_BIT2() SetSfrA9Bit2Atomic()
#define APPLY_FIXED_PROFILE(code) ApplyFixedControllerRegisterProfileForOperatingCode(code)
#define PROFILE_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#endif
#endif
