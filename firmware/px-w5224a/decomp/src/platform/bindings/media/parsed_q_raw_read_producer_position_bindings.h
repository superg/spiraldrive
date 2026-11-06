#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveParsedQProducerPositionReadBit(u8 bit_address);
#define READ_PARSED_Q_PRODUCER_BIT(address) ObserveParsedQProducerPositionReadBit((address))
#define READ_PARSED_Q_PRODUCER_BIT_0013() READ_PARSED_Q_PRODUCER_BIT(0x13u)
#define READ_PARSED_Q_PRODUCER_BIT_001C() READ_PARSED_Q_PRODUCER_BIT(0x1cu)
#define READ_PARSED_Q_PRODUCER_BIT_001D() READ_PARSED_Q_PRODUCER_BIT(0x1du)
#else
u8 bdata ParsedQProducerPositionBitByte22 _at_ 0x22;
u8 bdata ParsedQProducerPositionBitByte23 _at_ 0x23;
sbit ParsedQProducerPositionBit0013 = ParsedQProducerPositionBitByte22 ^ 3;
sbit ParsedQProducerPositionBit001C = ParsedQProducerPositionBitByte23 ^ 4;
sbit ParsedQProducerPositionBit001D = ParsedQProducerPositionBitByte23 ^ 5;
#define READ_PARSED_Q_PRODUCER_BIT_0013() ((u8)ParsedQProducerPositionBit0013)
#define READ_PARSED_Q_PRODUCER_BIT_001C() ((u8)ParsedQProducerPositionBit001C)
#define READ_PARSED_Q_PRODUCER_BIT_001D() ((u8)ParsedQProducerPositionBit001D)
#endif
#endif
