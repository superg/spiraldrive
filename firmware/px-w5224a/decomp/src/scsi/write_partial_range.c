#include "write_partial_range.h"
#include "../media/synthetic_write_descriptor_batch.h"
#include "../media/write_preservation_read.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "write_record_prepare_predicate.h"

static u32 ReadPartialWriteU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WritePartialWriteU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WritePartialWriteU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static s32 DividePartialWriteBy32TowardZero(s32 value)
{
    if(value >= 0l)
        return value / 32l;
    return -(s32)(((u32)(-(value + 1l)) + 1ul) / 32ul);
}

static void ResetPartialWriteRawReadRing(void)
{
    u32 slot_count = ReadPartialWriteU32(FW_NEAR_UNKNOWN_156A);

    WritePartialWriteU16(FW_NEAR_UNKNOWN_1298, 0u);
    WritePartialWriteU16(FW_NEAR_UNKNOWN_1294, 0u);
    WritePartialWriteU16(FW_NEAR_UNKNOWN_1296, 0u);
    WritePartialWriteU16(FW_NEAR_UNKNOWN_1292, 0u);
    WritePartialWriteU16(FW_NEAR_UNKNOWN_129A, (u16)(slot_count - 1ul));
    WritePartialWriteU16(FW_NEAR_UNKNOWN_129C, (u16)(slot_count - 6ul));
    WritePartialWriteU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WritePartialWriteU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WritePartialWriteU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WritePartialWriteU32(FW_NEAR_UNKNOWN_126A, ReadPartialWriteU32(FW_NEAR_UNKNOWN_1562));
    WritePartialWriteU32(FW_NEAR_UNKNOWN_126E, 0ul);
}

/* RAM:fcc7de */
u8 PreparePartialWriteControllerBufferRange(u32 block_count)
{
    u32 saved_target = ReadPartialWriteU32(FW_NEAR_UNKNOWN_1598);
    s32 current = (s32)ReadPartialWriteU32(FW_HUGE_UNKNOWN_2BC703);
    s32 aligned = (s32)((u32)DividePartialWriteBy32TowardZero(current) << 5);
    s32 end = (s32)((u32)current + block_count);
    s32 next_boundary = (s32)((u32)aligned + 32ul);

    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xefu));
    if(end >= next_boundary)
        return 1u;

    if(DoesWriteStartRequireControllerRecordPreparation(current, 1u) != 0u)
    {
        u8 saved_write_mode_bit = 0u;
        u8 write_mode_was_changed = 0u;
        u8 saved_option = 0u;
        u8 option_was_cleared = 0u;
        u8 result;
        u8 desired_write_mode_bit;

        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x10u));
        desired_write_mode_bit = (u8)((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0x04u) >> 2);
        if((u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >> 7) & 1u) != desired_write_mode_bit)
        {
            saved_write_mode_bit = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >> 7) & 1u);
            desired_write_mode_bit = (u8)((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0x04u) << 5);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x7fu) | desired_write_mode_bit));
            write_mode_was_changed = 1u;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA1, 1u);
        InitializeReadVerifyWorkingTrackState();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
        FillControllerBufferRange(0ul, 0x00019940ul, 0u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 1u) != 0u)
        {
            saved_option = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 0xfeu));
            option_was_cleared = 1u;
        }

        result = AcquireWritePreservationRawReadAtAddress(aligned, 0u);
        if(option_was_cleared != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3C, saved_option);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA1, 0u);
        if(write_mode_was_changed != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x7fu) | (u8)(saved_write_mode_bit << 7)));
        WritePartialWriteU32(FW_NEAR_UNKNOWN_1598, saved_target);
        ResetPartialWriteRawReadRing();
        if(result == 0u)
        {
            SetSenseData(3u, 0x0c07u);
            return 0u;
        }
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
        FillControllerBufferRange(0ul, 0x0000cca0ul, 0u);
        ResetPartialWriteRawReadRing();
        DecrementPackedBcdMsf(0x1275u);
        DecrementPackedBcdMsf(0x1279u);
        EnqueueThirtyTwoWritePreservationDescriptors();
        ResetPartialWriteRawReadRing();
        WritePartialWriteU32(FW_NEAR_UNKNOWN_1598, saved_target);
    }
    return 1u;
}
