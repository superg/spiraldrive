#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#include <math.h>
#define FLOOR_PROFILE_VALUE(value) floor((double)(value))
#else
extern double FloorFloat64(double value);
#define FLOOR_PROFILE_VALUE(value) FloorFloat64((double)(value))
#endif
#endif
