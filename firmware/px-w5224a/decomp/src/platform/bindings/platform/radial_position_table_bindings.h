#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#include <math.h>
#define RADIAL_TABLE_SQRT(value) sqrt(value)
#else
extern double SquareRootFloat64(double value);
#define RADIAL_TABLE_SQRT(value) SquareRootFloat64(value)
#endif
#endif
