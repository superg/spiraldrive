#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#include <math.h>
extern void ObserveVendorF2SharedCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count);
#define VENDOR_F2_COPY(source, destination, count) ObserveVendorF2SharedCopy((source), (destination), (count))
#define VENDOR_F2_CEIL(value) ceil(value)
#else
extern double CeilFloat64(double value);
#define VENDOR_F2_COPY(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define VENDOR_F2_CEIL(value) CeilFloat64(value)
#endif
#endif
