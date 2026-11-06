#include "read_full_toc_response.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "../platform/type2_leadin_toc_sequence.h"

static u16 ReadFullTocResponseWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadFullTocControllerOffset(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteFullTocResponseWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static fw_addr_t FullTocResponseDescriptorAddress(u8 descriptor_index)
{
    return FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)((u16)descriptor_index * 11u);
}

static void ConvertFullTocByteToPackedBcd(fw_addr_t descriptor, u8 offset)
{
    FirmwareWrite8(descriptor + offset, BinaryByteToPackedBcd(FirmwareRead8(descriptor + offset)));
}

static void ConvertFullTocAdr1Descriptor(fw_addr_t descriptor, u8 point)
{
    if(point < 0xa0u)
    {
        ConvertFullTocByteToPackedBcd(descriptor, 3u);
        ConvertFullTocByteToPackedBcd(descriptor, 4u);
        ConvertFullTocByteToPackedBcd(descriptor, 5u);
        ConvertFullTocByteToPackedBcd(descriptor, 6u);
        FirmwareWrite8(descriptor + 7u, 0u);
        ConvertFullTocByteToPackedBcd(descriptor, 8u);
        ConvertFullTocByteToPackedBcd(descriptor, 9u);
        ConvertFullTocByteToPackedBcd(descriptor, 10u);
    }
    else if(point == 0xa0u)
    {
        ConvertFullTocByteToPackedBcd(descriptor, 4u);
        ConvertFullTocByteToPackedBcd(descriptor, 5u);
        ConvertFullTocByteToPackedBcd(descriptor, 6u);
        ConvertFullTocByteToPackedBcd(descriptor, 8u);
        ConvertFullTocByteToPackedBcd(descriptor, 10u);
    }
    else if(point == 0xa1u)
    {
        ConvertFullTocByteToPackedBcd(descriptor, 4u);
        ConvertFullTocByteToPackedBcd(descriptor, 5u);
        ConvertFullTocByteToPackedBcd(descriptor, 6u);
        ConvertFullTocByteToPackedBcd(descriptor, 8u);
    }
    else if(point == 0xa2u)
    {
        ConvertFullTocByteToPackedBcd(descriptor, 4u);
        ConvertFullTocByteToPackedBcd(descriptor, 5u);
        ConvertFullTocByteToPackedBcd(descriptor, 6u);
        ConvertFullTocByteToPackedBcd(descriptor, 8u);
        ConvertFullTocByteToPackedBcd(descriptor, 9u);
        ConvertFullTocByteToPackedBcd(descriptor, 10u);
    }
}

static void ConvertFullTocOtherAdrDescriptor(fw_addr_t descriptor, u8 point)
{
    if(point < 0xb0u)
    {
        ConvertFullTocByteToPackedBcd(descriptor, 3u);
        ConvertFullTocByteToPackedBcd(descriptor, 4u);
        ConvertFullTocByteToPackedBcd(descriptor, 5u);
        ConvertFullTocByteToPackedBcd(descriptor, 6u);
        ConvertFullTocByteToPackedBcd(descriptor, 8u);
        ConvertFullTocByteToPackedBcd(descriptor, 9u);
        ConvertFullTocByteToPackedBcd(descriptor, 10u);
    }
    else if(point == 0xb0u)
    {
        if(FirmwareRead8(descriptor + 4u) != 0xffu)
        {
            ConvertFullTocByteToPackedBcd(descriptor, 4u);
            ConvertFullTocByteToPackedBcd(descriptor, 5u);
            ConvertFullTocByteToPackedBcd(descriptor, 6u);
        }
        ConvertFullTocByteToPackedBcd(descriptor, 7u);
        ConvertFullTocByteToPackedBcd(descriptor, 8u);
        ConvertFullTocByteToPackedBcd(descriptor, 9u);
        ConvertFullTocByteToPackedBcd(descriptor, 10u);
    }
    else if(point >= 0xb1u && point <= 0xb4u)
    {
        ConvertFullTocByteToPackedBcd(descriptor, 4u);
        ConvertFullTocByteToPackedBcd(descriptor, 5u);
        ConvertFullTocByteToPackedBcd(descriptor, 6u);
        ConvertFullTocByteToPackedBcd(descriptor, 7u);
        ConvertFullTocByteToPackedBcd(descriptor, 8u);
        ConvertFullTocByteToPackedBcd(descriptor, 9u);
        ConvertFullTocByteToPackedBcd(descriptor, 10u);
    }
    else if(point == 0xc0u)
    {
        ConvertFullTocByteToPackedBcd(descriptor, 8u);
        ConvertFullTocByteToPackedBcd(descriptor, 9u);
        ConvertFullTocByteToPackedBcd(descriptor, 10u);
    }
}

static void PrepareFullTocResponseDescriptor(u8 descriptor_index, u8 use_packed_bcd_encoding)
{
    fw_addr_t source = FullTocResponseDescriptorAddress(descriptor_index);
    u8 control_adr;
    u8 point;

    CopyBytesWith8BitCount(FW_NEAR_UNKNOWN_0B2A, source, 11u);
    control_adr = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, (u8)((control_adr << 4) | (control_adr >> 4)));
    point = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D);
    if(use_packed_bcd_encoding == 0u)
        return;

    if((control_adr & 0x0fu) == 1u)
        ConvertFullTocAdr1Descriptor(FW_NEAR_UNKNOWN_0B2A, point);
    else
        ConvertFullTocOtherAdrDescriptor(FW_NEAR_UNKNOWN_0B2A, point);
}

static u8 FindFullTocSessionDescriptor(u8 requested_session, u16 *skipped_bytes)
{
    u8 descriptor_index = 1u;

    *skipped_bytes = 0u;
    while(FirmwareRead8(FullTocResponseDescriptorAddress(descriptor_index)) != requested_session)
    {
        descriptor_index = (u8)(descriptor_index + 1u);
        *skipped_bytes = (u16)(*skipped_bytes + 11u);
    }
    return descriptor_index;
}

/* RAM:f9a1fa */
void PLEXTOR_NEAR_FUNCTION ExecuteReadFullTocResponse(u8 use_packed_bcd_encoding)
{
    u8 saved_session_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9);
    u8 requested_session;
    u8 descriptor_index;
    u16 skipped_bytes;
    u16 remaining_descriptor_count;
    u16 response_length;
    u16 transfer_length;
    u16 streamed_descriptors;
    u32 controller_offset;

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u))
    {
        BuildType2LeadInTocRecordSequence(1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, 1u);
        WriteFullTocResponseWord(FW_NEAR_UNKNOWN_12CA, (u16)(ReadFullTocResponseWord(FW_NEAR_UNKNOWN_800C) * 11u));
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    requested_session = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10);
    if(requested_session == 0u)
        requested_session = 1u;
    if(requested_session > FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9))
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9));
    descriptor_index = FindFullTocSessionDescriptor(requested_session, &skipped_bytes);
    response_length = (u16)(ReadFullTocResponseWord(FW_NEAR_UNKNOWN_12CA) - skipped_bytes + 4u);
    WriteFullTocResponseWord(FW_NEAR_UNKNOWN_0B2A, (u16)(response_length - 2u));
    transfer_length = MinU16(ReadFullTocResponseWord(FW_NEAR_UNKNOWN_0B11), response_length);
    controller_offset = ReadFullTocControllerOffset(FW_NEAR_UNKNOWN_155A);

    if(transfer_length <= 4u)
    {
        (void)CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, (u32)transfer_length);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, saved_session_count);
        if(ExecuteControllerDataInAndMapStatus((u32)transfer_length) != 4u)
            SetNoSenseData();
        return;
    }

    controller_offset = CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, 4ul);
    transfer_length = (u16)(transfer_length - 4u);
    remaining_descriptor_count = (u16)((ReadFullTocResponseWord(FW_NEAR_UNKNOWN_12CA) - skipped_bytes) / 11u);
    streamed_descriptors = 0u;

    while(streamed_descriptors < remaining_descriptor_count)
    {
        u16 staged_length;

        PrepareFullTocResponseDescriptor(descriptor_index, use_packed_bcd_encoding);
        descriptor_index = (u8)(descriptor_index + 1u);
        staged_length = MinU16(transfer_length, 11u);
        controller_offset = CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, (u32)staged_length);
        if(transfer_length <= 11u)
            break;
        transfer_length = (u16)(transfer_length - 11u);
        streamed_descriptors = (u16)(streamed_descriptors + 1u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, saved_session_count);
    transfer_length = MinU16(ReadFullTocResponseWord(FW_NEAR_UNKNOWN_0B11), response_length);
    if(ExecuteControllerDataInAndMapStatus((u32)transfer_length) != 4u)
        SetNoSenseData();
}
