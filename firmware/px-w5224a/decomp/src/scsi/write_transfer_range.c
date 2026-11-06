#include "write_transfer_range.h"
#include "../media/raw_read_frame_prefix_wait.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "../platform/write_controller_range_sequence.h"
#include "../platform/write_transfer_cursor_alignment.h"

extern void YieldCoroutine0(void);
extern u8 PreparePartialWriteControllerBufferRange(u32 block_count);

static u16 ReadWriteRangeU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadWriteRangeU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteWriteRangeU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static s32 DivideWriteRangeBy39TowardZero(s32 value)
{
    if(value >= 0l)
        return value / 39l;
    return -(s32)(((u32)(-(value + 1l)) + 1ul) / 39ul);
}

static void UpdateWriteControllerRangeCursor(void)
{
    WriteWriteRangeU32(FW_HUGE_UNKNOWN_2BA5B4, ReadWriteRangeU32(FW_HUGE_UNKNOWN_2BA5B4) + (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D2) - (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C7));
}

static u8 ExecutePendingWriteControllerRange(void)
{
    UpdateWriteControllerRangeCursor();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5CB, 1u);
    if(ExecuteWriteControllerBufferRangeSequence() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5CB, 0u);
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5CB, 0u);
    return 1u;
}

static u8 PrepareWriteRangeRawReadPrefix(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    WriteWriteRangeU32(FW_NEAR_UNKNOWN_14BC, 0x80000000ul);
    if(PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u) == 0u)
    {
        UpdateWorkingTrackAfterWriteFailure();
        return 0u;
    }
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
        YieldCoroutine0();
    return 1u;
}

/* RAM:fcc246 */
u8 ExecuteWriteTransferBlockRange(u32 block_count)
{
    u8 execute_prepared_range = 0u;
    u32 saved_pipeline_cursor;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 0u);
    if(ReadWriteRangeU16(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C6) != 0u)
    {
        u8 initialize_transfer = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x10u) != 0u);
        u8 aligned_or_different = IsWritePipelineAtAlignedCursorOrDifferentRecord();

        if((initialize_transfer == 0u && aligned_or_different != 0u) || (initialize_transfer != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C9) == 1u))
            block_count = AlignWriteTransferCursorForBlockCount(block_count);
        if(PreparePartialWriteControllerBufferRange(block_count) == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C8, 1u);
        WaitForWritePipelineStatusCompletion();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C8, 0u);
        if(initialize_transfer == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C9) == 1u || DoesTransferCrossNextThirtyTwoBoundary(ReadWriteRangeU32(FW_HUGE_UNKNOWN_2BC703)) == 0u)
            execute_prepared_range = 1u;
    }

    if(ReadWriteRangeU16(FW_NEAR_UNKNOWN_1292) != 0u)
    {
        s32 target = (s32)ReadWriteRangeU32(FW_NEAR_UNKNOWN_1598);
        s32 rounded = (s32)((u32)DivideWriteRangeBy39TowardZero((s32)((u32)target - 145ul)) * 39ul + 145ul);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
        WriteWriteRangeU32(FW_NEAR_UNKNOWN_1598, (u32)rounded);
        WriteWriteRangeU32(FW_NEAR_UNKNOWN_14BC, 0x80000000ul);
        if(PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u) == 0u)
        {
            UpdateWorkingTrackAfterWriteFailure();
            return 0u;
        }
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
            YieldCoroutine0();
        WriteWriteRangeU32(FW_NEAR_UNKNOWN_1598, (u32)target);
    }

    saved_pipeline_cursor = ReadWriteRangeU32(FW_NEAR_UNKNOWN_FB6E);
    if(execute_prepared_range != 0u && ExecutePendingWriteControllerRange() == 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C6) != 0u && execute_prepared_range == 0u)
    {
        block_count = AlignWriteTransferCursorForBlockCount(block_count);
        if(PreparePartialWriteControllerBufferRange(block_count) == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C8, 1u);
        WaitForWritePipelineStatusCompletion();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB69, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) | 1u));
        if(PrepareWriteRangeRawReadPrefix() == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C8, 0u);
        if(ExecutePendingWriteControllerRange() == 0u)
            return 0u;
    }

    WriteWriteRangeU32(FW_NEAR_UNKNOWN_FB6E, saved_pipeline_cursor);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u)
        return 0u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0x20u) != 0u)
    {
        WriteWriteRangeU32(FW_HUGE_UNKNOWN_2BC70B, ReadWriteRangeU32(FW_HUGE_UNKNOWN_2BC703) + ReadWriteRangeU32(FW_NEAR_UNKNOWN_0B06));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xdfu));
    }
    return 1u;
}
