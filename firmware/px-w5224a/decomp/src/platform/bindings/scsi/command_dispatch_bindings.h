#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define HandleWriteAndVerify10Command HandleWriteAndVerify10CommandForTest
#define HandlePlayTrackRelative12Command HandlePlayTrackRelative12CommandForTest
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#undef HandleWriteAndVerify10Command
#undef HandlePlayTrackRelative12Command
#endif
#endif
