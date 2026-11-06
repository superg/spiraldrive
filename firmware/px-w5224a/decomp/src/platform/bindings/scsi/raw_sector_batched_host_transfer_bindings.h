#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 FirmwareReadBit000D(void);
extern u8 FirmwareReadBit0045(void);
#define READ_BATCH_BIT_000D() FirmwareReadBit000D()
#define READ_BATCH_BIT_0045() FirmwareReadBit0045()
#else
u8 bdata RawSectorBatchBitByte21 _at_ 0x21;
u8 bdata RawSectorBatchBitByte28 _at_ 0x28;
sbit RawSectorBatchBit000D = RawSectorBatchBitByte21 ^ 5;
sbit RawSectorBatchBit0045 = RawSectorBatchBitByte28 ^ 5;
#define READ_BATCH_BIT_000D() ((u8)RawSectorBatchBit000D)
#define READ_BATCH_BIT_0045() ((u8)RawSectorBatchBit0045)
#endif
#endif
