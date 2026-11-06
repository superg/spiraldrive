#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawSectorCompletionReadBit(u8 bit_address);
extern void ObserveRawSectorCompletionWriteBit(u8 bit_address, u8 value);
#define READ_COMPLETION_BIT(address) ObserveRawSectorCompletionReadBit((address))
#define WRITE_COMPLETION_BIT(address, value) ObserveRawSectorCompletionWriteBit((address), (value))
#else
u8 bdata RawSectorCompletionBitByte20 _at_ 0x20;
u8 bdata RawSectorCompletionBitByte21 _at_ 0x21;
u8 bdata RawSectorCompletionBitByte22 _at_ 0x22;
u8 bdata RawSectorCompletionBitByte23 _at_ 0x23;
u8 bdata RawSectorCompletionBitByte24 _at_ 0x24;
sbit RawSectorCompletionBit0004 = RawSectorCompletionBitByte20 ^ 4;
sbit RawSectorCompletionBit0005 = RawSectorCompletionBitByte20 ^ 5;
sbit RawSectorCompletionBit0006 = RawSectorCompletionBitByte20 ^ 6;
sbit RawSectorCompletionBit0009 = RawSectorCompletionBitByte21 ^ 1;
sbit RawSectorCompletionBit000C = RawSectorCompletionBitByte21 ^ 4;
sbit RawSectorCompletionBit000F = RawSectorCompletionBitByte21 ^ 7;
sbit RawSectorCompletionBit0013 = RawSectorCompletionBitByte22 ^ 3;
sbit RawSectorCompletionBit001B = RawSectorCompletionBitByte23 ^ 3;
sbit RawSectorCompletionBit001C = RawSectorCompletionBitByte23 ^ 4;
sbit RawSectorCompletionBit001D = RawSectorCompletionBitByte23 ^ 5;
sbit RawSectorCompletionBit001F = RawSectorCompletionBitByte23 ^ 7;
sbit RawSectorCompletionBit0020 = RawSectorCompletionBitByte24 ^ 0;
sbit RawSectorCompletionBit0024 = RawSectorCompletionBitByte24 ^ 4;
sbit RawSectorCompletionBit0025 = RawSectorCompletionBitByte24 ^ 5;
sbit RawSectorCompletionBit0027 = RawSectorCompletionBitByte24 ^ 7;
#define READ_COMPLETION_BIT_0006() ((u8)RawSectorCompletionBit0006)
#define READ_COMPLETION_BIT_000C() ((u8)RawSectorCompletionBit000C)
#define READ_COMPLETION_BIT_000F() ((u8)RawSectorCompletionBit000F)
#define READ_COMPLETION_BIT_0013() ((u8)RawSectorCompletionBit0013)
#define READ_COMPLETION_BIT_001B() ((u8)RawSectorCompletionBit001B)
#define READ_COMPLETION_BIT_001C() ((u8)RawSectorCompletionBit001C)
#define READ_COMPLETION_BIT_001D() ((u8)RawSectorCompletionBit001D)
#define READ_COMPLETION_BIT_001F() ((u8)RawSectorCompletionBit001F)
#define READ_COMPLETION_BIT_0020() ((u8)RawSectorCompletionBit0020)
#define READ_COMPLETION_BIT_0027() ((u8)RawSectorCompletionBit0027)
#define WRITE_COMPLETION_BIT_0004(value) (RawSectorCompletionBit0004 = (value))
#define WRITE_COMPLETION_BIT_0005(value) (RawSectorCompletionBit0005 = (value))
#define WRITE_COMPLETION_BIT_0009(value) (RawSectorCompletionBit0009 = (value))
#define WRITE_COMPLETION_BIT_000C(value) (RawSectorCompletionBit000C = (value))
#define WRITE_COMPLETION_BIT_001F(value) (RawSectorCompletionBit001F = (value))
#define WRITE_COMPLETION_BIT_0020(value) (RawSectorCompletionBit0020 = (value))
#define WRITE_COMPLETION_BIT_0024(value) (RawSectorCompletionBit0024 = (value))
#define WRITE_COMPLETION_BIT_0025(value) (RawSectorCompletionBit0025 = (value))
#define WRITE_COMPLETION_BIT_0027(value) (RawSectorCompletionBit0027 = (value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define READ_COMPLETION_BIT_0006() READ_COMPLETION_BIT(0x06u)
#define READ_COMPLETION_BIT_000C() READ_COMPLETION_BIT(0x0cu)
#define READ_COMPLETION_BIT_000F() READ_COMPLETION_BIT(0x0fu)
#define READ_COMPLETION_BIT_0013() READ_COMPLETION_BIT(0x13u)
#define READ_COMPLETION_BIT_001B() READ_COMPLETION_BIT(0x1bu)
#define READ_COMPLETION_BIT_001C() READ_COMPLETION_BIT(0x1cu)
#define READ_COMPLETION_BIT_001D() READ_COMPLETION_BIT(0x1du)
#define READ_COMPLETION_BIT_001F() READ_COMPLETION_BIT(0x1fu)
#define READ_COMPLETION_BIT_0020() READ_COMPLETION_BIT(0x20u)
#define READ_COMPLETION_BIT_0027() READ_COMPLETION_BIT(0x27u)
#define WRITE_COMPLETION_BIT_0004(value) WRITE_COMPLETION_BIT(0x04u, value)
#define WRITE_COMPLETION_BIT_0005(value) WRITE_COMPLETION_BIT(0x05u, value)
#define WRITE_COMPLETION_BIT_0009(value) WRITE_COMPLETION_BIT(0x09u, value)
#define WRITE_COMPLETION_BIT_000C(value) WRITE_COMPLETION_BIT(0x0cu, value)
#define WRITE_COMPLETION_BIT_001F(value) WRITE_COMPLETION_BIT(0x1fu, value)
#define WRITE_COMPLETION_BIT_0020(value) WRITE_COMPLETION_BIT(0x20u, value)
#define WRITE_COMPLETION_BIT_0024(value) WRITE_COMPLETION_BIT(0x24u, value)
#define WRITE_COMPLETION_BIT_0025(value) WRITE_COMPLETION_BIT(0x25u, value)
#define WRITE_COMPLETION_BIT_0027(value) WRITE_COMPLETION_BIT(0x27u, value)
#endif
#endif
