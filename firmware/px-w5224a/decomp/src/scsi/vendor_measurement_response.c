#include "vendor_measurement_response.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "vendor.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_measurement_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. */
static u16 ReadNativeWord(fw_addr_t source)
{
    return FirmwareReadU16(source);
}

/* Reconstruction helper; no direct Ghidra address. */
static void WriteNativeWord(fw_addr_t destination, u16 value)
{
    FirmwareWriteU16(destination, value);
}

/* Reconstruction helper; no direct Ghidra address. */
static u32 ReadNativeDword(fw_addr_t source)
{
    return FirmwareReadU32(source);
}

/* RAM:fbdef0 */
void PLEXTOR_NEAR_FUNCTION BuildVendorF2Mode2Or6MeasurementResponse(void)
{
    u32 controller_offset;

    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F09D, FW_NEAR_UNKNOWN_0B38, 0x1eu);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F0BB, FW_NEAR_UNKNOWN_0B56, 0x1eu);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F0D9, FW_NEAR_UNKNOWN_0B74, 0x1eu);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F0F7, FW_NEAR_UNKNOWN_0B92, 0x1eu);
    BuildVendorF2SharedResponse();

    controller_offset = CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, ReadNativeDword(FW_NEAR_UNKNOWN_155A), 0xd4ul);

    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EF89, FW_NEAR_UNKNOWN_0B2A, 0x1eu);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFA7, FW_NEAR_UNKNOWN_0B48, 0x1eu);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFC5, FW_NEAR_UNKNOWN_0B66, 0x1eu);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFE3, FW_NEAR_UNKNOWN_0B84, 0x1eu);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 0x07u) == 6u)
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F01F, FW_NEAR_UNKNOWN_0BA2, 0x1eu);
    else
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F03F, FW_NEAR_UNKNOWN_0BA2, 0x1eu);

    WriteNativeWord(FW_NEAR_UNKNOWN_0BC0, ReadNativeWord(FW_NEAR_UNKNOWN_EF85));
    WriteNativeWord(FW_NEAR_UNKNOWN_0BC2, ReadNativeWord(FW_NEAR_UNKNOWN_EF87));
    WriteNativeWord(FW_NEAR_UNKNOWN_0BC4, ReadNativeWord(FW_NEAR_UNKNOWN_F062));
    WriteNativeWord(FW_NEAR_UNKNOWN_0BC8, ReadNativeWord(FW_NEAR_UNKNOWN_F064));

    (void)CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, 0xa1ul);
}

/* RAM:fbda50 */
/* The
 * RAM:fbda43 wrapper supplies the zero clear-loop index. */
void PLEXTOR_NEAR_FUNCTION BuildVendorF2ControllerMeasurementResponseBody(void)
{
    u16 index;
    u8 flags;
    u8 mode;

    for(index = 0u; index < 0x00ffu; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A + index, 0u);

    WriteNativeWord(FW_NEAR_UNKNOWN_0B2A, ReadNativeWord(FW_NEAR_UNKNOWN_F9A0));
    WriteNativeWord(FW_NEAR_UNKNOWN_0B2C, ReadNativeWord(FW_NEAR_UNKNOWN_F9A2));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, FirmwareReadNear8(FW_NEAR_UNKNOWN_1008));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, FirmwareReadNear8(FW_NEAR_UNKNOWN_EF74));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0x0fu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 0x07u));
    flags = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0x20u) << 1);
    flags |= (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0x10u) << 3);
    flags |= FirmwareReadNear8(FW_NEAR_UNKNOWN_F076);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B34, flags);

    mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 0x07u);
    switch(mode)
    {
    case 1u:
        WriteNativeWord(FW_NEAR_UNKNOWN_0B36, 0x0134u);
        BuildVendorF2Mode1Response();
        break;
    case 2u:
    case 6u:
        WriteNativeWord(FW_NEAR_UNKNOWN_0B36, 0x0169u);
        BuildVendorF2Mode2Or6Response();
        break;
    case 3u:
        WriteNativeWord(FW_NEAR_UNKNOWN_0B36, 0x00c6u);
        CopyVendorF2Series0123();
        BuildVendorF2SharedResponse();
        break;
    case 4u:
        WriteNativeWord(FW_NEAR_UNKNOWN_0B36, 0x007cu);
        CopyVendorF2Series0123();
        BuildVendorF2AlternateResponse();
        break;
    default:
        break;
    }
}

/* RAM:fbe20f */
void PLEXTOR_NEAR_FUNCTION BuildVendorF2ShortControllerMeasurementResponse(void)
{
    u8 index;

    for(index = 0u; index < 0x8eu; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A + index, 0u);

    WriteNativeWord(FW_NEAR_UNKNOWN_0B2A, 0x008bu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, (u8)ReadNativeWord(FW_NEAR_UNKNOWN_F9A0));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, (u8)ReadNativeWord(FW_NEAR_UNKNOWN_F9A2));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, FirmwareReadNear8(FW_NEAR_UNKNOWN_1008));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEF));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B38, 5u);

    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFA7, FW_NEAR_UNKNOWN_0B3A, 0x28u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFC5, FW_NEAR_UNKNOWN_0B62, 0x14u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFE3, FW_NEAR_UNKNOWN_0B76, 0x14u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F03F, FW_NEAR_UNKNOWN_0B8A, 0x28u);

    WriteNativeWord(FW_NEAR_UNKNOWN_0BB2, ReadNativeWord(FW_NEAR_UNKNOWN_FACF));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0BB4, FirmwareReadNear8(FW_NEAR_UNKNOWN_FADB));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0BB5, FirmwareReadNear8(FW_NEAR_UNKNOWN_F962));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0BB6, (u8)ReadNativeWord(FW_NEAR_UNKNOWN_FB5F));
}
