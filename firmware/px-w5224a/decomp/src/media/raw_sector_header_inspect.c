#include "raw_sector_header_inspect.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_sector_header_inspect_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra addresses. */
static u16 ReadHeaderWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadHeaderDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteHeaderWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteHeaderDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fc5cd8 */
u8 PLEXTOR_NEAR_FUNCTION InspectLatestRawSectorDescriptorAndCaptureHeader(void)
{
    u16 descriptor_address;
    u16 descriptor_status;
    u16 repeated_header_offset;
    u8 index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        return 5u;

    descriptor_address = GET_LATEST_HEADER_DESCRIPTOR();
    if(descriptor_address == 0u)
        return 0xfeu;

    descriptor_status = ReadHeaderWord((fw_addr_t)(u16)(descriptor_address + 0x0eu));
    if(descriptor_status != 0u)
    {
        if(descriptor_status == 0x0200u)
            return 8u;
        if(descriptor_status == 0x6400u)
            return 3u;
        if(descriptor_status == 0x800bu)
            return 4u;
        return 2u;
    }

    if((s16)ReadHeaderWord(FW_NEAR_UNKNOWN_1296) <= 30)
        return 0xfeu;

    WriteHeaderWord(FW_NEAR_UNKNOWN_1298, 30u);
    descriptor_address = GET_SECONDARY_HEADER_DESCRIPTOR();
    WriteHeaderDword(FW_NEAR_UNKNOWN_15D8, ReadHeaderDword((fw_addr_t)(u16)(descriptor_address + 4u)) + 12ul);

    for(index = 0u; index < 36u; ++index)
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BB919 + index, READ_HEADER_RING_BYTE());

    repeated_header_offset = 0u;
    if((FirmwareRead8(FW_HUGE_UNKNOWN_2BB91C) & 3u) == 2u)
    {
        repeated_header_offset = 8u;
        for(index = 4u; index < 8u; ++index)
        {
            if(FirmwareRead8(FW_HUGE_UNKNOWN_2BB919 + index) != FirmwareRead8(FW_HUGE_UNKNOWN_2BB91D + index))
            {
                repeated_header_offset = 0u;
                break;
            }
        }
    }

    if(FirmwareRead8(FW_HUGE_UNKNOWN_2BB922 + repeated_header_offset) == 0u)
    {
        WriteHeaderDword(FW_NEAR_UNKNOWN_15D8, ReadHeaderDword((fw_addr_t)(u16)(descriptor_address + 4u)) + repeated_header_offset + 24ul);

        for(index = 0u; index < 100u; ++index)
        {
            if(READ_HEADER_RING_BYTE() == FirmwareReadNear8(FW_NEAR_UNKNOWN_0F65))
            {
                u8 trailing_index;

                WriteHeaderDword(FW_NEAR_UNKNOWN_15D8, ReadHeaderDword(FW_NEAR_UNKNOWN_15D8) - 1ul);
                for(trailing_index = 0u; (s16)(u16)trailing_index < (s16)(24u - repeated_header_offset); ++trailing_index)
                    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB925 + repeated_header_offset + trailing_index, READ_HEADER_RING_BYTE());
                break;
            }

            WriteHeaderDword(FW_NEAR_UNKNOWN_15D8, ReadHeaderDword(FW_NEAR_UNKNOWN_15D8) + 15ul);
        }
    }

    return 1u;
}
