#include "write_command_raw_read_pipeline.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "../scsi/mmc_write.h"
#include "current_mode_raw_read_pipeline.h"
#include "raw_read_transfer_batch_service.h"
#include "write_command_raw_read_prefix.h"
#include "write_pipeline_finalize.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/write_command_raw_read_pipeline_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadWriteRawPipelineU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadWriteRawPipelineU32(fw_addr_t address)
{
    u32 high = ReadWriteRawPipelineU16(address);
    return (high << 16) | ReadWriteRawPipelineU16(address + 2ul);
}

static void WriteWriteRawPipelineU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteWriteRawPipelineU32(fw_addr_t address, u32 value)
{
    WriteWriteRawPipelineU16(address, (u16)(value >> 16));
    WriteWriteRawPipelineU16(address + 2ul, (u16)value);
}

static void ReportWriteRawPipelineFailure(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u)
        WriteRawPipelineFailure(0u);
    else
        WriteRawPipelineQueueSense();
}

/* RAM:fe32f7 */
u8 InitializeOrContinueWriteCommandRawReadPipeline(u8 initialize_pipeline)
{
    u8 batch_status;
    u32 target_address;

    CopyWriteRawPipelineModeBit();
    WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_0FB0, ReadWriteRawPipelineU32(FW_NEAR_UNKNOWN_0FB4));

    if(initialize_pipeline != 0u)
    {
        u32 ring_slot_count;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
        WriteRawPipelineFill(FW_HUGE_UNKNOWN_1B0000, 0ul, 0u);
        WriteRawPipelineReset();
        WriteWriteRawPipelineU16(FW_NEAR_UNKNOWN_1298, 0u);
        WriteWriteRawPipelineU16(FW_NEAR_UNKNOWN_1294, 0u);
        WriteWriteRawPipelineU16(FW_NEAR_UNKNOWN_1296, 0u);
        WriteWriteRawPipelineU16(FW_NEAR_UNKNOWN_1292, 0u);
        ring_slot_count = ReadWriteRawPipelineU32(FW_NEAR_UNKNOWN_156A);
        WriteWriteRawPipelineU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
        WriteWriteRawPipelineU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_1262, 0ul);
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_1266, 0ul);
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_126A, ReadWriteRawPipelineU32(FW_NEAR_UNKNOWN_1562));
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_126E, 0ul);
        SetWriteRawPipelineCaptureBit();
        target_address = WriteRawPipelineInitializeRange();
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_1598, target_address);
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_8000, target_address);
        if(WriteRawPipelineInitializeMode() == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    }
    else
    {
        if(ReadWriteRawPipelineU16(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
        {
            ReportWriteRawPipelineFailure();
            return 0u;
        }
        if(WriteRawPipelineContinuation() == 0u)
        {
            ReportWriteRawPipelineFailure();
            return 0u;
        }
        target_address = WriteRawPipelineInitializeRange();
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_1598, target_address);
        if(WriteRawPipelinePreparePrefix() == 0u)
            return 0u;
    }

    WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_1459, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1461, 1u);
    if(initialize_pipeline == 1u && ReadWriteRawPipelineU32(FW_NEAR_UNKNOWN_0B06) == 0ul)
    {
        batch_status = 1u;
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) != 0x41u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x20u);
        do
        {
            batch_status = WriteRawPipelineServiceBatch();
            if(batch_status == 0xfeu)
                WriteRawPipelineYield();
        } while(batch_status == 0xfeu);
    }

    if(batch_status != 1u || WriteRawPipelineFinalize() == 0u)
        return 0u;
    WriteRawPipelineNoSense();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
        WriteWriteRawPipelineU32(FW_NEAR_UNKNOWN_140A, ReadWriteRawPipelineU32(FW_NEAR_UNKNOWN_1598));
    return 1u;
}
