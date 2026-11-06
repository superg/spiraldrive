#include "close_sync_raw_read_prepare.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "../scsi/send_opc_information.h"
#include "raw_read_acquisition_plan.h"
#include "raw_read_frame_prefix_wait.h"
#include "raw_read_pipeline_reset.h"
#include "raw_read_q_record_queue.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/close_sync_raw_read_prepare_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadCloseSyncDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteCloseSyncWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteCloseSyncDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 BuildCloseSyncQQueueWithFallback(void)
{
    if(BUILD_QUEUE() != 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 12u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
    if(RUN_OPC() == 0u)
    {
        REPORT_SENSE(3u, 0x7304u);
        return 0u;
    }
    if(BUILD_QUEUE() == 0u)
        return 0u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    return 1u;
}

static u8 ExecuteCloseSyncSpecialSequence(u16 expected_count)
{
    u8 result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 1u);
    WriteCloseSyncWord(FW_NEAR_UNKNOWN_8010, expected_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    result = EXECUTE_SEQUENCE(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 0u);
    return result;
}

static void InitializeCloseSyncRawReadState(void)
{
    u32 ring_slots;

    RESET_PIPELINE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1286, 0u);
    WriteCloseSyncWord(FW_NEAR_UNKNOWN_1298, 0u);
    WriteCloseSyncWord(FW_NEAR_UNKNOWN_1294, 0u);
    WriteCloseSyncWord(FW_NEAR_UNKNOWN_1296, 0u);
    WriteCloseSyncWord(FW_NEAR_UNKNOWN_1292, 0u);
    ring_slots = ReadCloseSyncDword(FW_NEAR_UNKNOWN_156A);
    WriteCloseSyncWord(FW_NEAR_UNKNOWN_129A, (u16)(ring_slots - 1ul));
    ring_slots = ReadCloseSyncDword(FW_NEAR_UNKNOWN_156A);
    WriteCloseSyncWord(FW_NEAR_UNKNOWN_129C, (u16)(ring_slots - 6ul));
    WriteCloseSyncDword(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteCloseSyncDword(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteCloseSyncDword(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteCloseSyncDword(FW_NEAR_UNKNOWN_126A, ReadCloseSyncDword(FW_NEAR_UNKNOWN_1562));
    WriteCloseSyncDword(FW_NEAR_UNKNOWN_126E, 0ul);
    FillControllerBufferRange(0ul, FW_HUGE_UNKNOWN_1B0000, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, SELECT_SECTOR_CLASS());
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1287, 1u);
}

static u8 FinishCloseSyncRawRead(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    if(PREPARE_PREFIX(0u, 1u) == 0u)
        return 0u;
    WriteCloseSyncDword(FW_NEAR_UNKNOWN_1488, ReadCloseSyncDword(FW_NEAR_UNKNOWN_1598));
    return 1u;
}

/* RAM:fe5189 */
u8 PrepareCloseTrackOrSyncRawReadState(void)
{
    u8 before_track_start = 0u;
    u8 track_type;
    s32 target;
    s32 start;
    s32 distance;
    u8 tuple[3];

    InitializeCloseSyncRawReadState();
    start = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD3D);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x40u) != 0u)
    {
        target = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD51);
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_1598, (u32)target);
        BUILD_Q(target - start, 0x1272ul);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        target = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD45);
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_1598, (u32)target);
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_8000, (u32)target);
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_1488, (u32)target);
        BUILD_Q(target - start, 0x1272ul);
    }
    else
    {
        target = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD4D);
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_1598, (u32)target);
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_8000, (u32)target);
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_1488, (u32)target);
        BUILD_Q(0l, 0x1272ul);
    }

    track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    if(track_type == 0u)
        return 1u;
    if(track_type == 1u)
    {
        distance = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_1598) - start;
        if(distance < 300l)
        {
            SET_MSF(distance, (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_1598));
            if(ACQUIRE_FRAMES(300l - distance) == 0u)
                return 0u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) != 0u && PREPARE_PREFIX(1u, 0u) == 0u)
                return 0u;
            if(PREPARE_PREFIX(0u, 1u) == 0u)
                return 0u;
        }
        else if(distance - 2l < 300l)
        {
            target = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_1598) + (distance == 300l ? 2l : 1l);
            WriteCloseSyncDword(FW_NEAR_UNKNOWN_1598, (u32)target);
            WriteCloseSyncDword(FW_NEAR_UNKNOWN_8000, (u32)target);
        }
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_1488, ReadCloseSyncDword(FW_NEAR_UNKNOWN_1598));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
        {
            u16 expected = ((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) > 0 && FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 1u) ? 20u : 10u;
            return ExecuteCloseSyncSpecialSequence(expected);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 0u);
        return BuildCloseSyncQQueueWithFallback();
    }

    if(track_type == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_128B, 0u);
        if(PREPARE_PLAN(0u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_128B, 0u);
            return 0u;
        }
    }
    else if(track_type == 4u)
    {
        goto finish_prefix;
    }
    else if(track_type == 6u)
    {
        WriteCloseSyncDword(FW_NEAR_UNKNOWN_8000, (u32)start);
        before_track_start = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD51) < start ? 1u : 0u;
        if(before_track_start != 0u)
        {
            if(PREPARE_PLAN(1u) == 0u)
            {
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x40u) != 0u)
                    REPORT_TARGET_SENSE();
                return 0u;
            }
            distance = 0l;
            WriteCloseSyncDword(FW_NEAR_UNKNOWN_1598, (u32)start);
        }
        else
        {
            target = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD51);
            distance = target - start;
            WriteCloseSyncDword(FW_NEAR_UNKNOWN_1598, (u32)target);
        }
        SET_MSF((s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_1598) - start, (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_1598));
        if(distance < 300l && ACQUIRE_FRAMES(300l - distance) == 0u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x40u) != 0u)
                REPORT_TARGET_SENSE();
            return 0u;
        }
        goto finish_prefix;
    }
    else if(track_type != 3u && track_type != 5u)
    {
        goto finish_prefix;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x40u) != 0u)
    {
        target = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD51);
        before_track_start = target < start ? 1u : 0u;
        if(before_track_start != 0u)
            target = start;
    }
    else if(track_type == 2u)
    {
        target = start;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        target = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD45);
    }
    else
    {
        target = (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD4D);
    }
    WriteCloseSyncDword(FW_NEAR_UNKNOWN_1598, (u32)target);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x40u) != 0u)
    {
        if(before_track_start != 0u && PREPARE_PLAN(1u) == 0u)
        {
            REPORT_TARGET_SENSE();
            return 0u;
        }
        BUILD_TUPLE(target - (s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD51), tuple);
        PUBLISH_TUPLE(0x1275u, tuple);
    }
    else
    {
        BUILD_TUPLE(target, tuple);
        PUBLISH_TUPLE(0x1279u, tuple);
    }
    distance = ((s32)ReadCloseSyncDword(FW_NEAR_UNKNOWN_DD41) - target) - 2l;
    if(distance < 0l)
    {
        REPORT_TARGET_SENSE();
        return 0u;
    }
    if(ACQUIRE_FRAMES(distance) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x40u) != 0u)
            REPORT_TARGET_SENSE();
        return 0u;
    }
finish_prefix:
    if(FinishCloseSyncRawRead() == 0u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x40u) == 0u)
        return 1u;

    if(track_type != 2u && track_type != 3u && track_type != 5u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
        {
            if(ExecuteCloseSyncSpecialSequence(10u) == 0u)
                return 0u;
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 0u);
            if(BuildCloseSyncQQueueWithFallback() == 0u)
                return 0u;
        }
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u && (track_type == 5u || track_type == 6u))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 3u);
        if(BuildCloseSyncQQueueWithFallback() == 0u)
            return 0u;
    }
    return 1u;
}
