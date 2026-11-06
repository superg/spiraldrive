#include "vendor_f5_response.h"
#include "../platform/arithmetic.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_f5_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. */
static u16 ReadNativeWord(fw_addr_t source)
{
    return FirmwareReadU16(source);
}

/* Reconstruction helper; no direct Ghidra address. */
static u32 ReadNativeDword(fw_addr_t source)
{
    u16 high_word = ReadNativeWord(source);
    u16 low_word = ReadNativeWord(FirmwareAddressIncrement24(source, 2u));

    return ((u32)high_word << 16) | low_word;
}

/* Reconstruction helper; no direct Ghidra address. */
static void WriteNativeWord(fw_addr_t destination, u16 value)
{
    FirmwareWriteU16(destination, value);
}

/* Reconstruction helper; no direct Ghidra address. */
static void WriteNativeDword(fw_addr_t destination, u32 value)
{
    WriteNativeWord(destination, (u16)(value >> 16));
    WriteNativeWord(FirmwareAddressIncrement24(destination, 2u), (u16)value);
}

/* RAM:fbe9bc */
void BuildVendorF5Selector09ControllerStateResponse(void)
{
    u8 index;
    u8 controller_word_high;
    u16 optional_word;
    u16 allocation_length;

    for(index = 0u; index < 0x37u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A + index, 0u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0x35u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, FirmwareReadNear8(FW_NEAR_UNKNOWN_1008));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, (u8)ReadNativeWord(FW_NEAR_UNKNOWN_F9A0));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, (u8)ReadNativeWord(FW_NEAR_UNKNOWN_F9A2));
    WriteNativeWord(FW_NEAR_UNKNOWN_0B34, ReadNativeWord(FW_NEAR_UNKNOWN_FC05));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B36, FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A6));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B37, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0A));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B38, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B39, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0C));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3A, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0D));

    controller_word_high = (u8)(ReadVendorF5Selector09ControllerWord(0x0309u) >> 8);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3C, controller_word_high);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1056));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3E, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3F, (u8)(ReadNativeWord(FW_NEAR_UNKNOWN_F990) >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B40, (u8)(ReadNativeWord(FW_NEAR_UNKNOWN_FBDE) >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B41, (u8)(ReadNativeWord(FW_NEAR_UNKNOWN_FBE0) >> 8));

    controller_word_high = (u8)(ReadVendorF5Selector09ControllerWord(0x0319u) >> 8);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B42, controller_word_high);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B43, (u8)(controller_word_high - FirmwareReadNear8(FW_NEAR_UNKNOWN_1056)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B44, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F));
    WriteNativeWord(FW_NEAR_UNKNOWN_0B46, ReadNativeWord(FW_NEAR_UNKNOWN_F99C));
    WriteNativeDword(FW_NEAR_UNKNOWN_0B48, ReadNativeDword(FW_NEAR_UNKNOWN_0CF5));
    WriteNativeWord(FW_NEAR_UNKNOWN_0B4C, ReadNativeWord(FW_NEAR_UNKNOWN_0CF9));
    WriteNativeWord(FW_NEAR_UNKNOWN_0B4E, ReadNativeWord(FW_NEAR_UNKNOWN_F984));

    optional_word = ReadNativeWord(FW_NEAR_UNKNOWN_F986);
    if(optional_word != 0u)
        optional_word = ReadNativeWord(FW_NEAR_UNKNOWN_F986);
    WriteNativeWord(FW_NEAR_UNKNOWN_0B50, optional_word);
    WriteNativeWord(FW_NEAR_UNKNOWN_0B52, ReadNativeWord(FW_NEAR_UNKNOWN_F988));
    optional_word = ReadNativeWord(FW_NEAR_UNKNOWN_F98A);
    if(optional_word != 0u)
        optional_word = ReadNativeWord(FW_NEAR_UNKNOWN_F98A);
    WriteNativeWord(FW_NEAR_UNKNOWN_0B54, optional_word);
    WriteNativeWord(FW_NEAR_UNKNOWN_0B56, ReadNativeWord(FW_NEAR_UNKNOWN_0FD5));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B58, FirmwareReadNear8(FW_NEAR_UNKNOWN_F99E));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B59, FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A9));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B5A, FirmwareReadNear8(FW_NEAR_UNKNOWN_F9AA));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B5B, FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A7));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B5C, FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B5D, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCB));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B5E, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCC));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B5F, FirmwareReadNear8(FW_NEAR_UNKNOWN_1157));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B60, FirmwareReadNear8(FW_NEAR_UNKNOWN_1158));

    allocation_length = MinU16(ReadNativeWord(FW_NEAR_UNKNOWN_0B12), 0x37u);
    if(TransferVendorF5Selector09Response(FW_NEAR_UNKNOWN_0B2A, (u32)allocation_length) != 4u)
        SetNoSenseData();
}
