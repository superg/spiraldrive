#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawReadHeaderByteRead(fw_addr_t address);
extern void ObserveRawReadHeaderByteWrite(fw_addr_t address, u8 value);
extern u8 ObserveRawReadHeaderPackedBcdToBinary(u8 value);
#define READ_RAW_READ_HEADER_BYTE(address) ObserveRawReadHeaderByteRead(address)
#define WRITE_RAW_READ_HEADER_BYTE(address, value) ObserveRawReadHeaderByteWrite((address), (value))
#define CONVERT_RAW_READ_HEADER_BCD(value) ObserveRawReadHeaderPackedBcdToBinary(value)
#define DISABLE_RAW_READ_HEADER_INTERRUPTS() FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu))
#else
sfr RawReadHeaderInterruptEnable = 0xa8;
sfr RawReadHeaderInterruptControl = 0xa9;
sbit RawReadHeaderGlobalInterruptEnable = 0xaf;
#define READ_RAW_READ_HEADER_BYTE(address) FirmwareRead8(address)
#define WRITE_RAW_READ_HEADER_BYTE(address, value) FirmwareWrite8((address), (value))
#define CONVERT_RAW_READ_HEADER_BCD(value) PackedBcdByteToBinary(value)
#define DISABLE_RAW_READ_HEADER_INTERRUPTS() (RawReadHeaderGlobalInterruptEnable = 0)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteDirectSfr8(A8, saved_ie);
FirmwareWriteDirectSfr8(A9, saved_a9);
#else
RawReadHeaderInterruptEnable = saved_ie;
RawReadHeaderInterruptControl = saved_a9;
#endif
#endif
