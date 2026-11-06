#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#include <math.h>
#define FLOOR_GRID_VALUE(value) floor((double)(value))
#else
extern double FloorFloat64(double value);
#define FLOOR_GRID_VALUE(value) FloorFloat64((double)(value))
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
FirmwareWrite8(address + 1ul, (u8)(encoded.bits >> 16));
FirmwareWrite8(address + 2ul, (u8)(encoded.bits >> 8));
FirmwareWrite8(address + 3ul, (u8)encoded.bits);
#else
*((volatile float huge *)address) = value;
#endif
#endif
