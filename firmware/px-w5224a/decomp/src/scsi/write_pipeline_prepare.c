#include "write_pipeline_prepare.h"
#include "../media/raw_read_pipeline_reset.h"
#include "../media/synthetic_write_descriptor_batch.h"
#include "../media/write_preservation_read.h"
#include "../platform/controller_record_index_range_acquisition.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_read_buffer_setup.h"
#include "../platform/status_mapping.h"
#include "write_record_prepare_predicate.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/write_pipeline_prepare_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadWritePrepareU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteWritePrepareU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteWritePrepareU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static s32 DivideWritePrepareTowardZero(s32 value, s32 divisor)
{
    if(value >= 0l)
        return value / divisor;
    return -(s32)(((u32)(-(value + 1l)) + 1ul) / (u32)divisor);
}

static void ResetWritePreparationRawReadRing(void)
{
    u32 slot_count = ReadWritePrepareU32(FW_NEAR_UNKNOWN_156A);

    WriteWritePrepareU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteWritePrepareU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteWritePrepareU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteWritePrepareU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteWritePrepareU16(FW_NEAR_UNKNOWN_129A, (u16)(slot_count - 1ul));
    WriteWritePrepareU16(FW_NEAR_UNKNOWN_129C, (u16)(slot_count - 6ul));
    WriteWritePrepareU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteWritePrepareU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteWritePrepareU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteWritePrepareU32(FW_NEAR_UNKNOWN_126A, ReadWritePrepareU32(FW_NEAR_UNKNOWN_1562));
    WriteWritePrepareU32(FW_NEAR_UNKNOWN_126E, 0ul);
}

static void AcquireMissingPrecedingControllerRecordIndex(void)
{
    s32 target = (s32)ReadWritePrepareU32(FW_NEAR_UNKNOWN_1598);
    s16 index = (s16)DivideWritePrepareTowardZero((s32)((u32)target - 145ul), 39l);
    fw_addr_t table;

    if(index <= 0)
        return;
    --index;
    table = ReadWritePrepareU32(FW_NEAR_UNKNOWN_FAFF);
    table = FirmwareAddressIncrementLow16(table, (u16)index);
    if(FirmwareRead8(table) != 0u)
        return;

    {
        u32 saved_boundary = ReadWritePrepareU32(FW_HUGE_UNKNOWN_2BA59F);

        ClearRawReadControllerBufferRegions();
        ResetRawReadPipelineBuffersAndState();
        ResetWritePreparationRawReadRing();
        (void)AcquireThirtyNineFrameControllerRecordIndexRange((u16)index, (u16)index, 0u);
        WriteWritePrepareU32(FW_HUGE_UNKNOWN_2BA59F, saved_boundary);
    }
}

/* RAM:fcc572 */
u8 PrepareWritePipelineForStartAddress(s32 start_address)
{
    u32 saved_command_start = ReadWritePrepareU32(FW_NEAR_UNKNOWN_0B02);
    s32 aligned_start;

    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xefu));
    WriteWritePrepareU32(FW_NEAR_UNKNOWN_1598, InitializeHostTransferRange());
    AcquireMissingPrecedingControllerRecordIndex();

    aligned_start = (s32)((u32)DivideWritePrepareTowardZero(start_address, 32l) << 5);
    if(start_address == aligned_start)
        return 1u;

    if(DoesWriteStartRequireControllerRecordPreparation(start_address, 0u) != 0u)
    {
        u8 saved_option = 0u;
        u8 option_was_cleared = 0u;
        u8 result;

        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x10u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA1, 1u);
        InitializeReadVerifyWorkingTrackState();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
        FillControllerBufferRange(0x00019940ul, 0ul, 0u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 1u) != 0u)
        {
            saved_option = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 0xfeu));
            option_was_cleared = 1u;
        }
        result = AcquireWritePreservationRawReadAtAddress(aligned_start, 0u);
        if(option_was_cleared != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3C, saved_option);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA1, 0u);
        WriteWritePrepareU32(FW_NEAR_UNKNOWN_0B02, saved_command_start);
        if(result == 0u)
        {
            SetSenseData(3u, 0x0c07u);
            return 0u;
        }
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
        FillControllerBufferRange(0x0000cca0ul, 0ul, 0u);
        ResetWritePreparationRawReadRing();
        WriteWritePrepareU32(FW_NEAR_UNKNOWN_0B02, (u32)(MAP_WRITE_PIPELINE_ADDRESS(aligned_start) - 150l));
        EnqueueThirtyTwoSyntheticWriteDescriptorsForStartAddress(aligned_start);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x10u));
        WriteWritePrepareU32(FW_NEAR_UNKNOWN_0B02, saved_command_start);
    }
    return 1u;
}
