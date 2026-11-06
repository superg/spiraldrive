#include "raw_read_preparation_segments_clone.h"
#include "firmware_memory.h"

static u16 ReadPreparationCloneU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadPreparationCloneU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePreparationCloneU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void SetPreparationCloneSegments(s32 first_count, u8 first_mode, s32 second_count, u8 second_mode)
{
    WritePreparationCloneU32(FW_NEAR_UNKNOWN_1430, (u32)first_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, first_mode);
    WritePreparationCloneU32(FW_NEAR_UNKNOWN_1434, (u32)second_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, second_mode);
}

/* RAM:faae3b */
void InitializeRawReadPreparationSegmentsFromPlanCodeClone(u8 read_plan_code)
{
    u32 reference_frame;
    u32 start_frame;
    s32 frame_distance;

    if(read_plan_code == 3u || read_plan_code == 4u)
        SetPreparationCloneSegments(75l, 3u, 150l, 2u);
    else if(read_plan_code == 5u || read_plan_code == 6u || read_plan_code == 8u)
        SetPreparationCloneSegments(0l, 0u, 150l, 2u);
    else if(read_plan_code == 7u || read_plan_code == 9u || read_plan_code == 10u)
        SetPreparationCloneSegments(0l, 0u, (s32)ReadPreparationCloneU16(FW_NEAR_UNKNOWN_0C4F), 1u);
    else if(read_plan_code == 11u)
        SetPreparationCloneSegments(0l, 0u, 5l, 2u);
    else if(read_plan_code == 2u)
        SetPreparationCloneSegments(75l, 1u, 150l, 2u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u
        && (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_14B3) == 1u
            || FirmwareReadNear8(FW_NEAR_UNKNOWN_14B7) == 1u))
    {
        SetPreparationCloneSegments(0l, 0u, 5l, (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) <= 4 ? 3u : 2u);
    }
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        SetPreparationCloneSegments(0l, 0u, 5l, 2u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 2u)
        return;

    reference_frame = ReadPreparationCloneU32(FW_NEAR_UNKNOWN_DD39);
    start_frame = ReadPreparationCloneU32(FW_NEAR_UNKNOWN_DD3D);
    frame_distance = (s32)(start_frame - reference_frame);
    if(frame_distance > 150l)
    {
        WritePreparationCloneU32(FW_NEAR_UNKNOWN_1430, (u32)(frame_distance - 150l));
        WritePreparationCloneU32(FW_NEAR_UNKNOWN_1434, 150ul);
        if(read_plan_code == 3u || read_plan_code == 4u || read_plan_code == 5u || read_plan_code == 6u || read_plan_code == 8u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 2u);
        }
        else if(read_plan_code == 7u || read_plan_code == 9u || read_plan_code == 10u)
        {
            WritePreparationCloneU32(FW_NEAR_UNKNOWN_1430, 0u);
            WritePreparationCloneU32(FW_NEAR_UNKNOWN_1434, (u32)frame_distance);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 1u);
        }
        else if(read_plan_code == 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 2u);
        }
        return;
    }

    WritePreparationCloneU32(FW_NEAR_UNKNOWN_1430, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, 0u);
    WritePreparationCloneU32(FW_NEAR_UNKNOWN_1434, (u32)frame_distance);
    if(read_plan_code == 2u || read_plan_code == 3u || read_plan_code == 4u || read_plan_code == 5u || read_plan_code == 6u || read_plan_code == 8u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 2u);
    else if(read_plan_code == 7u || read_plan_code == 9u || read_plan_code == 10u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 1u);
}
