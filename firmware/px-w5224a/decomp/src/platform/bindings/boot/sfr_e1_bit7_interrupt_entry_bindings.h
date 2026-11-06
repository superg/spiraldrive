#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define PLEXTOR_INTERRUPT(vector)
#define SfrE1Bit7InterruptBody SfrE1Bit7InterruptEntry
#else
#define PLEXTOR_INTERRUPT(vector) interrupt vector
sfr SfrE1EntryPsw = 0xd0;
sfr SfrE1EntryPsw1 = 0xd1;
#define VectorEntryPsw SfrE1EntryPsw
#define VectorEntryPsw1 SfrE1EntryPsw1
#endif
#endif
