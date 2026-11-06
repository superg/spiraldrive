#include "vendor_eb.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define VENDOR_EB_CDB FW_NEAR_HOST_COMMAND_OPCODE
#define VENDOR_EB_RESPONSE FW_NEAR_UNKNOWN_0B2A

static u16 ReadVendorEbU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadVendorEbU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteVendorEbU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u8 SelectVendorEbFallbackWriteSpeed(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        return 4u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
        return 0x18u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) < 2u)
        return 0u;
    return 4u;
}

static void WriteVendorEbSpeed(u8 raw_format, u8 speed_x, fw_addr_t destination)
{
    if(raw_format != 0u)
        WriteVendorEbU16(destination, speed_x);
    else
        WriteCdSpeedKilobytesPerSecond(speed_x, destination);
}

/* RAM:fcab1a */
void HandleScsiOpcodeEB(void)
{
    u16 allocation_length = ReadVendorEbU16(VENDOR_EB_CDB + 8u);
    u8 response_format;
    u8 options;
    u8 primary_speed;
    u8 secondary_speed;
    u16 transfer_length;

    if((FirmwareRead8(VENDOR_EB_CDB + 1u) & 0x1fu) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    WriteVendorEbU16(VENDOR_EB_RESPONSE, 8u);
    response_format = (u8)(FirmwareRead8(VENDOR_EB_CDB + 2u) & 0x0fu);
    options = (u8)(FirmwareRead8(VENDOR_EB_CDB + 3u) & 3u);
    FirmwareWrite8(VENDOR_EB_RESPONSE + 2u, response_format);
    FirmwareWrite8(VENDOR_EB_RESPONSE + 3u, options);

    if((options & 1u) == 0u)
    {
        primary_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C7B);
        secondary_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C7A);
    }
    else
    {
        primary_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C7D);
        if((options & 2u) == 0u)
            secondary_speed = SelectStateEligibleLowestWriteSpeed();
        else
            secondary_speed = SelectVendorEbFallbackWriteSpeed();
    }

    WriteVendorEbSpeed((u8)(response_format == 1u), primary_speed, VENDOR_EB_RESPONSE + 4u);
    WriteVendorEbSpeed((u8)(response_format == 1u), secondary_speed, VENDOR_EB_RESPONSE + 6u);
    WriteVendorEbSpeed((u8)(response_format == 1u), FirmwareReadNear8(FW_NEAR_UNKNOWN_0C7C), VENDOR_EB_RESPONSE + 8u);

    (void)CopyToControllerBufferWindow(VENDOR_EB_RESPONSE, ReadVendorEbU32(FW_NEAR_UNKNOWN_155A), 10ul);
    transfer_length = allocation_length < 10u ? allocation_length : 10u;
    if(ExecuteControllerDataInAndMapStatus((u32)transfer_length) != 4u)
        SetNoSenseData();
}
