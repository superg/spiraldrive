#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#include <math.h>
#define FLOOR_PROFILE_COORDINATE(value) floor((double)(value))
#else
extern double FloorFloat64(double value);
#define FLOOR_PROFILE_COORDINATE(value) ConvertFloat64ToFloat32(FloorFloat64(ConvertFloat32ToFloat64(value)))
#endif
#endif
