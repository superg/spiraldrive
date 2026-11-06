#include "mode_sense_response.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "mode_sense_pages.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/mode_sense_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. */
static u16 ReadModeSenseResponseU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* Reconstruction helper; no direct Ghidra address. */
static u32 ReadModeSenseResponseU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* Reconstruction helper; no direct Ghidra address. */
static void WriteModeSenseResponseU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* Reconstruction helper; no direct Ghidra address. */
static u8 IsSupportedModeSensePage(u8 page_code)
{
    switch(page_code)
    {
    case 0x01u:
    case 0x02u:
    case 0x03u:
    case 0x05u:
    case 0x07u:
    case 0x08u:
    case 0x0du:
    case 0x0eu:
    case 0x1au:
    case 0x1du:
    case 0x2au:
    case 0x3fu:
        return 1u;
    default:
        return 0u;
    }
}

/* RAM:fc2b4a */
void ExecuteModeSense6Or10Response(u8 cdb_length, u16 allocation_length)
{
    u8 page_control = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) >> 6);
    u8 page_code = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) & 0x3fu);
    u16 response_length;
    u8 header_length;
    u32 response_address;

    if(page_control == 3u)
    {
        MODE_SENSE_SET_SENSE(5u, 0x3900u);
        return;
    }
    if(IsSupportedModeSensePage(page_code) == 0u)
    {
        MODE_SENSE_UNSUPPORTED();
        return;
    }
    if(allocation_length == 0u)
    {
        MODE_SENSE_NO_SENSE();
        return;
    }

    if(cdb_length == 6u)
    {
        response_length = 4u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x08u) == 0u)
            response_length = (u16)(response_length + 8u);
    }
    else
    {
        response_length = 8u;
    }
    if(page_code == 0x3fu)
        response_length = (u16)(response_length + 0x00dau);
    else
        response_length = (u16)(response_length + MODE_SENSE_PAYLOAD_LENGTH(page_code) + 2u);

    response_address = ReadModeSenseResponseU32(FW_NEAR_UNKNOWN_155A);
    if(cdb_length == 6u)
    {
        if(response_length < 0x00ffu)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, (u8)(response_length - 1u));
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0xfeu);
            response_length = 0x00ffu;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);
        header_length = 4u;
    }
    else
    {
        WriteModeSenseResponseU16(FW_NEAR_UNKNOWN_0B2A, (u16)(response_length - 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
        header_length = 8u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x08u) == 0u && cdb_length == 6u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, 0u);
        if(page_control == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B34, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B35, 0xffu);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, 0u);
            WriteModeSenseResponseU16(FW_NEAR_UNKNOWN_0B34, page_control == 2u ? 0x0800u : ReadModeSenseResponseU16(FW_NEAR_UNKNOWN_0B00));
        }
        header_length = (u8)(header_length + 8u);
    }
    else if(cdb_length == 6u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0u);
    }

    response_address = MODE_SENSE_COPY(FW_NEAR_UNKNOWN_0B2A, response_address, (u32)header_length);
    MODE_SENSE_APPEND(response_address, page_code, page_control);
    response_length = MinU16(allocation_length, response_length);
    if(MODE_SENSE_DATA_IN((u32)response_length) != 4u)
        MODE_SENSE_NO_SENSE();
}
