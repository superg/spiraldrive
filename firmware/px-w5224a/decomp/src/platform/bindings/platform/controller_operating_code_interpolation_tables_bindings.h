#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern float ObserveOperatingInterpolationNativeStackResidue(void);
extern u16 ObserveOperatingInterpolationMeasurementBound(float value);
extern u16 ObserveOperatingInterpolationAdjustment(float value, u16 reference);
#define UPDATE_MEASUREMENT_BOUND(value) ObserveOperatingInterpolationMeasurementBound(value)
#define COMPUTE_ADJUSTMENT(value, reference) ObserveOperatingInterpolationAdjustment((value), (reference))
#else
#define UPDATE_MEASUREMENT_BOUND(value) UpdateOperatingCodeMeasurementBounds(value)
#define COMPUTE_ADJUSTMENT(value, reference) ComputeBoundedControllerAdjustmentCode((value), (reference))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
union
{
    float value;
    u32 bits;
} encoded;

encoded.value = value;
FirmwareWrite8(address, (u8)(encoded.bits >> 24));
FirmwareWrite8(address + 1u, (u8)(encoded.bits >> 16));
FirmwareWrite8(address + 2u, (u8)(encoded.bits >> 8));
FirmwareWrite8(address + 3u, (u8)encoded.bits);
#else
*((volatile float huge *)address) = value;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
if(low_code != 0x14u)
    low_value = ObserveOperatingInterpolationNativeStackResidue();
#endif
#endif
