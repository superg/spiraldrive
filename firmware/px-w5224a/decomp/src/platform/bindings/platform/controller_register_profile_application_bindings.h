#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveControllerRegisterProfileClearEa(void);
extern void ObserveControllerRegisterProfileSetEa(void);
extern void ObserveControllerRegisterProfileSetEc(void);
extern void ObserveControllerRegisterProfileSetA9Bit2(void);
extern void ObserveApplyFixedControllerRegisterProfile(u8 operating_code);
extern void ObserveUpdateControllerProfileDerivedValue(void);
extern void ObserveWriteControllerProfileE8Value(u8 value);
#define CLEAR_PROFILE_EA() ObserveControllerRegisterProfileClearEa()
#define SET_PROFILE_EA() ObserveControllerRegisterProfileSetEa()
#define SET_PROFILE_EC() ObserveControllerRegisterProfileSetEc()
#define SET_PROFILE_A9_BIT2() ObserveControllerRegisterProfileSetA9Bit2()
#define APPLY_FIXED_PROFILE(code) ObserveApplyFixedControllerRegisterProfile(code)
#define UPDATE_PROFILE_DERIVED_VALUE() ObserveUpdateControllerProfileDerivedValue()
#define WRITE_PROFILE_E8_VALUE(value) ObserveWriteControllerProfileE8Value(value)
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
#define UPDATE_PROFILE_DERIVED_VALUE() UpdateControllerProfileDerivedValue()
#define WRITE_PROFILE_E8_VALUE(value) WriteControllerParameterE8Value(value)
#endif
#endif
