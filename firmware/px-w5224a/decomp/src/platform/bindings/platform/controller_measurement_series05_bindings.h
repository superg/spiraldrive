#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern double ObserveControllerMeasurementCeil(double value);
#define CONTROLLER_MEASUREMENT_CEIL(value) ObserveControllerMeasurementCeil(value)
#else
extern double CeilFloat64(double value);
#define CONTROLLER_MEASUREMENT_CEIL(value) CeilFloat64(value)
#endif
#endif
