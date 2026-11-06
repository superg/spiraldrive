#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR9ForControllerProfileSelector(void);
extern void ObserveControllerProfileSelectorClearEa(void);
extern void ObserveControllerProfileSelectorSetEa(void);
extern void ObserveControllerProfileSelectorSetEc(void);
extern void ObserveControllerProfileSelectorSetA9Bit2(void);
#define CAPTURE_AUXILIARY_INDEX() ObserveCaptureInheritedR9ForControllerProfileSelector()
#define CLEAR_PROFILE_EA() ObserveControllerProfileSelectorClearEa()
#define SET_PROFILE_EA() ObserveControllerProfileSelectorSetEa()
#define SET_PROFILE_EC() ObserveControllerProfileSelectorSetEc()
#define SET_PROFILE_A9_BIT2() ObserveControllerProfileSelectorSetA9Bit2()
#else
extern u8 PLEXTOR_NEAR_FUNCTION CaptureInheritedR9ForControllerProfileSelector(void);
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
#define CAPTURE_AUXILIARY_INDEX() CaptureInheritedR9ForControllerProfileSelector()
#define CLEAR_PROFILE_EA() ClearBit0547Atomic()
#define SET_PROFILE_EA() SetBit0547Atomic()
#define SET_PROFILE_EC() SetBit0546Atomic()
#define SET_PROFILE_A9_BIT2() SetSfrA9Bit2Atomic()
#endif
#endif
