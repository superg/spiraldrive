#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void FirmwareWriteBit0013(u8 value);
extern void FirmwareWriteBit001E(u8 value);
extern void FirmwareWriteBit0580(u8 value);
extern void FirmwareWriteBit0581(u8 value);
#define WRITE_CD_TEXT_BIT_0013(value) FirmwareWriteBit0013(value)
#define WRITE_CD_TEXT_BIT_001E(value) FirmwareWriteBit001E(value)
#define WRITE_CD_TEXT_P3_RX(value) FirmwareWriteBit0580(value)
#define WRITE_CD_TEXT_P3_TX(value) FirmwareWriteBit0581(value)
#else
u8 bdata CdTextAcquisitionBitByte22 _at_ 0x22;
u8 bdata CdTextAcquisitionBitByte23 _at_ 0x23;
sbit CdTextAcquisitionBit0013 = CdTextAcquisitionBitByte22 ^ 3;
sbit CdTextAcquisitionBit001E = CdTextAcquisitionBitByte23 ^ 6;
sbit CdTextAcquisitionP3Rx = 0xb0;
sbit CdTextAcquisitionP3Tx = 0xb1;
#define WRITE_CD_TEXT_BIT_0013(value) (CdTextAcquisitionBit0013 = (value))
#define WRITE_CD_TEXT_BIT_001E(value) (CdTextAcquisitionBit001E = (value))
#define WRITE_CD_TEXT_P3_RX(value) (CdTextAcquisitionP3Rx = (value))
#define WRITE_CD_TEXT_P3_TX(value) (CdTextAcquisitionP3Tx = (value))
#endif
#endif
