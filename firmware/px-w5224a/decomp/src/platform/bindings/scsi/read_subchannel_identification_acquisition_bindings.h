#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveClearReadSubChannelCaptureBit(void);
#define CLEAR_READ_SUBCHANNEL_CAPTURE_BIT() ObserveClearReadSubChannelCaptureBit()
#else
extern void PLEXTOR_NEAR_FUNCTION ClearReadSubChannelCaptureBit(void);
#define CLEAR_READ_SUBCHANNEL_CAPTURE_BIT() ClearReadSubChannelCaptureBit()
#endif
#endif
