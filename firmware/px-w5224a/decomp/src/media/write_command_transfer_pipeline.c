#include "write_command_transfer_pipeline.h"
#include "../platform/arithmetic.h"
#include "../platform/controller_buffer_transfer.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_read_buffer_setup.h"
#include "../platform/status_mapping.h"
#include "../platform/write_controller_range_sequence.h"
#include "raw_read_frame_prefix_wait.h"
#include "raw_read_pipeline_reset.h"
#include "write_pipeline_q.h"
#include "write_pipeline_transfer_batch_poll.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/write_command_transfer_pipeline_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadWritePipelineU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteWritePipelineU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteWritePipelineU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 ComputeWritePipelineSlotCount(s32 target)
{
    s32 relative = target - 150l;
    s32 quotient = DIVIDE_S32(relative, 39l);

    return (u8)(MULTIPLY_U32((u32)quotient, 39ul) + 32ul - (u32)relative);
}

static void WaitForHostControllerTransfer(void)
{
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1596) == 0u)
        PIPELINE_YIELD();
}

static void InitializeWriteCommandPipelineState(void)
{
    u32 ring_slots;
    u32 target;
    u32 controller_base;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
    CLEAR_CONTROLLER_REGIONS();
    RESET_RAW_READ();
    WriteWritePipelineU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteWritePipelineU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteWritePipelineU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteWritePipelineU16(FW_NEAR_UNKNOWN_1292, 0u);
    ring_slots = ReadWritePipelineU32(FW_NEAR_UNKNOWN_156A);
    WriteWritePipelineU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slots - 1ul));
    WriteWritePipelineU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slots - 6ul));
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_126A, ReadWritePipelineU32(FW_NEAR_UNKNOWN_1562));
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_126E, 0ul);

    target = INITIALIZE_HOST_RANGE();
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_1598, target);
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_8000, target);
    WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5B4, target);
    WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5B8, target);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    INITIALIZE_RAW_READ();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 1u);
    controller_base = ReadWritePipelineU32(FW_NEAR_UNKNOWN_1572);
    FILL_CONTROLLER_RANGE(controller_base, 0x00024c00ul, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
    WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5C2, controller_base);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5CA, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5D0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5D1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5D2, 0u);
}

static void ResumeWriteCommandPipelineState(void)
{
    u32 target;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    target = INITIALIZE_HOST_RANGE();
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_1598, target);
    WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5B4, target);
}

static u8 FinishWriteCommandPipeline(u8 completion_status)
{
    s32 target;
    s32 previous_target;

    if(completion_status != 1u)
        return 0u;
    if(BEGIN_PIPELINE() == 0u)
        return 0u;

    target = (s32)ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598);
    previous_target = (s32)ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5AB);
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >= 0 && previous_target < target)
    {
        WriteWritePipelineU32(FW_NEAR_UNKNOWN_140A, (u32)target);
        WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5AB, (u32)target);
    }
    SET_NO_SENSE();
    return 1u;
}

static u8 TryRestartWritePipelineAtPrefix(u32 total_blocks, u32 completed_blocks, u8 deferred_transfer, u8 deferred_source_index)
{
    u8 table_index;

    if(total_blocks == completed_blocks)
        return 0u;

    for(table_index = 0u; table_index < 0x40u; ++table_index)
    {
        fw_addr_t offset = (fw_addr_t)table_index * 4ul;

        if(ReadWritePipelineU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490) + offset) != ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598) + 5ul)
            continue;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C9, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 0u);
        if(PREPARE_PREFIX(0u, 1u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C6, 0u);
            return 2u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 1u)
            PIPELINE_YIELD();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9F) == 2u)
        {
            REPORT_FAILURE(1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C6, 0u);
            return 2u;
        }

        WriteWritePipelineU32(FW_NEAR_UNKNOWN_1598, ReadWritePipelineU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84690) + offset));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C6, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5D2, (u8)((u8)total_blocks - (u8)completed_blocks));
        WriteWritePipelineU32(FW_NEAR_UNKNOWN_8000, ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598));
        WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5B4, ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598));
        if(deferred_transfer == 0u)
        {
            INITIALIZE_LAYOUT();
            START_TRANSFER(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D2), ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5C2));
            WaitForHostControllerTransfer();
        }
        else
        {
            u8 block_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D2);
            u32 controller_base = ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5C2);

            WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5D4, ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5D4) + block_count);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C6, block_count);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C7, block_count);
            TRANSFER_SECTORS(controller_base + MULTIPLY_U32(deferred_source_index, 0x930ul), controller_base, block_count);
        }
        SET_NO_SENSE();
        return 1u;
    }
    return 0u;
}

/* RAM:fec121 */
u8 ExecuteWriteCommandTransferPipeline(u8 initialize_transfer)
{
    u8 completion_status = 0u;
    u8 pending_source_index = 0u;
    u8 pending_block_count = 0u;

    CLEAR_BIT30();
    WriteWritePipelineU32(FW_NEAR_UNKNOWN_0FB0, ReadWritePipelineU32(FW_NEAR_UNKNOWN_0FB4));
    if(initialize_transfer == 1u)
        InitializeWriteCommandPipelineState();
    else
        ResumeWriteCommandPipelineState();

    WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5D4, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    if(ReadWritePipelineU32(FW_NEAR_UNKNOWN_1455) == 0ul)
        completion_status = 1u;
    else
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0x28u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5DD, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5DE, 0u);

    for(;;)
    {
        u32 total_blocks = ReadWritePipelineU32(FW_NEAR_UNKNOWN_1455);
        u32 completed_blocks = ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5D4);
        u8 slot_count;
        u8 requested_count;
        u8 controller_mode;
        u32 controller_base;
        u32 controller_address;

        if(total_blocks <= completed_blocks)
            return FinishWriteCommandPipeline(completion_status);

        slot_count = ComputeWritePipelineSlotCount((s32)ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5D1, slot_count);
        if(slot_count > 0x20u)
        {
            WriteWritePipelineU32(FW_NEAR_UNKNOWN_1598, ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598) + 5ul);
            slot_count = ComputeWritePipelineSlotCount((s32)ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5D1, slot_count);
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5DD, 0u);
        requested_count = (u8)((u8)total_blocks - (u8)completed_blocks);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5D2, requested_count);
        controller_mode = (u8)(FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0) & 1u);
        if(slot_count < requested_count)
        {
            if(controller_mode == 0u)
            {
                requested_count = slot_count > 0x1fu ? 0x1fu : slot_count;
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5DD, 1u);
                if(requested_count > 0x20u)
                    requested_count = 0x20u;
            }
        }
        else if(controller_mode == 0u && requested_count > 0x1fu)
        {
            requested_count = 0x1fu;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5D2, requested_count);

        controller_base = ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5C2);
        controller_address = controller_base + MULTIPLY_U32(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C6), 0x930ul);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5DD) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5DE) == 0u)
            {
                INITIALIZE_LAYOUT();
                START_TRANSFER(requested_count, controller_address);
                WaitForHostControllerTransfer();
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C6, pending_block_count);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C7, pending_block_count);
                WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5D4, ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5D4) + pending_block_count);
                TRANSFER_SECTORS(controller_base + MULTIPLY_U32(pending_source_index, 0x930ul), controller_base, pending_block_count);
                WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5B4, ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598));
            }
        }
        else
        {
            u32 saved_completed = completed_blocks;

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5DE, 1u);
            pending_source_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C6);
            pending_block_count = requested_count;
            INITIALIZE_LAYOUT();
            START_TRANSFER(requested_count, controller_address);
            WaitForHostControllerTransfer();
            WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5D4, saved_completed + slot_count);
            FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C6, (u8)(slot_count + pending_source_index));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C7, FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C6));
            pending_block_count = (u8)(pending_block_count - slot_count);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5D2, slot_count);
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FAIL_PREPARATION() == 1u)
            return 0u;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D1) == FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D2))
        {
            u32 saved_target;
            u32 saved_probe_target;
            u8 restart_status;

            WriteWritePipelineU32(FW_HUGE_UNKNOWN_2BA5B4, ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5B4) + FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D2) - FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C7));
            do
            {
                completion_status = POLL_BATCH();
                if(completion_status == 0xfeu)
                    PIPELINE_YIELD();
            } while(completion_status == 0xfeu);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5CB, 0u);
            saved_target = ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598);
            saved_probe_target = ReadWritePipelineU32(FW_NEAR_UNKNOWN_8000);
            if(EXECUTE_RANGE() == 0u)
                return 0u;
            WriteWritePipelineU32(FW_NEAR_UNKNOWN_1598, saved_target);
            WriteWritePipelineU32(FW_NEAR_UNKNOWN_8000, saved_probe_target);

            restart_status = TryRestartWritePipelineAtPrefix(total_blocks, ReadWritePipelineU32(FW_HUGE_UNKNOWN_2BA5D4), FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5DE), pending_source_index);
            if(restart_status == 1u)
                return 1u;
            if(restart_status == 2u)
                return 0u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5C6, 0u);
        }
        else
        {
            WriteWritePipelineU32(FW_NEAR_UNKNOWN_1598, ReadWritePipelineU32(FW_NEAR_UNKNOWN_1598) + FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D2));
            completion_status = 1u;
        }
        PIPELINE_YIELD();
    }
}
