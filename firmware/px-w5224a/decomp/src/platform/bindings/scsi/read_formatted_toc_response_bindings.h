#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
ConvertFramePositionToBinaryMsfTuple((s32)ReadFormattedTocDword(0x002ba58dul), destination + 1u);
#else
ConvertFramePositionToBinaryMsfTuple((s32)ReadFormattedTocDword(0x002ba58dul), (fw_addr_t)tuple);
FirmwareWrite8(destination + 1u, tuple[0]);
FirmwareWrite8(destination + 2u, tuple[1]);
FirmwareWrite8(destination + 3u, tuple[2]);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if !defined(PLEXTOR_HOST_TEST)
u8 tuple[3];
#endif
#endif
