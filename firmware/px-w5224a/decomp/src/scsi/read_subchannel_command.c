#include "read_subchannel_command.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "read_subchannel_current_position.h"
#include "read_subchannel_identification_acquisition.h"
#include "read_subchannel_isrc_f1.h"

extern void LoadTrackInformationByNumber(u8 track_number);
extern void HandleUnsupportedScsiCommand(void);
extern void SetNoSenseData(void);
extern u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source, u32 byte_count);

static void WriteReadSubChannelWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u8 SwapReadSubChannelNibbles(u8 value)
{
    return (u8)((value << 4) | (value >> 4));
}

static u8 IsReadSubChannelTrackUsable(u8 track_number)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        return (u8)(track_number <= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
    return (u8)(track_number <= GetEffectiveLastTrackNumber());
}

static void AcquireReadSubChannelTrackIfRequired(u8 track_number)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        AcquireReadSubChannelIdentificationForTrack(track_number);
    }
    else
    {
        LoadTrackInformationByNumber(track_number);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 2u)
            AcquireReadSubChannelIdentificationForTrack(track_number);
    }
}

static void BuildReadSubChannelMcnResponse(void)
{
    u8 first_track;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 2u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79B) | 2u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
            first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
        else
            first_track = GetEffectiveFirstTrackNumber();
        AcquireReadSubChannelTrackIfRequired(first_track);
    }
    WriteReadSubChannelWord(FW_NEAR_UNKNOWN_0B2C, 20u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
    BuildReadSubChannelMediaCatalogNumberField(FW_NEAR_UNKNOWN_0B32);
}

static void BuildReadSubChannelF0Response(void)
{
    u8 source = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C);
    u8 value;

    WriteReadSubChannelWord(FW_NEAR_UNKNOWN_0B2C, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0xf0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, 0u);
    if((source & 0x40u) == 0u && (source & 0x3fu) != 0u)
        value = 0x3fu;
    else
        value = (u8)(source & 0x7fu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, value);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B34, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B35, 0u);
}

static void BuildReadSubChannelIsrcResponse(u8 format, u8 track_number)
{
    u8 control;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79B) | (format == 3u ? 4u : 8u)));
    AcquireReadSubChannelTrackIfRequired(track_number);
    WriteReadSubChannelWord(FW_NEAR_UNKNOWN_0B2C, 20u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, format);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        control = FirmwareReadNear8(FW_NEAR_UNKNOWN_F2B7 + (fw_addr_t)((u16)track_number * 6u));
    else
        control = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, SwapReadSubChannelNibbles(control));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
    BuildReadSubChannelIsrcOrF1PayloadC(format, FW_NEAR_UNKNOWN_0B32);
}

/* RAM:f98b03 */
void PLEXTOR_NEAR_FUNCTION HandleReadSubChannelCommand(void)
{
    u8 format;
    u8 track_number;
    u16 transfer_length;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DC8)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    format = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D);
    if(format != 1u && format != 2u && format != 3u && format != 0xf0u && format != 0xf1u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, GetScsiAudioStatusCode(1u));
    WriteReadSubChannelWord(FW_NEAR_UNKNOWN_0B2C, 0u);

    if(format == 1u || format == 2u || format == 0xf0u)
    {
        if(track_number != 0u)
        {
            HandleUnsupportedScsiCommand();
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) & 0x40u) != 0u)
        {
            if(format == 1u)
            {
                WriteReadSubChannelWord(FW_NEAR_UNKNOWN_0B2C, 12u);
                BuildReadSubChannelCurrentPositionPayloadC(FW_NEAR_UNKNOWN_0B2E);
            }
            else if(format == 2u)
            {
                BuildReadSubChannelMcnResponse();
            }
            else
            {
                BuildReadSubChannelF0Response();
            }
        }
    }
    else
    {
        if(track_number == 0u || IsReadSubChannelTrackUsable(track_number) == 0u)
        {
            HandleUnsupportedScsiCommand();
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) & 0x40u) != 0u)
            BuildReadSubChannelIsrcResponse(format, track_number);
    }

    transfer_length = MinU16(ReadBigEndianU16(FW_NEAR_UNKNOWN_0B11), (u16)(ReadBigEndianU16(FW_NEAR_UNKNOWN_0B2C) + 4u));
    if(CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, (u32)transfer_length) != 4u)
        SetNoSenseData();
}
