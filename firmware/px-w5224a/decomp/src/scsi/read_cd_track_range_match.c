#include "read_cd_track_range_match.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_cd_track_range_match_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra addresses. */
static s32 ReadTrackRangeS32(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static u8 RawTrackRecordMatches(u8 track_number, fw_addr_t record_base)
{
    return FirmwareRead8(record_base + (u16)track_number * 10u) == track_number;
}

static s32 BuildTrackUpperBoundary(u8 track_number, u8 last_track_number, u16 frame_bias, s16 *latched_adjustment)
{
    s32 boundary;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && RawTrackRecordMatches(track_number, FW_HUGE_UNKNOWN_2BB047) != 0u)
    {
        if((u16)track_number + 1u > last_track_number)
        {
            boundary = ReadTrackRangeS32(FW_NEAR_CURRENT_FRAME_POSITION);
        }
        else
        {
            boundary = ConvertBinaryMsfToAdjustedFramePosition(FW_HUGE_UNKNOWN_2BB04C + (u16)track_number * 10u);
            if((FirmwareRead8(FW_HUGE_UNKNOWN_2BB045 + (u16)track_number * 10u) & 0x40u) == 0u)
                *latched_adjustment = (s16)frame_bias;
        }
    }
    else
    {
        boundary = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_F2BF + (u16)track_number * 6u);
        if((u16)track_number + 1u <= last_track_number && (FirmwareRead8(FW_HUGE_UNKNOWN_00F2BD + (u16)track_number * 6u) & 0x40u) == 0u)
            *latched_adjustment = (s16)frame_bias;
    }

    if(boundary == 0)
        return (s32)((u32)ReadTrackRangeS32(FW_NEAR_CURRENT_FRAME_POSITION) - frame_bias);
    return (s32)((u32)boundary - frame_bias + (u32)(s32)*latched_adjustment);
}

static u8 SelectedTrackHasDataControl(u8 track_number)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && RawTrackRecordMatches(track_number, FW_HUGE_UNKNOWN_2BB03D) != 0u)
        return (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_2BB03B + (u16)track_number * 10u) & 0x40u);
    return (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_00F2B7 + (u16)track_number * 6u) & 0x40u);
}

/* RAM:fd3716 */
u8 MatchReadCdFramePositionToTrackRange(s32 target_frame_position, u8 first_track_number, u8 last_track_number, u16 frame_bias)
{
    u8 track_number = first_track_number;
    u8 selected_track;
    s16 latched_adjustment = 0;
    s32 upper_boundary = 0;
    s32 continuation_threshold;

    while(track_number <= last_track_number)
    {
        upper_boundary = BuildTrackUpperBoundary(track_number, last_track_number, frame_bias, &latched_adjustment);
        if(target_frame_position < upper_boundary)
            break;
        track_number++;
    }

    selected_track = track_number <= last_track_number ? track_number : last_track_number;
    if(frame_bias == 0x0096u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E9, selected_track);
        continuation_threshold = (s32)((u32)upper_boundary - 0x96ul - (u32)(s32)latched_adjustment);
        if(target_frame_position < continuation_threshold)
        {
            if(READ_TRACK_RANGE_BIT(FW_BIT_UNKNOWN_0011) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
            WRITE_TRACK_RANGE_BIT(FW_BIT_UNKNOWN_0010, 0u);
            WRITE_TRACK_RANGE_BIT(FW_BIT_UNKNOWN_0011, 0u);
        }
        else
        {
            WRITE_TRACK_RANGE_BIT(FW_BIT_UNKNOWN_0011, 1u);
        }
    }

    return SelectedTrackHasDataControl(selected_track) == 0u;
}
