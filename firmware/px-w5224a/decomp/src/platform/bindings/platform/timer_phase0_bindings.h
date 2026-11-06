#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define READ_P1() FirmwareReadDirectSfr8(90)
#else
sfr TimerPhase0P1 = 0x90;
#define READ_P1() TimerPhase0P1
#endif
#endif
