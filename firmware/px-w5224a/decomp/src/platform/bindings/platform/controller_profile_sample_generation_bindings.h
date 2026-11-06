#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s8 ObserveInterpolateControllerProfileSample(float profile_value, u8 sample_index);
extern s8 ObserveLookupControllerProfileAuxiliaryValue(float profile_value, u8 auxiliary_index);
#define INTERPOLATE_PROFILE_SAMPLE(value, index) ObserveInterpolateControllerProfileSample((value), (index))
#define LOOKUP_PROFILE_AUXILIARY(value, index) ObserveLookupControllerProfileAuxiliaryValue((value), (index))
#else
#define INTERPOLATE_PROFILE_SAMPLE(value, index) InterpolateControllerProfileSampleByIndex((value), (index))
#define LOOKUP_PROFILE_AUXILIARY(value, index) LookupControllerProfileAuxiliaryValue((value), (index))
#endif
#endif
