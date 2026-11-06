#include "operating_code_measurement_bounds.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/operating_code_measurement_bounds_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f97d05 */
u16 UpdateOperatingCodeMeasurementBounds(float measurement_value)
{
    double adjusted = (double)measurement_value + 4.6592;
    float narrowed = (float)adjusted;
    s32 rounded = (s32)CEIL_OPERATING_BOUND((double)narrowed / 0.0043);
    u16 result = (u16)((u16)rounded + READ_OPERATING_BOUND_WORD());
    u16 limit = (u16)(READ_OPERATING_BOUND_WORD() + 0x1100u);

    if(result > limit)
        result = (u16)(READ_OPERATING_BOUND_WORD() + 0x1100u);
    return result;
}
