#include "vendor.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. Native word access uses
 * consecutive addresses in the full RAM space, not a low-word pointer bump. */
static void WriteNativeWord(fw_addr_t destination, u16 value)
{
    FirmwareWriteU16(destination, value);
}

/* Reconstruction helper; no direct Ghidra address. The low half is published
 * first, matching the native @DR0+2 store even when the outputs alias. */
static void PublishVendorWindowAddress(fw_addr_t output, u16 source)
{
    WriteNativeWord(FirmwareAddressIncrement24(output, 2u), source);
    WriteNativeWord(output, 0u);
}

/* Reconstruction helper; no direct Ghidra address. */
static u16 ReadSummaryWord(u16 source)
{
    u8 high = FirmwareReadNear8(source);
    u8 low = FirmwareReadNear8((u16)(source + 1u));
    return (u16)(((u16)high << 8) | low);
}

/* RAM:fcb861 */
/* Source DR0 points to a four-byte address slot, DR4 to a two-byte length
 * slot; source LCALL/RET. Internal reconstruction uses the C251 ABI. */
void SelectVendorF5Mode1BufferWindow(fw_addr_t source_output, fw_addr_t length_output)
{
    PublishVendorWindowAddress(source_output, 0xfb6au);
    WriteNativeWord(length_output, 12u);
}

/* RAM:fcb876 */
void SelectVendorF5Mode6BufferWindow(fw_addr_t source_output, fw_addr_t length_output)
{
    PublishVendorWindowAddress(source_output, 0xfb7au);
    WriteNativeWord(length_output, 18u);
}

/* RAM:fcb88b */
/* The initial clear is one word, not a four-byte output-slot clear. */
void SelectVendorF5Mode7BufferWindow(fw_addr_t source_output, fw_addr_t length_output)
{
    WriteNativeWord(length_output, 0u);
    PublishVendorWindowAddress(source_output, 0x116eu);
    WriteNativeWord(length_output, 92u);
}

/* RAM:fcb8a5 */
void SelectVendorF5Mode5BufferWindow(fw_addr_t source_output, fw_addr_t length_output)
{
    u16 length;
    PublishVendorWindowAddress(source_output, 0xdd2fu);
    length = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) * 8u + 63u);
    WriteNativeWord(length_output, length);
}

/* RAM:fcb963 */
void HandleScsiOpcodeF7(void)
{
    SetNoSenseData();
}

/* RAM:fcb968 */
void HandleScsiOpcodeF8(void)
{
    SetNoSenseData();
}

/* RAM:fcb8c3 */
void HandleScsiOpcodeDE(void)
{
    u8 selector = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 7u);
    u8 response = 0u;

    if(selector == 1u || selector == 2u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 4u);

    if(selector == 1u || selector == 3u)
        response = (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018042) ^ 0xffu);
    else if(selector == 2u || selector == 4u)
        response = (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043) ^ 0xffu);
    else
    {
        if((FirmwareReadDirectSfr8(80) & 0x02u) == 0u)
            response |= 0x01u;
        if((FirmwareReadDirectSfr8(80) & 0x04u) == 0u)
            response |= 0x02u;
        if((FirmwareReadDirectSfr8(80) & 0x40u) == 0u)
            response |= 0x04u;
        if((FirmwareReadDirectSfr8(80) & 0x01u) == 0u)
            response |= 0x08u;
        if((FirmwareReadDirectSfr8(80) & 0x20u) == 0u)
            response |= 0x10u;
    }

    if(selector == 1u || selector == 2u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, response);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, (u8)(response ^ 0xffu));
    if(TransferVendorDeStatusResponse(FW_NEAR_UNKNOWN_0B2A, 2ul) != 4u)
        SetNoSenseData();
}

/* RAM:fbdc2d */
/* The explicit stack slots stage source pointers and byte counts; they are
 * not response arrays or input parameters. Source return instruction is RET. */
void PLEXTOR_NEAR_FUNCTION CopyControllerMeasurementSeries0123ToVendorF2Response(void)
{
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EF89, FW_NEAR_UNKNOWN_0B38, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFA7, FW_NEAR_UNKNOWN_0B56, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFC5, FW_NEAR_UNKNOWN_0B74, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFE3, FW_NEAR_UNKNOWN_0B92, 30u);
}

/* RAM:fbe138 */
/* No direct callers are recorded; source RET and DR12 preservation were
 * reviewed from instructions. Physical measurement meanings are unassigned. */
void CopyControllerMeasurementSeries456ToActiveWorkspace(void)
{
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F001, FW_NEAR_UNKNOWN_0BB0, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F01F, FW_NEAR_UNKNOWN_0BCE, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F03F, FW_NEAR_UNKNOWN_0BEC, 30u);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0C0C, ReadSummaryWord(FW_NEAR_UNKNOWN_F060));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0C10, ReadSummaryWord(FW_NEAR_UNKNOWN_F062));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0C14, ReadSummaryWord(FW_NEAR_UNKNOWN_F064));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0C18, ReadSummaryWord(FW_NEAR_UNKNOWN_EF85));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0C1A, ReadSummaryWord(FW_NEAR_UNKNOWN_EF87));
}

/* RAM:fbdb32 */
void PLEXTOR_NEAR_FUNCTION BuildVendorF2Mode1ControllerMeasurementResponse(void)
{
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EF89, FW_NEAR_UNKNOWN_0BD2, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFA7, FW_NEAR_UNKNOWN_0BF0, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFC5, FW_NEAR_UNKNOWN_0C0E, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_EFE3, FW_NEAR_UNKNOWN_0C2C, 30u);
    CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_F03F, FW_NEAR_UNKNOWN_0C4A, 30u);
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0C68, ReadSummaryWord(FW_NEAR_UNKNOWN_EF85));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_0C6A, ReadSummaryWord(FW_NEAR_UNKNOWN_EF87));
}
