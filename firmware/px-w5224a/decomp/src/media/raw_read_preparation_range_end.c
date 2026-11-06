#include "raw_read_preparation_range_end.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_preparation_range_end_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPreparationRangeEndU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadPreparationRangeEndU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePreparationRangeEndU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 IsOrdinaryPreparationTrackState(void)
{
    return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_14B7) != 1u);
}

static u8 MustUseFiveFramePreparationRange(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) == 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B3) == 1u)
        return 1u;
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B7) == 1u);
}

static s32 SelectCurrentPreparationRangeEnd(void)
{
    u32 first_count = ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_1430);
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(first_count == 0ul)
    {
        if(write_type == 0u)
        {
            if(IsOrdinaryPreparationTrackState() != 0u)
                return (s32)ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_1434);
            return 5l;
        }
        if(write_type == 2u)
            return FRAME_DISTANCE_TO_NEXT_TYPE1(ReadPreparationRangeEndU16(FW_NEAR_UNKNOWN_149B));
        if(write_type == 3u)
            return 1l;
        return (s32)ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_1434);
    }

    if(write_type == 1u)
        return (s32)(first_count + ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_1434));
    if(write_type == 2u)
        return FRAME_DISTANCE_TO_NEXT_TYPE1(ReadPreparationRangeEndU16(FW_NEAR_UNKNOWN_149B));
    if(write_type == 0u)
    {
        s32 range_end;

        if(IsOrdinaryPreparationTrackState() != 0u)
            range_end = (s32)(first_count + ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_1434));
        else
            range_end = 5l;
        if(MustUseFiveFramePreparationRange() != 0u)
            range_end = 5l;
        return range_end;
    }

    /* Nonzero segment zero is not paired with other write types natively. */
    return (s32)ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_1434);
}

static s32 ApplyPreparationRange150FramePartition(void)
{
    s32 range_end = (s32)(ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_DD3D) - ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_DD51));
    s32 track_distance = (s32)(ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_DD3D) - ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_DD39));
    s32 second_count;

    if(track_distance <= 150l || range_end <= 150l)
    {
        WritePreparationRangeEndU32(FW_NEAR_UNKNOWN_1434, 150ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 2u);
        WritePreparationRangeEndU32(FW_NEAR_UNKNOWN_1430, 0ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, 3u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD62) == 2u ? 1u : 3u);
        WritePreparationRangeEndU32(FW_NEAR_UNKNOWN_1434, 150ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 2u);
        WritePreparationRangeEndU32(FW_NEAR_UNKNOWN_1430, (u32)track_distance - 150ul);
    }

    second_count = (s32)ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_1434);
    if(range_end <= second_count)
    {
        WritePreparationRangeEndU32(FW_NEAR_UNKNOWN_1434, (u32)range_end);
    }
    else
    {
        second_count = (s32)ReadPreparationRangeEndU32(FW_NEAR_UNKNOWN_1434);
        WritePreparationRangeEndU32(FW_NEAR_UNKNOWN_1430, (u32)range_end - (u32)second_count);
    }
    return range_end;
}

/* RAM:fea45c */
s32 PLEXTOR_NEAR_FUNCTION SelectRawReadPreparationRangeEnd(u8 apply_150_frame_partition)
{
    s32 range_end = SelectCurrentPreparationRangeEnd();

    if(apply_150_frame_partition == 1u)
        range_end = ApplyPreparationRange150FramePartition();
    return range_end;
}
