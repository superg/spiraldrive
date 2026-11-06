#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define PLEXTOR_INTERRUPT(vector)
#define ControllerEventInterruptBody ControllerEventInterruptEntry
u8 VectorEntryPsw;
u8 VectorEntryPsw1;
#else
#define PLEXTOR_INTERRUPT(vector) interrupt vector
sfr ControllerEventPsw1 = 0xd1;
#define VectorEntryPsw1 ControllerEventPsw1
#endif
#endif
