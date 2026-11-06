#include "image.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"

/* RAM:fdc434 */
void SelectMainFirmwareInterruptHandlers(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FC, 1u);
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/boot/image_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f84900 */
void InitializeInquiryResponseBuffer(void)
{
    u8 response_offset;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCC5, 0x05u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCC6, 0x80u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCC7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCC8, 0x21u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCC9, 0x5bu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCCA, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCCB, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCCC, 0u);
    CopyBytesWith8BitCount(FW_NEAR_UNKNOWN_DCCD, PLEXTOR_IMAGE_HEADER_ADDRESS(FW_ROM_UNKNOWN_F80000), 8u);
    CopyBytesWith8BitCount(FW_NEAR_UNKNOWN_DCD5, PLEXTOR_IMAGE_HEADER_ADDRESS(FW_ROM_UNKNOWN_F80008), 16u);
    CopyBytesWith8BitCount(FW_NEAR_UNKNOWN_DCE5, PLEXTOR_IMAGE_HEADER_ADDRESS(FW_ROM_UNKNOWN_F80018), 4u);
    CopyBytesWith8BitCount(FW_NEAR_UNKNOWN_DCE9, PLEXTOR_IMAGE_HEADER_ADDRESS(FW_ROM_UNKNOWN_F8001C), 20u);
    for(response_offset = 0x38u; response_offset < 0x60u; response_offset++)
        FirmwareWriteNear8((fw_addr_t)(FW_NEAR_UNKNOWN_DCC5 + response_offset), 0u);
}

/* RAM:fdc84f */
u8 VerifyAdditiveChecksumRange(fw_addr_t end_address, fw_addr_t start_address)
{
    u16 sum = 0;
    fw_addr_t cursor = start_address;

    if(end_address == FW_ROM_UNKNOWN_F7A000)
    {
        while(cursor < FW_ROM_UNKNOWN_F70000)
        {
            sum = (u16)(sum + FirmwareRead8(cursor));
            cursor++;
        }
        cursor = FW_ROM_UNKNOWN_F78000;
        while(cursor < FW_ROM_UNKNOWN_F79FFE)
        {
            sum = (u16)(sum + FirmwareRead8(cursor));
            cursor++;
        }
    }
    else
    {
        while(cursor < end_address - 2ul)
        {
            sum = (u16)(sum + FirmwareRead8(cursor));
            cursor++;
        }
    }
    return (u8)(sum == FirmwareReadBigEndianU16(cursor));
}

/* RAM:fdc8c7 */
u8 IsMemoryRangeErased(fw_addr_t end_address, fw_addr_t start_address)
{
    fw_addr_t cursor = start_address;

    while(cursor < end_address)
    {
        if(FirmwareRead8(cursor) != 0xffu)
            return 0;
        cursor++;
    }
    return 1;
}

/* RAM:fdc806 */
u8 ValidateBootAndConfigurationMemory(void)
{
    if(!VerifyAdditiveChecksumRange(FW_ROM_UNKNOWN_F7A000, FW_ROM_UNKNOWN_F00000))
        return 0;
    if(VerifyAdditiveChecksumRange(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C060), PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C000)))
        return 1;
    return IsMemoryRangeErased(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C060), PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C000));
}
