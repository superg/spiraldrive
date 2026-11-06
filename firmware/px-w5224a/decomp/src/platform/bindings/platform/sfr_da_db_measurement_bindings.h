#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveSfrDaDbMeasurementBit(u8 bit_address);
extern void ObserveWriteSfrDaDbMeasurementBit(u8 bit_address, u8 value);
#define READ_SFR_DA() FirmwareReadDirectSfr8(DA)
#define READ_SFR_DB() FirmwareReadDirectSfr8(DB)
#define READ_BIT0028() ObserveSfrDaDbMeasurementBit(0x28u)
#define READ_BIT002C() ObserveSfrDaDbMeasurementBit(0x2cu)
#define READ_BIT002D() ObserveSfrDaDbMeasurementBit(0x2du)
#define READ_BIT0036() ObserveSfrDaDbMeasurementBit(0x36u)
#define READ_BIT0039() ObserveSfrDaDbMeasurementBit(0x39u)
#define WRITE_BIT002C(value) ObserveWriteSfrDaDbMeasurementBit(0x2cu, value)
#define WRITE_BIT002D(value) ObserveWriteSfrDaDbMeasurementBit(0x2du, value)
#define WRITE_BIT0039(value) ObserveWriteSfrDaDbMeasurementBit(0x39u, value)
#else
sfr SfrDaDbMeasurementDa = 0xda;
sfr SfrDaDbMeasurementDb = 0xdb;
u8 bdata SfrDaDbMeasurementBitByte25 _at_ 0x25;
u8 bdata SfrDaDbMeasurementBitByte26 _at_ 0x26;
u8 bdata SfrDaDbMeasurementBitByte27 _at_ 0x27;
sbit SfrDaDbMeasurementBit0028 = SfrDaDbMeasurementBitByte25 ^ 0;
sbit SfrDaDbMeasurementBit002C = SfrDaDbMeasurementBitByte25 ^ 4;
sbit SfrDaDbMeasurementBit002D = SfrDaDbMeasurementBitByte25 ^ 5;
sbit SfrDaDbMeasurementBit0036 = SfrDaDbMeasurementBitByte26 ^ 6;
sbit SfrDaDbMeasurementBit0039 = SfrDaDbMeasurementBitByte27 ^ 1;
#define READ_SFR_DA() SfrDaDbMeasurementDa
#define READ_SFR_DB() SfrDaDbMeasurementDb
#define READ_BIT0028() SfrDaDbMeasurementBit0028
#define READ_BIT002C() SfrDaDbMeasurementBit002C
#define READ_BIT002D() SfrDaDbMeasurementBit002D
#define READ_BIT0036() SfrDaDbMeasurementBit0036
#define READ_BIT0039() SfrDaDbMeasurementBit0039
#define WRITE_BIT002C(value) (SfrDaDbMeasurementBit002C = (value))
#define WRITE_BIT002D(value) (SfrDaDbMeasurementBit002D = (value))
#define WRITE_BIT0039(value) (SfrDaDbMeasurementBit0039 = (value))
#endif
#endif
