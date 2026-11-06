#include "raw_read_header_normalization.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_header_normalization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadRawReadHeaderBigEndianWord(u16 address)
{
    return (u16)(((u16)READ_RAW_READ_HEADER_BYTE((fw_addr_t)address) << 8) | READ_RAW_READ_HEADER_BYTE((fw_addr_t)(u16)(address + 1u)));
}

/* Reconstruction helper; no direct Ghidra function address. The firmware's
 * descriptor handle stores the descriptor address in the low word and a
 * controller-buffer window selector in the upper word. */
static fw_addr_t AdvanceRawReadHeaderWindow(u32 descriptor_handle, u16 window_increment)
{
    u16 window = (u16)(descriptor_handle >> 16);

    return ((fw_addr_t)(u16)(window + window_increment) << 16) | (fw_addr_t)(u16)descriptor_handle;
}

/* RAM:fbfe72 */
void CopyAndNormalizeRawReadHeaderToLinkedBuffer(u32 descriptor_handle, u8 header_type)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 header[16];
    fw_addr_t source_address;
    fw_addr_t destination_address;
    u16 descriptor_address = (u16)descriptor_handle;
    u16 linked_address;
    u16 linked_window;
    u8 index;

    DISABLE_RAW_READ_HEADER_INTERRUPTS();

    source_address = AdvanceRawReadHeaderWindow(descriptor_handle, 0x10u);
    for(index = 0u; index < 12u; ++index)
    {
        header[index] = READ_RAW_READ_HEADER_BYTE(source_address);
        source_address = FirmwareAddressIncrement24(source_address, 1u);
        if(index != 0u && index < 10u && FirmwareReadBit0013() == 0u && (header[0] & 0x0fu) == 1u && (index != 1u || header[1] != 0xaau))
            header[index] = CONVERT_RAW_READ_HEADER_BCD(header[index]);
    }

    header[12] = 0u;
    header[13] = header_type;
    header[14] = 0xffu;
    header[15] = 0xe0u;

    linked_address = ReadRawReadHeaderBigEndianWord((u16)(descriptor_address + 2u));
    linked_window = ReadRawReadHeaderBigEndianWord(linked_address);
    destination_address = ((fw_addr_t)(u16)(linked_window + 0x10u) << 16) | descriptor_address;
    for(index = 0u; index < 16u; ++index)
    {
        WRITE_RAW_READ_HEADER_BYTE(destination_address, header[index]);
        destination_address = FirmwareAddressIncrement24(destination_address, 1u);
    }

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_header_normalization_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}
