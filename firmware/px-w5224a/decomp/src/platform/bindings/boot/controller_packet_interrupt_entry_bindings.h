#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define PLEXTOR_INTERRUPT(vector)
#define ControllerPacketInterruptBody ControllerPacketInterruptEntry
#else
#define PLEXTOR_INTERRUPT(vector) interrupt vector
sfr ControllerPacketPsw1 = 0xd1;
#define VectorEntryPsw1 ControllerPacketPsw1
#endif
#endif
