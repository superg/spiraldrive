#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWriteControllerBufferByteForAddressingTest(fw_addr_t buffer_offset, u8 value);
extern u8 ObserveReadControllerBufferByteForAddressingTest(fw_addr_t buffer_offset);
#define WRITE_BANK_STRIDE_BYTE(offset, value) ObserveWriteControllerBufferByteForAddressingTest((offset), (value))
#define READ_BANK_STRIDE_BYTE(offset) ObserveReadControllerBufferByteForAddressingTest(offset)
#define READ_RESERVED_CONTROLLER_BUFFER_BASE()                                                                                                                                                         \
    (((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_155A) << 24) | ((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_155B) << 16) | ((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_155C) << 8)                                 \
        | (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_155D))
#else
#define WRITE_BANK_STRIDE_BYTE(offset, value) WriteControllerBufferWindowByteAtomic((offset), (value))
#define READ_BANK_STRIDE_BYTE(offset) ReadControllerBufferByteAtomic(offset)
#define READ_RESERVED_CONTROLLER_BUFFER_BASE() (*(volatile u32 huge *)FW_NEAR_UNKNOWN_155A)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u8 ReadControllerBufferByteAtomic(fw_addr_t buffer_offset)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 value;

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    value = FirmwareRead8(FirmwareAddressIncrement24(buffer_offset, FW_ADDRESS_BANK_STRIDE));
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
    return value;
}
#else
/* RAM:feea1c is implemented by the native atomic boundary. */
extern u8 ReadControllerBufferByteAtomic(fw_addr_t buffer_offset);
#endif
#endif
