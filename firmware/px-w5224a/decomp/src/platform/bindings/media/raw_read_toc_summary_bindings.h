#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawReadTocConvertTuple(u16 tuple_address);
#define RAW_TOC_CONVERT(a) ObserveRawReadTocConvertTuple(a)
#else
#define RAW_TOC_CONVERT(a) ConvertThreeBinaryBytesToPackedBcd(a)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u16 tuple_address = 0x7000u;
#else
u8 tuple[3];
u16 tuple_address = (u16)tuple;
#endif
#endif
