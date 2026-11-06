#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObservePrimaryRawReadOperatingCodeBit(u8 bit_address);
#define READ_OPERATING_CODE_BIT(address) ObservePrimaryRawReadOperatingCodeBit((address))
#define READ_OPERATING_CODE_BIT_000B() READ_OPERATING_CODE_BIT(0x0bu)
#define READ_OPERATING_CODE_BIT_001C() READ_OPERATING_CODE_BIT(0x1cu)
#define READ_OPERATING_CODE_BIT_001D() READ_OPERATING_CODE_BIT(0x1du)
#define READ_OPERATING_CODE_BIT_0020() READ_OPERATING_CODE_BIT(0x20u)
#else
u8 bdata PrimaryRawReadOperatingCodeBitByte21 _at_ 0x21;
u8 bdata PrimaryRawReadOperatingCodeBitByte23 _at_ 0x23;
u8 bdata PrimaryRawReadOperatingCodeBitByte24 _at_ 0x24;
sbit PrimaryRawReadOperatingCodeBit000B = PrimaryRawReadOperatingCodeBitByte21 ^ 3;
sbit PrimaryRawReadOperatingCodeBit001C = PrimaryRawReadOperatingCodeBitByte23 ^ 4;
sbit PrimaryRawReadOperatingCodeBit001D = PrimaryRawReadOperatingCodeBitByte23 ^ 5;
sbit PrimaryRawReadOperatingCodeBit0020 = PrimaryRawReadOperatingCodeBitByte24 ^ 0;
#define READ_OPERATING_CODE_BIT_000B() ((u8)PrimaryRawReadOperatingCodeBit000B)
#define READ_OPERATING_CODE_BIT_001C() ((u8)PrimaryRawReadOperatingCodeBit001C)
#define READ_OPERATING_CODE_BIT_001D() ((u8)PrimaryRawReadOperatingCodeBit001D)
#define READ_OPERATING_CODE_BIT_0020() ((u8)PrimaryRawReadOperatingCodeBit0020)
#endif
#endif
