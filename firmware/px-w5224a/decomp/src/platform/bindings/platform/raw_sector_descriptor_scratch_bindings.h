#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawSectorDescriptorBit001F(void);
extern u8 ObserveRawSectorDescriptorControllerRead(u32 controller_address);
#define READ_DESCRIPTOR_BIT_001F() ObserveRawSectorDescriptorBit001F()
#define READ_DESCRIPTOR_CONTROLLER_BYTE(address) ObserveRawSectorDescriptorControllerRead(address)
#else
u8 bdata RawSectorDescriptorBitByte23 _at_ 0x23;
sbit RawSectorDescriptorBit001F = RawSectorDescriptorBitByte23 ^ 7;
#define READ_DESCRIPTOR_BIT_001F() ((u8)RawSectorDescriptorBit001F)
#define READ_DESCRIPTOR_CONTROLLER_BYTE(address) ReadControllerByteAtAddress(address)
#endif
#endif
