#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define ReadSfrD1Bit5ForMsfAdjustment() ((u8)((FirmwareReadDirectSfr8(D1) & 0x20u) != 0u))
#else
extern u8 PLEXTOR_NEAR_FUNCTION CaptureR3ForOneHotByteMapping(void);
extern u8 PLEXTOR_NEAR_FUNCTION PublishR7ForOneHotByteMapping(u8 value);
extern u8 ReadSfrD1Bit5ForMsfAdjustment(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if !defined(PLEXTOR_HOST_TEST)
one_hot_value = CaptureR3ForOneHotByteMapping();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if !defined(PLEXTOR_HOST_TEST)
result = PublishR7ForOneHotByteMapping(result);
#endif
#endif
