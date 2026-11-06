#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u16 ObserveUpdateOperatingCode14MeasurementBound(float value);
extern u16 ObserveComputeOperatingCode14BoundedAdjustment(float value, u16 reference_word);
#define UPDATE_MEASUREMENT_BOUND(value) ObserveUpdateOperatingCode14MeasurementBound(value)
#define COMPUTE_BOUNDED_ADJUSTMENT(value, reference) ObserveComputeOperatingCode14BoundedAdjustment((value), (reference))
#else
#define UPDATE_MEASUREMENT_BOUND(value) UpdateOperatingCodeMeasurementBounds(value)
#define COMPUTE_BOUNDED_ADJUSTMENT(value, reference) ComputeBoundedControllerAdjustmentCode((value), (reference))
#endif
#endif
