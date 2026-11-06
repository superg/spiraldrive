#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObservePrimaryProducerPositionReadBit(u8 bit_address);
extern void ObservePrimaryProducerPositionWriteBit(u8 bit_address, u8 value);
#define READ_PRODUCER_BIT(address) ObservePrimaryProducerPositionReadBit((address))
#define WRITE_PRODUCER_BIT(address, value) ObservePrimaryProducerPositionWriteBit((address), (value))
#else
u8 bdata PrimaryProducerPositionBitByte23 _at_ 0x23;
u8 bdata PrimaryProducerPositionBitByte24 _at_ 0x24;
sbit PrimaryProducerPositionBit001B = PrimaryProducerPositionBitByte23 ^ 3;
sbit PrimaryProducerPositionBit0020 = PrimaryProducerPositionBitByte24 ^ 0;
sbit PrimaryProducerPositionBit0026 = PrimaryProducerPositionBitByte24 ^ 6;
#define READ_PRODUCER_BIT_001B() ((u8)PrimaryProducerPositionBit001B)
#define READ_PRODUCER_BIT_0020() ((u8)PrimaryProducerPositionBit0020)
#define READ_PRODUCER_BIT_0026() ((u8)PrimaryProducerPositionBit0026)
#define WRITE_PRODUCER_BIT_0026(value) (PrimaryProducerPositionBit0026 = (value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define READ_PRODUCER_BIT_001B() READ_PRODUCER_BIT(0x1bu)
#define READ_PRODUCER_BIT_0020() READ_PRODUCER_BIT(0x20u)
#define READ_PRODUCER_BIT_0026() READ_PRODUCER_BIT(0x26u)
#define WRITE_PRODUCER_BIT_0026(value) WRITE_PRODUCER_BIT(0x26u, value)
#endif
#endif
