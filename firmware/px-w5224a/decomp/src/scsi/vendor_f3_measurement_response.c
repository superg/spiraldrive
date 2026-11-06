#include "vendor_f3_measurement_response.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_f3_measurement_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadVendorF3ResponseWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadVendorF3ResponseDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteVendorF3ResponseWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteVendorF3ResponseDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void ClearVendorF3ResponseRange(fw_addr_t first, fw_addr_t exclusive_end)
{
    while(first != exclusive_end)
    {
        FirmwareWrite8(first, 0u);
        ++first;
    }
}

static void BuildVendorF3EightChannelCompactResponse(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0x0eu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1080));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1080) == 0u)
    {
        ClearVendorF3ResponseRange(FW_NEAR_UNKNOWN_0B2D, FW_NEAR_UNKNOWN_0B38);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1082));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, FirmwareReadNear8(FW_NEAR_UNKNOWN_1081));
        WriteVendorF3ResponseDword(FW_NEAR_UNKNOWN_0B30, ReadVendorF3ResponseDword(FW_NEAR_UNKNOWN_1083) - 0x96ul);
        WriteVendorF3ResponseWord(FW_NEAR_UNKNOWN_0B34, ReadVendorF3ResponseWord(FW_NEAR_UNKNOWN_1087));
        WriteVendorF3ResponseWord(FW_NEAR_UNKNOWN_0B36, ReadVendorF3ResponseWord(FW_NEAR_UNKNOWN_1091));
    }
    WriteVendorF3ResponseWord(FW_NEAR_UNKNOWN_0B38, 0u);
}

static void BuildVendorF3EightChannelExtendedResponse(void)
{
    u8 index;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0x18u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1080));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1080) == 0u)
    {
        for(index = 3u; index < 0x1au; ++index)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A + index, 0u);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1082));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, FirmwareReadNear8(FW_NEAR_UNKNOWN_1081));
    WriteVendorF3ResponseDword(FW_NEAR_UNKNOWN_0B30, ReadVendorF3ResponseDword(FW_NEAR_UNKNOWN_1083) - 0x96ul);
    for(index = 0u; index < 8u; ++index)
        WriteVendorF3ResponseWord(FW_NEAR_UNKNOWN_0B34 + (fw_addr_t)((u16)index << 1), ReadVendorF3ResponseWord(FW_NEAR_UNKNOWN_1087 + (fw_addr_t)((u16)index << 1)));
}

static void BuildVendorF3ControllerCommand0EResponse(u8 selector)
{
    u8 status;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0x0eu);
    status = FirmwareReadNear8(FW_NEAR_UNKNOWN_10A2);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, status);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_10A2) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, (u8)(selector << 4));
        ClearVendorF3ResponseRange(FW_NEAR_UNKNOWN_0B2E, FW_NEAR_UNKNOWN_0B38);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, (u8)((selector << 4) | ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) >> 5) & 1u)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, FirmwareReadNear8(FW_NEAR_UNKNOWN_10A3));
        WriteVendorF3ResponseDword(FW_NEAR_UNKNOWN_0B30, ReadVendorF3ResponseDword(FW_NEAR_UNKNOWN_10A4) - 0x96ul);
        WriteVendorF3ResponseWord(FW_NEAR_UNKNOWN_0B34, ReadVendorF3ResponseWord(FW_NEAR_UNKNOWN_10AC));
        WriteVendorF3ResponseWord(FW_NEAR_UNKNOWN_0B36, ReadVendorF3ResponseWord(FW_NEAR_UNKNOWN_10B2));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B38, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) >> 6) & 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B39, 0u);
}

/* RAM:fbe744 */
void BuildVendorF3MeasurementResultResponse(void)
{
    u8 transfer_limit = CAPTURE_VENDOR_F3_RESPONSE_R15();
    u8 response_form = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) & 3u);
    u8 selector = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) >> 4);
    u8 transfer_length;
    u8 transfer_status;

    if(selector == 0u)
    {
        if(response_form == 0u)
        {
            BuildVendorF3EightChannelCompactResponse();
            transfer_limit = 0x10u;
        }
        else
        {
            BuildVendorF3EightChannelExtendedResponse();
            transfer_limit = 0x1au;
        }
    }
    else if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u)
    {
        VENDOR_F3_RESPONSE_UNSUPPORTED();
    }
    else
    {
        transfer_limit = 0x10u;
        BuildVendorF3ControllerCommand0EResponse(selector);
    }

    transfer_length = VENDOR_F3_RESPONSE_MIN(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14), transfer_limit);
    transfer_status = VENDOR_F3_RESPONSE_TRANSFER(FW_NEAR_UNKNOWN_0B2A, (u32)transfer_length);
    if(transfer_status != 4u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xdfu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1082, 0u);
        VENDOR_F3_RESPONSE_NO_SENSE();
    }
}
