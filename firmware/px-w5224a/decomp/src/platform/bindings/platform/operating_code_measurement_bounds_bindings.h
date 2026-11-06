#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#include <math.h>
#define CEIL_OPERATING_BOUND(value) ceil(value)
static u16 ReadOperatingBoundWord(void)
{
    return (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_F074);
}
#define READ_OPERATING_BOUND_WORD() ReadOperatingBoundWord()
#else
extern double CeilFloat64(double value);
#define CEIL_OPERATING_BOUND(value) CeilFloat64(value)
#define READ_OPERATING_BOUND_WORD() (*((volatile u16 huge *)FW_NEAR_UNKNOWN_F074))
#endif
#endif
