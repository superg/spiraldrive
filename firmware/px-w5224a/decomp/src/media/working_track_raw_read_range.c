#include "working_track_raw_read_range.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "raw_read_acquisition_plan.h"
#include "raw_read_frame_prefix_wait.h"
#include "raw_read_pipeline_reset.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/working_track_raw_read_range_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadWorkingRangeU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteWorkingRangeU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteWorkingRangeU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void InitializeWorkingTrackRawReadState(void)
{
    u32 slot_count;

    RANGE_RESET();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1286, 0u);
    WriteWorkingRangeU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteWorkingRangeU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteWorkingRangeU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteWorkingRangeU16(FW_NEAR_UNKNOWN_1292, 0u);
    slot_count = ReadWorkingRangeU32(FW_NEAR_UNKNOWN_156A);
    WriteWorkingRangeU16(FW_NEAR_UNKNOWN_129A, (u16)(slot_count - 1ul));
    WriteWorkingRangeU16(FW_NEAR_UNKNOWN_129C, (u16)(slot_count - 6ul));
    WriteWorkingRangeU32(FW_NEAR_UNKNOWN_129E, 0x88bbul);
    WriteWorkingRangeU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteWorkingRangeU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteWorkingRangeU32(FW_NEAR_UNKNOWN_126A, ReadWorkingRangeU32(FW_NEAR_UNKNOWN_1562));
    WriteWorkingRangeU32(FW_NEAR_UNKNOWN_126E, 0ul);
    RANGE_FILL(0ul, FW_HUGE_UNKNOWN_1B0000, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, RANGE_SECTOR_CLASS(0u, 0u));
}

static u8 PrepareWorkingTrackStart(s32 start, s32 requested, u8 *before)
{
    *before = (u8)(requested < start);
    if(*before == 1u && RANGE_PREPARE_PLAN(1u) == 0u)
    {
        RANGE_REPORT_FAILURE();
        return 0u;
    }
    WriteWorkingRangeU32(FW_NEAR_UNKNOWN_1598, (u32)(*before == 1u ? start : requested));
    return 1u;
}

/* RAM:fe345c */
u8 PrepareAndAcquireWorkingTrackRawReadRange(void)
{
    s32 start;
    s32 end;
    s32 requested;
    s32 target;
    s32 count;
    s32 aligned;
    u32 packet_size;
    u8 before;
    u8 track_specific = 0u;
    u8 result;

    InitializeWorkingTrackRawReadState();
    start = (s32)ReadWorkingRangeU32(FW_NEAR_UNKNOWN_DD3D);
    requested = (s32)ReadWorkingRangeU32(FW_NEAR_UNKNOWN_DD51);
    RANGE_SET_MSF((s32)((u32)requested - (u32)start), requested);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
    {
        RANGE_SEQUENCE_ERROR();
        return 0u;
    }

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) == 1u)
    {
        if(PrepareWorkingTrackStart(start, requested, &before) == 0u)
            return 0u;
        target = before == 1u ? start : requested;
        aligned = start - 5l;
        packet_size = ReadWorkingRangeU32(FW_NEAR_UNKNOWN_DD57);
        while((u32)(aligned + (s32)packet_size + 7l) <= (u32)target)
            aligned = (s32)((u32)aligned + packet_size + 7ul);
        count = (s32)(packet_size + (u32)aligned + 5ul - (u32)target);
        if(count < 0l)
        {
            end = (s32)((u32)aligned + packet_size * 2ul + 15ul);
            WriteWorkingRangeU32(FW_NEAR_UNKNOWN_8000, (u32)end);
            if((s32)ReadWorkingRangeU32(FW_NEAR_UNKNOWN_DD41) < end)
            {
                RANGE_REPORT_FAILURE();
                return 0u;
            }
            count = (s32)((u32)count + packet_size + 7ul);
        }
        else
        {
            WriteWorkingRangeU32(FW_NEAR_UNKNOWN_8000, (u32)aligned + packet_size + 8ul);
        }
        RANGE_SET_MSF((s32)((u32)target - (u32)start), target);
        if(RANGE_ACQUIRE(count) == 0u)
        {
            RANGE_REPORT_FAILURE();
            return 0u;
        }
    }
    else
    {
        if(PrepareWorkingTrackStart(start, requested, &before) == 0u)
            return 0u;
        target = (s32)ReadWorkingRangeU32(FW_NEAR_UNKNOWN_1598);
        if(ReadWorkingRangeU32(FW_NEAR_UNKNOWN_1484) == 0x80000000ul || before == 1u)
        {
            WriteWorkingRangeU32(FW_NEAR_UNKNOWN_8000, (u32)start);
        }
        else
        {
            end = (s32)ReadWorkingRangeU32(FW_NEAR_UNKNOWN_DD45);
            if(end < start)
                end = start;
            WriteWorkingRangeU32(FW_NEAR_UNKNOWN_8000, (u32)end);
        }
        RANGE_SET_MSF((s32)((u32)target - (u32)start), target);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u)
        {
            track_specific = 1u;
            count = (s32)((u32)ReadWorkingRangeU32(FW_NEAR_UNKNOWN_DD41) - (u32)target - 2ul);
            if(count < 0l)
            {
                RANGE_REPORT_FAILURE();
                return 0u;
            }
        }
        else if(ReadWorkingRangeU32(FW_NEAR_UNKNOWN_1484) == 0x80000000ul)
        {
            if(target < start)
                count = (s32)(ReadWorkingRangeU32(FW_NEAR_UNKNOWN_1430) + ReadWorkingRangeU32(FW_NEAR_UNKNOWN_1434) + 300ul - ((u32)target - (u32)start));
            else
                count = (s32)(300ul - ((u32)target - (u32)start));
        }
        else
        {
            count = 0l;
        }
        if(before != 1u && track_specific == 0u && count < 10l)
            count = 10l;
        if(RANGE_ACQUIRE(count) == 0u)
        {
            RANGE_REPORT_FAILURE();
            return 0u;
        }
        WriteWorkingRangeU32(FW_NEAR_UNKNOWN_8000, (u32)target + 3ul);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    result = RANGE_PREFIX(0u, 1u);
    if(result != 0u)
        WriteWorkingRangeU32(FW_NEAR_UNKNOWN_1488, (u32)target - 1ul);
    return result;
}
