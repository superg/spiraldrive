#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#include <math.h>
#define RADIAL_SQRT(value) ((float)sqrt((double)(value)))
#else
extern float SquareRootFloat32ViaFloat64(float value);
#define RADIAL_SQRT(value) SquareRootFloat32ViaFloat64(value)
#endif
#endif
