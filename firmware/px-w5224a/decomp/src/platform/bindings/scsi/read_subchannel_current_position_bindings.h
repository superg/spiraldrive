#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadSubChannelCaptureBit(void);
#define READ_SUBCHANNEL_CAPTURE_BIT() ObserveReadSubChannelCaptureBit()
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadSubChannelCaptureBit(void);
#define READ_SUBCHANNEL_CAPTURE_BIT() ReadSubChannelCaptureBit()
#endif
#endif
