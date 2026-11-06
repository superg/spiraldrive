#include "read_subchannel_identification_acquisition.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

extern void LoadTrackInformationByNumber(u8 track_number);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_subchannel_identification_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadSubChannelAcquisitionDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteSubChannelAcquisitionDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static fw_addr_t SubChannelTrackMsfAddress(u8 track_number)
{
    return FW_NEAR_UNKNOWN_F2B9 + (fw_addr_t)((u16)track_number * 6u);
}

static u8 IsReadSubChannelTerminalTrackType(void)
{
    u8 track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);

    return (u8)(track_type == 5u || track_type == 6u || track_type == 7u);
}

/* RAM:f9943d */
void PLEXTOR_NEAR_FUNCTION AcquireReadSubChannelIdentificationForTrack(u8 track_number)
{
    u8 saved_identification[10];
    u8 index;
    u8 effective_last_track;
    u32 target_position;
    u32 end_position = 0ul;
    u32 capture_end;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 8u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        target_position = (u32)ConvertBinaryMsfToAdjustedFramePosition(SubChannelTrackMsfAddress(track_number));
        if(track_number < FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
            end_position = (u32)ConvertBinaryMsfToAdjustedFramePosition(SubChannelTrackMsfAddress((u8)(track_number + 1u)));
        else
            end_position = ReadSubChannelAcquisitionDword(FW_NEAR_UNKNOWN_12C1);
    }
    else
    {
        effective_last_track = GetEffectiveLastTrackNumber();
        LoadTrackInformationByNumber(track_number);
        target_position = ReadSubChannelAcquisitionDword(FW_NEAR_UNKNOWN_DD3D);
        if(track_number < effective_last_track)
        {
            LoadTrackInformationByNumber((u8)(track_number + 1u));
            end_position = ReadSubChannelAcquisitionDword(FW_NEAR_UNKNOWN_DD3D);
            LoadTrackInformationByNumber(track_number);
        }
        else if(IsReadSubChannelTerminalTrackType() == 0u)
        {
            end_position = (u32)SelectWorkingTrackFramePosition();
        }
    }

    target_position += 75ul;
    WriteSubChannelAcquisitionDword(FW_NEAR_UNKNOWN_0C8E, target_position);
    (void)DispatchAndWaitForReadAcquisitionResult();
    for(index = 0u; index < 10u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F51E + index, 0u);

    CLEAR_READ_SUBCHANNEL_CAPTURE_BIT();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8D, 1u);
    capture_end = target_position + 220ul;
    if((s32)end_position > 150l)
    {
        end_position -= 150ul;
        if(end_position >= capture_end + 200ul)
            capture_end += 200ul;
        else if(end_position >= capture_end + 100ul)
            capture_end += 100ul;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        u32 lead_out = ReadSubChannelAcquisitionDword(FW_NEAR_UNKNOWN_12C1);

        if(capture_end >= lead_out)
            capture_end = lead_out;
    }
    else if(IsReadSubChannelTerminalTrackType() == 0u)
    {
        u32 selected_end = (u32)SelectWorkingTrackFramePosition();

        if(selected_end < capture_end)
            capture_end = selected_end;
    }
    WriteSubChannelAcquisitionDword(FW_NEAR_UNKNOWN_0C92, capture_end);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 0xf9u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79B) | 1u));
    ExecuteReadAcquisitionAndWaitForCompletion(0u);
    for(index = 0u; index < 10u; ++index)
        saved_identification[index] = FirmwareReadNear8(FW_NEAR_UNKNOWN_F51E + index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfdu));
    for(index = 0u; index < 10u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F51E + index, saved_identification[index]);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79B) & 0xf0u));
}
