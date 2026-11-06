#include "read_formatted_toc_response.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

static u16 ReadFormattedTocWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadFormattedTocDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteFormattedTocWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteFormattedTocDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 SwapFormattedTocNibbles(u8 value)
{
    return (u8)((value << 4) | (value >> 4));
}

static fw_addr_t FormattedTocTrackRecordAddress(u8 track_number)
{
    return FW_NEAR_UNKNOWN_F2B6 + (fw_addr_t)((u16)track_number * 6u);
}

static u8 IsFormattedTocBatchBoundary(u8 track_number)
{
    while(track_number >= 20u)
        track_number = (u8)(track_number - 20u);
    return track_number == 0u;
}

static void WriteActiveLeadOutAddress(fw_addr_t destination)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_formatted_toc_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u)
    {
        WriteFormattedTocDword(destination, (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u ? 0x0000122bul : 0x00000400ul);
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u)
    {
        FirmwareWrite8(destination, 0u);
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_formatted_toc_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    }
    else
    {
        WriteFormattedTocDword(destination, ComputeAndPublishMediaCapacityGeometry());
    }
}

static void ExecuteActiveFormattedTocResponse(u8 start_track)
{
    u16 response_length;
    u16 transfer_length;

    if(start_track == 0xaau)
    {
        WriteFormattedTocWord(FW_NEAR_UNKNOWN_0B2A, 10u);
        response_length = 12u;
        WriteFormattedTocWord(FW_NEAR_UNKNOWN_0B2C, 0x0101u);
        WriteFormattedTocDword(FW_NEAR_UNKNOWN_0B2E, 0x0016aa00ul);
        WriteActiveLeadOutAddress(FW_NEAR_UNKNOWN_0B32);
    }
    else
    {
        WriteFormattedTocWord(FW_NEAR_UNKNOWN_0B2A, 18u);
        response_length = 20u;
        WriteFormattedTocWord(FW_NEAR_UNKNOWN_0B2C, 0x0101u);
        WriteFormattedTocDword(FW_NEAR_UNKNOWN_0B2E, 0x00170100ul);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u)
            WriteFormattedTocDword(FW_NEAR_UNKNOWN_0B32, (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u ? 0x00000200ul : 0ul);
        else
            WriteFormattedTocDword(FW_NEAR_UNKNOWN_0B32, (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u ? 0x0000163cul : 0ul);
        WriteFormattedTocDword(FW_NEAR_UNKNOWN_0B36, 0x0016aa00ul);
        WriteActiveLeadOutAddress(FW_NEAR_UNKNOWN_0B3A);
    }

    transfer_length = MinU16(ReadFormattedTocWord(FW_NEAR_UNKNOWN_0B11), response_length);
    if(CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, (u32)transfer_length) != 4u)
        SetNoSenseData();
}

static void BuildCachedFormattedTocDescriptor(u8 track_number, u8 workspace_offset)
{
    fw_addr_t source = FormattedTocTrackRecordAddress(track_number);
    fw_addr_t destination = FW_NEAR_UNKNOWN_0B2A + workspace_offset;

    FirmwareWrite8(destination, 0u);
    FirmwareWrite8(destination + 1u, SwapFormattedTocNibbles(FirmwareRead8(source + 1u)));
    FirmwareWrite8(destination + 2u, FirmwareRead8(source));
    FirmwareWrite8(destination + 3u, 0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u)
    {
        FirmwareWrite8(destination + 4u, 0u);
        FirmwareWrite8(destination + 5u, FirmwareRead8(source + 3u));
        FirmwareWrite8(destination + 6u, FirmwareRead8(source + 4u));
        FirmwareWrite8(destination + 7u, FirmwareRead8(source + 5u));
    }
    else
    {
        WriteFormattedTocDword(destination + 4u, (u32)ConvertBinaryMsfToAdjustedFramePosition(source + 3u) - 150ul);
    }
}

static void ExecuteCachedFormattedTocResponse(u8 requested_track)
{
    u8 first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    u8 last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
    u8 track_number;
    u8 workspace_offset = 0u;
    u16 response_length;
    u16 transfer_length;
    u16 copied_length;
    u32 controller_offset;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if(requested_track == 0xaau)
    {
        track_number = (u8)(last_track + 1u);
        WriteFormattedTocWord(FW_NEAR_UNKNOWN_0B2A, 10u);
        response_length = 12u;
    }
    else
    {
        if(requested_track == 0u)
            requested_track = first_track;
        if(requested_track > last_track)
        {
            HandleUnsupportedScsiCommand();
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 8u) != 0u && last_track >= 2u && requested_track <= 2u)
            requested_track = 2u;
        track_number = requested_track;
        WriteFormattedTocWord(FW_NEAR_UNKNOWN_0B2A, (u16)(((u16)last_track - requested_track) * 8u + 18u));
        response_length = (u16)(ReadFormattedTocWord(FW_NEAR_UNKNOWN_0B2A) + 2u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, first_track);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, last_track);
    transfer_length = MinU16(ReadBigEndianU16(FW_NEAR_UNKNOWN_0B11), response_length);
    controller_offset = ReadFormattedTocDword(FW_NEAR_UNKNOWN_155A);

    if(transfer_length <= 4u)
    {
        (void)CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, (u32)transfer_length);
        if(ExecuteControllerDataInAndMapStatus((u32)transfer_length) != 4u)
            SetNoSenseData();
        return;
    }

    controller_offset = CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, 4ul);
    copied_length = 4u;
    do
    {
        BuildCachedFormattedTocDescriptor(track_number, workspace_offset);
        if((u16)(transfer_length - 4u) < (u16)(workspace_offset + 8u))
            break;
        workspace_offset = (u8)(workspace_offset + 8u);
        if(IsFormattedTocBatchBoundary(track_number) != 0u)
        {
            controller_offset = CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, 160ul);
            copied_length = (u16)(copied_length + 160u);
            workspace_offset = 0u;
        }
        track_number = (u8)(track_number + 1u);
    } while((u16)track_number <= (u16)(last_track + 1u));

    (void)CopyToControllerBufferWindow(FW_NEAR_UNKNOWN_0B2A, controller_offset, (u32)(u16)(transfer_length - copied_length));
    if(ExecuteControllerDataInAndMapStatus((u32)transfer_length) != 4u)
        SetNoSenseData();
}

/* RAM:f99acf */
void PLEXTOR_NEAR_FUNCTION ExecuteReadFormattedTocResponse(void)
{
    u8 requested_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10);

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        ExecuteActiveFormattedTocResponse(requested_track);
    else
        ExecuteCachedFormattedTocResponse(requested_track);
}
