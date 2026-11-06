#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveProgramControllerRegisterProfileSelector(u8 profile_selector);
#define PROGRAM_CONTROLLER_PROFILE_SELECTOR(profile_selector) ObserveProgramControllerRegisterProfileSelector(profile_selector)
#else
#define PROGRAM_CONTROLLER_PROFILE_SELECTOR(profile_selector) ProgramControllerRegisterProfileSelector(profile_selector)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFixedControllerProfileClearEa(void);
extern void ObserveFixedControllerProfileSetEa(void);
extern void ObserveFixedControllerProfileSetEc(void);
extern void ObserveFixedControllerProfileSetA9Bit2(void);
#define CLEAR_FIXED_PROFILE_EA() ObserveFixedControllerProfileClearEa()
#define SET_FIXED_PROFILE_EA() ObserveFixedControllerProfileSetEa()
#define SET_FIXED_PROFILE_EC() ObserveFixedControllerProfileSetEc()
#define SET_FIXED_PROFILE_A9_BIT2() ObserveFixedControllerProfileSetA9Bit2()
#else
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
#define CLEAR_FIXED_PROFILE_EA() ClearBit0547Atomic()
#define SET_FIXED_PROFILE_EA() SetBit0547Atomic()
#define SET_FIXED_PROFILE_EC() SetBit0546Atomic()
#define SET_FIXED_PROFILE_A9_BIT2() SetSfrA9Bit2Atomic()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
#define READ_CONTROLLER_PROFILE_WORD(address) ((u16)(((u16)FirmwareRead8(address) << 8) | FirmwareRead8((address) + 1u)))
#else
#define READ_CONTROLLER_PROFILE_WORD(address) (*(volatile u16 huge *)(address))
#endif
#endif
