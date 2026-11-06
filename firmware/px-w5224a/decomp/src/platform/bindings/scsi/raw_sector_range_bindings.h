#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 FirmwareReadBit000D(void);
extern u8 FirmwareReadBit000E(void);
extern u8 FirmwareReadBit0013(void);
extern void FirmwareWriteBit0001(u8 value);
extern void FirmwareWriteBit000C(u8 value);
extern void FirmwareWriteBit0013(u8 value);
extern void FirmwareWriteBit001B(u8 value);
extern void FirmwareWriteBit001C(u8 value);
extern void FirmwareWriteBit001D(u8 value);
extern void FirmwareWriteBit0045(u8 value);
extern void FirmwareWriteBit0580(u8 value);
extern void FirmwareWriteBit0581(u8 value);
#define POSITION_RAW_READ_PIPELINE(target) PositionRawReadPipelineAtFramePositionForTest(target)
#define READ_RANGE_BIT_000D() FirmwareReadBit000D()
#define READ_RANGE_BIT_000E() FirmwareReadBit000E()
#define READ_RANGE_BIT_0013() FirmwareReadBit0013()
#define WRITE_RANGE_BIT_0001(value) FirmwareWriteBit0001(value)
#define WRITE_RANGE_BIT_000C(value) FirmwareWriteBit000C(value)
#define WRITE_RANGE_BIT_0013(value) FirmwareWriteBit0013(value)
#define WRITE_RANGE_BIT_001B(value) FirmwareWriteBit001B(value)
#define WRITE_RANGE_BIT_001C(value) FirmwareWriteBit001C(value)
#define WRITE_RANGE_BIT_001D(value) FirmwareWriteBit001D(value)
#define WRITE_RANGE_BIT_0045(value) FirmwareWriteBit0045(value)
#define WRITE_RANGE_P3_RX(value) FirmwareWriteBit0580(value)
#define WRITE_RANGE_P3_TX(value) FirmwareWriteBit0581(value)
#else
u8 bdata RawSectorRangeBitByte20 _at_ 0x20;
u8 bdata RawSectorRangeBitByte21 _at_ 0x21;
u8 bdata RawSectorRangeBitByte22 _at_ 0x22;
u8 bdata RawSectorRangeBitByte23 _at_ 0x23;
u8 bdata RawSectorRangeBitByte28 _at_ 0x28;
sbit RawSectorRangeBit0001 = RawSectorRangeBitByte20 ^ 1;
sbit RawSectorRangeBit000C = RawSectorRangeBitByte21 ^ 4;
sbit RawSectorRangeBit000D = RawSectorRangeBitByte21 ^ 5;
sbit RawSectorRangeBit000E = RawSectorRangeBitByte21 ^ 6;
sbit RawSectorRangeBit0013 = RawSectorRangeBitByte22 ^ 3;
sbit RawSectorRangeBit001B = RawSectorRangeBitByte23 ^ 3;
sbit RawSectorRangeBit001C = RawSectorRangeBitByte23 ^ 4;
sbit RawSectorRangeBit001D = RawSectorRangeBitByte23 ^ 5;
sbit RawSectorRangeBit0045 = RawSectorRangeBitByte28 ^ 5;
sbit RawSectorRangeP3Rx = 0xb0;
sbit RawSectorRangeP3Tx = 0xb1;
#define READ_RANGE_BIT_000D() ((u8)RawSectorRangeBit000D)
#define READ_RANGE_BIT_000E() ((u8)RawSectorRangeBit000E)
#define READ_RANGE_BIT_0013() ((u8)RawSectorRangeBit0013)
#define WRITE_RANGE_BIT_0001(value) (RawSectorRangeBit0001 = (value))
#define WRITE_RANGE_BIT_000C(value) (RawSectorRangeBit000C = (value))
#define WRITE_RANGE_BIT_0013(value) (RawSectorRangeBit0013 = (value))
#define WRITE_RANGE_BIT_001B(value) (RawSectorRangeBit001B = (value))
#define WRITE_RANGE_BIT_001C(value) (RawSectorRangeBit001C = (value))
#define WRITE_RANGE_BIT_001D(value) (RawSectorRangeBit001D = (value))
#define WRITE_RANGE_BIT_0045(value) (RawSectorRangeBit0045 = (value))
#define WRITE_RANGE_P3_RX(value) (RawSectorRangeP3Rx = (value))
#define WRITE_RANGE_P3_TX(value) (RawSectorRangeP3Tx = (value))
#define POSITION_RAW_READ_PIPELINE(target) PositionRawReadPipelineAtFramePosition(target)
#endif
#endif
