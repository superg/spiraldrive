#include "vendor_ed.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_ed_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadVendorEdU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadVendorEdU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteVendorEdU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fcba91 */
void HandleScsiOpcodeED(void)
{
    u16 allocation_length;
    u16 response_length;
    u8 speed_x;

    if(ReadVendorEdU32(FW_NEAR_UNKNOWN_0B0C) != 0ul || ReadVendorEdU16(FW_NEAR_UNKNOWN_0B10) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) != 0u)
    {
        RejectVendorEdCommand();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x10u) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x01u) != 0u)
        {
            WriteVendorEdU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadVendorEdU16(FW_NEAR_UNKNOWN_FAD7) | 0x0001u));
        }
        else
        {
            WriteVendorEdU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadVendorEdU16(FW_NEAR_UNKNOWN_FAD7) & 0xfffeu));
            if((ReadVendorEdU16(FW_NEAR_UNKNOWN_FAD7) & 0x0004u) != 0u)
            {
                WriteVendorEdU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadVendorEdU16(FW_NEAR_UNKNOWN_FAD7) & 0xfffbu));
                speed_x = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD3);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, speed_x);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, speed_x);
            }
        }
    }

    allocation_length = ReadVendorEdU16(FW_NEAR_UNKNOWN_0B12);
    WriteVendorEdU16(FW_NEAR_UNKNOWN_0B2A, 6u);
    WriteVendorEdU16(FW_NEAR_UNKNOWN_0B30, 0u);
    WriteVendorEdU16(FW_NEAR_UNKNOWN_0B2C, 0u);
    if((ReadVendorEdU16(FW_NEAR_UNKNOWN_FAD7) & 0x0001u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2C) | 0x01u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
        speed_x = SelectVendorEdWriteSpeed();
    else
        speed_x = 0u;
    WriteVendorEdCdSpeed(speed_x, FW_NEAR_UNKNOWN_0B2E);

    response_length = (u16)(ReadVendorEdU16(FW_NEAR_UNKNOWN_0B2A) + 2u);
    if(response_length < allocation_length)
        allocation_length = response_length;
    if(ExecuteVendorEdDataIn(FW_NEAR_UNKNOWN_0B2A, allocation_length) != 4u)
        SetVendorEdNoSense();
}

/* RAM:fcb976 */
/* ERET after the
 * completion path; RestartFirmware does not return on the reset path. */
void HandleScsiOpcodeEE(void)
{
    u8 selector;
    u8 requested_mode;
    u8 mode_class;
    u8 mode_index;
    u8 staged_mode;
    u8 accepted = 0u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x10u) != 0u)
    {
        requested_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D);
        selector = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x03u);
        if(selector != 0u)
        {
            staged_mode = requested_mode;
            if(selector != 3u)
            {
                mode_class = (u8)(requested_mode & 0xf8u);
                mode_index = (u8)(requested_mode & 0x07u);
                if(mode_class == 0x20u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2D, mode_index < 3u ? mode_index : 2u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 3u);
                    accepted = 1u;
                }
                else if(mode_class == 0x40u)
                {
                    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) >> 3) & 1u) != 0u)
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2C, mode_index < 3u ? mode_index : 2u);
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 7u);
                        accepted = 1u;
                    }
                }
                else if(mode_class == 0x08u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2E, mode_index < 5u ? mode_index : 4u);
                }
                else if(requested_mode == 0xffu)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2E, 4u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2D, 2u);
                    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) >> 4) & 1u) == 0u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) >> 3) & 1u) == 0u)
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 3u);
                        staged_mode = 0x22u;
                    }
                    else
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2C, 2u);
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2B, 7u);
                        staged_mode = 0x42u;
                    }
                    accepted = 1u;
                }
            }

            if(accepted != 0u || selector == 3u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FE, staged_mode);
            if(selector > 1u)
            {
                SetVendorEeNoSense();
                return;
            }
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FD, 0x99u);
    RestartVendorEeFirmware();
}
