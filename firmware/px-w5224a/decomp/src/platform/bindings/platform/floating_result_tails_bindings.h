#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern double ObserveNegateFloat64ForCeil(double value);
extern double ObserveFloorFloat64ForCeil(double value);
#define NEGATE_FLOAT64_FOR_CEIL(value) ObserveNegateFloat64ForCeil(value)
#define FLOOR_FLOAT64_FOR_CEIL(value) ObserveFloorFloat64ForCeil(value)
#else
extern double NegateFloat64(double value);
extern double FloorFloat64(double value);
#define NEGATE_FLOAT64_FOR_CEIL(value) NegateFloat64(value)
#define FLOOR_FLOAT64_FOR_CEIL(value) FloorFloat64(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
result->bytes[7u - index] = byte;
#else
result->bytes[index] = byte;
#endif
#endif
