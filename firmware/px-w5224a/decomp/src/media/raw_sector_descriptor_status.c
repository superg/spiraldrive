#include "raw_sector_descriptor_status.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_sector_descriptor_status_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadDescriptorWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadDescriptorDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void PublishRawSectorDescriptorSense(u16 descriptor_address, u8 sense_key, u16 asc_ascq)
{
    u16 frame_source = ReadDescriptorWord((fw_addr_t)(u16)(descriptor_address + 2u));
    u32 information = ReadDescriptorDword(frame_source);

    SET_DESCRIPTOR_SENSE(sense_key, asc_ascq);
    information -= ReadDescriptorDword(FW_NEAR_UNKNOWN_0F0C);
    information -= 150ul;
    SET_DESCRIPTOR_INFORMATION(information);
}

/* RAM:fc5364 */
u8 ValidateRawSectorDescriptorStatus(u16 descriptor_address)
{
    u8 sense_key = FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u));
    u16 asc_ascq;

    if(sense_key == 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 3u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u)
        return 0u;

    asc_ascq = ReadDescriptorWord((fw_addr_t)(u16)(descriptor_address + 0x0eu));
    if(asc_ascq == 0x1105u || asc_ascq == 0x1106u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1466, 0u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 0x10u) != 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1467) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1467, 1u);
                PublishRawSectorDescriptorSense(descriptor_address, sense_key, asc_ascq);
            }
            return 1u;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1467, 1u);
        PublishRawSectorDescriptorSense(descriptor_address, sense_key, asc_ascq);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 0x20u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 1u);
            return 1u;
        }
        return 0u;
    }

    if(asc_ascq == 0x1803u || asc_ascq == 0x1804u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 0x04u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1466, 1u);
            PublishRawSectorDescriptorSense(descriptor_address, sense_key, asc_ascq);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 0x02u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 0u);
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 0x20u) != 0u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 1u);
                    return 1u;
                }
                return 0u;
            }
        }
        return 1u;
    }

    PublishRawSectorDescriptorSense(descriptor_address, sense_key, asc_ascq);
    return 0u;
}
