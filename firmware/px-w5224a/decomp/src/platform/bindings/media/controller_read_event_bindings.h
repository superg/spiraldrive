#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveControllerReadEventReadBit(u8 bit_address);
extern void ObserveControllerReadEventWriteBit(u8 bit_address, u8 value);
#define READ_CONTROLLER_EVENT_BIT(address) ObserveControllerReadEventReadBit((address))
#define WRITE_CONTROLLER_EVENT_BIT(address, value) ObserveControllerReadEventWriteBit((address), (value))
#else
u8 bdata ControllerReadEventBitByte20 _at_ 0x20;
u8 bdata ControllerReadEventBitByte21 _at_ 0x21;
u8 bdata ControllerReadEventBitByte22 _at_ 0x22;
u8 bdata ControllerReadEventBitByte23 _at_ 0x23;
sbit ControllerReadEventBit0004 = ControllerReadEventBitByte20 ^ 4;
sbit ControllerReadEventBit0006 = ControllerReadEventBitByte20 ^ 6;
sbit ControllerReadEventBit000C = ControllerReadEventBitByte21 ^ 4;
sbit ControllerReadEventBit000E = ControllerReadEventBitByte21 ^ 6;
sbit ControllerReadEventBit0013 = ControllerReadEventBitByte22 ^ 3;
sbit ControllerReadEventBit0015 = ControllerReadEventBitByte22 ^ 5;
sbit ControllerReadEventBit001B = ControllerReadEventBitByte23 ^ 3;
#define READ_CONTROLLER_EVENT_BIT_0006() ((u8)ControllerReadEventBit0006)
#define READ_CONTROLLER_EVENT_BIT_000C() ((u8)ControllerReadEventBit000C)
#define READ_CONTROLLER_EVENT_BIT_000E() ((u8)ControllerReadEventBit000E)
#define READ_CONTROLLER_EVENT_BIT_0013() ((u8)ControllerReadEventBit0013)
#define READ_CONTROLLER_EVENT_BIT_0015() ((u8)ControllerReadEventBit0015)
#define READ_CONTROLLER_EVENT_BIT_001B() ((u8)ControllerReadEventBit001B)
#define WRITE_CONTROLLER_EVENT_BIT_0004(value) (ControllerReadEventBit0004 = (value))
#define WRITE_CONTROLLER_EVENT_BIT_0006(value) (ControllerReadEventBit0006 = (value))
#define WRITE_CONTROLLER_EVENT_BIT_0015(value) (ControllerReadEventBit0015 = (value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define READ_CONTROLLER_EVENT_BIT_0006() READ_CONTROLLER_EVENT_BIT(0x06u)
#define READ_CONTROLLER_EVENT_BIT_000C() READ_CONTROLLER_EVENT_BIT(0x0cu)
#define READ_CONTROLLER_EVENT_BIT_000E() READ_CONTROLLER_EVENT_BIT(0x0eu)
#define READ_CONTROLLER_EVENT_BIT_0013() READ_CONTROLLER_EVENT_BIT(0x13u)
#define READ_CONTROLLER_EVENT_BIT_0015() READ_CONTROLLER_EVENT_BIT(0x15u)
#define READ_CONTROLLER_EVENT_BIT_001B() READ_CONTROLLER_EVENT_BIT(0x1bu)
#define WRITE_CONTROLLER_EVENT_BIT_0004(value) WRITE_CONTROLLER_EVENT_BIT(0x04u, value)
#define WRITE_CONTROLLER_EVENT_BIT_0006(value) WRITE_CONTROLLER_EVENT_BIT(0x06u, value)
#define WRITE_CONTROLLER_EVENT_BIT_0015(value) WRITE_CONTROLLER_EVENT_BIT(0x15u, value)
#endif
#endif
