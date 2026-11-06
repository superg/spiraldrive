#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if !defined(PLEXTOR_HOST_TEST)
extern u8 CaptureR3ForFourByteBitConsensusState(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if !defined(PLEXTOR_HOST_TEST)
new_value = CaptureR3ForFourByteBitConsensusState();
#endif
#endif
