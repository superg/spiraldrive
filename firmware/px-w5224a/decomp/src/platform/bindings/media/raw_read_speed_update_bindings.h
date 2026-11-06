#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawReadSpeedBit(u8 bit_address);
#define READ_SPEED_BIT(address) ObserveRawReadSpeedBit(address)
#else
u8 bdata RawReadSpeedBitByte21 _at_ 0x21;
u8 bdata RawReadSpeedBitByte23 _at_ 0x23;
u8 bdata RawReadSpeedBitByte24 _at_ 0x24;
sbit RawReadSpeedBit000C = RawReadSpeedBitByte21 ^ 4;
sbit RawReadSpeedBit000F = RawReadSpeedBitByte21 ^ 7;
sbit RawReadSpeedBit001B = RawReadSpeedBitByte23 ^ 3;
sbit RawReadSpeedBit001C = RawReadSpeedBitByte23 ^ 4;
sbit RawReadSpeedBit001D = RawReadSpeedBitByte23 ^ 5;
sbit RawReadSpeedBit0021 = RawReadSpeedBitByte24 ^ 1;
#define READ_SPEED_BIT_000C() ((u8)RawReadSpeedBit000C)
#define READ_SPEED_BIT_000F() ((u8)RawReadSpeedBit000F)
#define READ_SPEED_BIT_001B() ((u8)RawReadSpeedBit001B)
#define READ_SPEED_BIT_001C() ((u8)RawReadSpeedBit001C)
#define READ_SPEED_BIT_001D() ((u8)RawReadSpeedBit001D)
#define READ_SPEED_BIT_0021() ((u8)RawReadSpeedBit0021)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define READ_SPEED_BIT_000C() READ_SPEED_BIT(0x0cu)
#define READ_SPEED_BIT_000F() READ_SPEED_BIT(0x0fu)
#define READ_SPEED_BIT_001B() READ_SPEED_BIT(0x1bu)
#define READ_SPEED_BIT_001C() READ_SPEED_BIT(0x1cu)
#define READ_SPEED_BIT_001D() READ_SPEED_BIT(0x1du)
#define READ_SPEED_BIT_0021() READ_SPEED_BIT(0x21u)
#endif
#endif
