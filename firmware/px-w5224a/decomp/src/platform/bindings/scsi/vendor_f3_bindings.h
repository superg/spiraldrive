#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveVendorF3ClearBit000B(void);
#define CLEAR_VENDOR_F3_BIT_000B() ObserveVendorF3ClearBit000B()
#else
u8 bdata VendorF3BitByte21 _at_ 0x21;
sbit VendorF3Bit000B = VendorF3BitByte21 ^ 3;
#define CLEAR_VENDOR_F3_BIT_000B() (VendorF3Bit000B = 0)
#endif
#endif
