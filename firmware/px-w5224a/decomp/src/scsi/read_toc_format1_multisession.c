#include "read_toc_format1_multisession.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

static u16 ReadMultiSessionWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteMultiSessionWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteMultiSessionDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static fw_addr_t MultiSessionDescriptorAddress(u8 descriptor_index)
{
    return FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)((u16)descriptor_index * 11u);
}

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_toc_format1_multisession_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u8 FindLastUsableSessionDescriptor(u8 session_count)
{
    u8 descriptor_index = 1u;
    u8 previous_session = 1u;
    u8 wanted_session = session_count;

    while(FirmwareRead8(MultiSessionDescriptorAddress(descriptor_index)) != wanted_session)
    {
        u8 current_session = FirmwareRead8(MultiSessionDescriptorAddress(descriptor_index));

        if(previous_session == current_session)
        {
            descriptor_index = (u8)(descriptor_index + 1u);
        }
        else if((u16)(previous_session + 1u) == (u16)FirmwareRead8(MultiSessionDescriptorAddress(descriptor_index)))
        {
            previous_session = FirmwareRead8(MultiSessionDescriptorAddress(descriptor_index));
            descriptor_index = (u8)(descriptor_index + 1u);
        }
        else
        {
            wanted_session = (u8)(wanted_session - 1u);
            previous_session = 1u;
            descriptor_index = 1u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, wanted_session);
        }
    }
    return descriptor_index;
}

static void BuildActiveMultiSessionResponse(void)
{
    u16 response_length;
    u16 transfer_length;
    u8 high_media_flag;
    u8 msf_format;

    WriteBigEndianU16(FW_NEAR_UNKNOWN_0B2A, 10u);
    response_length = (u16)(ReadBigEndianU16(FW_NEAR_UNKNOWN_0B2A) + 2u);
    transfer_length = MinU16(ReadBigEndianU16(FW_NEAR_UNKNOWN_0B11), response_length);
    WriteMultiSessionWord(FW_NEAR_UNKNOWN_0B2C, 0x0101u);
    WriteMultiSessionDword(FW_NEAR_UNKNOWN_0B2E, 0x00170100ul);

    high_media_flag = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u);
    msf_format = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u);
    if(msf_format != 0u)
        if(high_media_flag != 0u)
            WriteMultiSessionDword(FW_NEAR_UNKNOWN_0B32, 0x00000200ul);
        else
            WriteMultiSessionDword(FW_NEAR_UNKNOWN_0B32, 0x0000163cul);
    else
        WriteMultiSessionDword(FW_NEAR_UNKNOWN_0B32, 0ul);

    if(CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, (u32)transfer_length) != 4u)
        SetNoSenseData();
}

/* RAM:f99f71 */
void PLEXTOR_NEAR_FUNCTION ExecuteReadTocFormat1MultiSessionResponse(void)
{
    u8 session_count;
    u8 descriptor_index;
    fw_addr_t descriptor;
    u16 transfer_length;

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        BuildActiveMultiSessionResponse();
        return;
    }

    session_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9);
    if(session_count == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    WriteMultiSessionWord(FW_NEAR_UNKNOWN_0B2A, 10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, session_count);
    descriptor_index = FindLastUsableSessionDescriptor(session_count);
    descriptor_index = (u8)(descriptor_index + 3u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 8u) != 0u && descriptor_index == 4u)
    {
        WriteMultiSessionDword(FW_NEAR_UNKNOWN_0B2E, 0x00140100ul);
        WriteMultiSessionDword(FW_NEAR_UNKNOWN_0B32, (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u ? 0x00000200ul : 0ul);
    }
    else
    {
        u8 control_adr;

        descriptor = MultiSessionDescriptorAddress(descriptor_index);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);
        control_adr = FirmwareRead8(descriptor + 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, (u8)((control_adr << 4) | (control_adr >> 4)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, FirmwareRead8(descriptor + 3u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, FirmwareRead8(descriptor + 8u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B34, FirmwareRead8(descriptor + 9u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B35, FirmwareRead8(descriptor + 10u));
        }
        else
        {
            WriteMultiSessionDword(FW_NEAR_UNKNOWN_0B32, (u32)ConvertMultiSessionMsf(descriptor + 8u) - 150ul);
        }
    }

    transfer_length = MinU16(ReadMultiSessionWord(FW_NEAR_UNKNOWN_0B11), 12u);
    if(CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, (u32)transfer_length) != 4u)
        SetNoSenseData();
}
