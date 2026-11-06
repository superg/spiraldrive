#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0547(0u);
#else
ConfigurationUpdateGlobalInterruptEnable = 0;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if !defined(PLEXTOR_HOST_TEST)
sbit ConfigurationUpdateGlobalInterruptEnable = 0xaf;
#endif
#endif
