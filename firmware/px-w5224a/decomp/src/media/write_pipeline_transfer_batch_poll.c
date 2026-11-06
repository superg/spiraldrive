#include "write_pipeline_transfer_batch_poll.h"
#include "../platform/arithmetic.h"
#include "../platform/controller_buffer_atomic.h"
#include "../platform/controller_buffer_transfer.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/raw_read_capacity.h"
#include "../platform/status_mapping.h"
#include "raw_read_frame_sequence.h"
#include "raw_read_runtime.h"
#include "write_pipeline_q.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/write_pipeline_transfer_batch_poll_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBatchU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadBatchU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBatchU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 ComputeWriteBatchSlotOffset(s32 target_address)
{
    s32 relative_address = target_address - 150l;
    s32 quotient = DIVIDE_S32(relative_address, 39l);
    u32 block_start = MULTIPLY_U32((u32)quotient, 39ul);

    return (u8)(block_start + 32ul - (u32)relative_address);
}

static void ReconcileActiveWriteBatch(void)
{
    u32 saved_target;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u || ReadBatchU16(FW_NEAR_UNKNOWN_FBB4) >= 0x50u)
        return;

    if(ReadBatchU16(FW_NEAR_UNKNOWN_1292) > 5u)
    {
        u16 descriptor_address;
        u16 target_high_address;

        saved_target = ReadBatchU32(FW_NEAR_UNKNOWN_1598);
        descriptor_address = LATEST_DESCRIPTOR();
        target_high_address = ReadBatchU16((u16)(descriptor_address + 2u));
        WriteBatchU32(FW_NEAR_UNKNOWN_1598, (((u32)ReadBatchU16(target_high_address) << 16) | descriptor_address) + 1ul);
        (void)ACQUIRE_PREFIX(2u, 4u);
        WriteBatchU32(FW_NEAR_UNKNOWN_1598, saved_target);
    }

    WriteBatchU32(FW_NEAR_UNKNOWN_8000, ReadBatchU32(FW_NEAR_UNKNOWN_1598));
    WriteBatchU32(FW_NEAR_UNKNOWN_14BC, 0x80000000ul);
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
        YIELD_BATCH();
}

static u8 PollWriteBatchCapacity(void)
{
    if(SHOULD_WAIT() == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u && RECOVER_FAILURE() == 0u)
            return 0u;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u)
        {
            if(RESTART_ACQUISITION() == 0u)
                return 0u;
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FAIL_PREPARATION() == 1u)
        {
            return 0u;
        }
        return 0xfeu;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FAIL_PREPARATION() == 1u)
        return 0u;
    if(RELEASE_WAIT() == 0u)
        return 0xfeu;
    return 1u;
}

static fw_addr_t InitializeWriteBatchDescriptorRun(void)
{
    BUILD_Q();
    COPY_Q_MSF();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 2u);
    return RESUME_RING();
}

static void EmitWriteBatchControlDescriptors(fw_addr_t controller_address, u16 first_index, u16 end_index_exclusive, u8 wait_for_capacity, u8 packed_msf[3])
{
    u16 index = first_index;

    while(index < end_index_exclusive)
    {
        if(wait_for_capacity != 0u && WAIT_CAPACITY() == 0u)
            LATCH_FAILURE();
        WRITE_REGISTER_BLOCK(FW_NEAR_UNKNOWN_1428);
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
        SUBMIT_COMMAND(controller_address, 0u);
        UPDATE_Q_CONTROL();
        ENQUEUE_DESCRIPTOR();
        WriteBatchU32(FW_NEAR_UNKNOWN_1598, ReadBatchU32(FW_NEAR_UNKNOWN_1598) + 1ul);
        INCREMENT_LOCAL_MSF(packed_msf);
        INCREMENT_PACKET_MSF();
        controller_address = ADVANCE_RING(controller_address);
        ADVANCE_Q_MSF();
        ++index;
    }
}

static void WriteWriteBatchAuxiliaryRecord(fw_addr_t controller_address, const u8 packed_msf[3])
{
    fw_addr_t destination = FirmwareAddressIncrement24(controller_address, FW_HUGE_UNKNOWN_10000C);
    u32 subheader = ReadBatchU32(FW_NEAR_UNKNOWN_0C71);

    FirmwareWrite8(destination, packed_msf[0]);
    FirmwareWrite8(destination + 1u, packed_msf[1]);
    FirmwareWrite8(destination + 2u, packed_msf[2]);
    FirmwareWrite8(destination + 3u, 2u);
    WriteBatchU32(destination + 4u, subheader);
    WriteBatchU32(destination + 8u, subheader);
    COPY_AUXILIARY_PREFIX(controller_address, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF967A), 12u);
}

static void EmitWriteBatchPayloadDescriptors(u8 packed_msf[3])
{
    u8 index = 0u;

    BUILD_Q();
    while(index < 0x20u)
    {
        fw_addr_t controller_address = ReadBatchU32(FW_NEAR_UNKNOWN_126E);

        ENQUEUE_DESCRIPTOR();
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/write_pipeline_transfer_batch_poll_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        WriteWriteBatchAuxiliaryRecord(controller_address, packed_msf);
        WriteBatchU32(FW_NEAR_UNKNOWN_1598, ReadBatchU32(FW_NEAR_UNKNOWN_1598) + 1ul);
        WriteBatchU32(FW_NEAR_UNKNOWN_126E, ADVANCE_RING(controller_address));
        ADVANCE_Q_MSF();
        ++index;
    }
}

static fw_addr_t SelectWriteBatchTransferDestination(u8 block_count)
{
    if(block_count == 0x20u)
        return RESUME_RING();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C8) == 0u)
        return MULTIPLY_U32((u16)(0x25u - block_count), 0x930ul);
    return MULTIPLY_U32((u16)(0x25u - ComputeWriteBatchSlotOffset((s32)ReadBatchU32(FW_NEAR_UNKNOWN_8000))), 0x930ul);
}

static void RelocateWriteBatchControllerRange(u8 block_count)
{
    fw_addr_t destination = SelectWriteBatchTransferDestination(block_count);
    u32 ring_byte_count = MULTIPLY_U32(ReadBatchU32(FW_NEAR_UNKNOWN_156A), 0x930ul);
    u16 contiguous_count = (u16)DIVIDE_U32(ring_byte_count - destination, 0x930ul);
    fw_addr_t source = ReadBatchU32(FW_NEAR_UNKNOWN_1572);

    if(contiguous_count < (u16)block_count)
    {
        TRANSFER_SECTORS(source, destination, (u8)contiguous_count);
        TRANSFER_SECTORS(source + MULTIPLY_U32((u32)contiguous_count, 0x930ul), 0ul, (u8)(block_count - (u8)contiguous_count));
    }
    else
    {
        TRANSFER_SECTORS(source, destination, block_count);
    }
}

static void PrepareWriteBatchTarget(void)
{
    u8 slot_offset;
    u8 backup_count;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C8) == 0u)
    {
        backup_count = (u8)(0x25u - FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D2));
    }
    else
    {
        slot_offset = ComputeWriteBatchSlotOffset((s32)ReadBatchU32(FW_NEAR_UNKNOWN_1598));
        backup_count = (u8)(0x25u - slot_offset);
    }
    WriteBatchU32(FW_NEAR_UNKNOWN_1598, ReadBatchU32(FW_NEAR_UNKNOWN_1598) - backup_count);
}

/* RAM:fecb50 */
u8 PollWritePipelineTransferBatchStatus(void) PLEXTOR_REENTRANT
{
    u8 packed_msf[3];
    u8 capacity_status;
    u8 block_count;
    fw_addr_t controller_address;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1596) != 0u)
        return 0u;

    ReconcileActiveWriteBatch();
    capacity_status = PollWriteBatchCapacity();
    if(capacity_status != 1u)
        return capacity_status;

    PrepareWriteBatchTarget();
    controller_address = InitializeWriteBatchDescriptorRun();
    EmitWriteBatchControlDescriptors(controller_address, 0u, 5u, 0u, packed_msf);

    block_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5C6);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5CA) == 0u)
        RelocateWriteBatchControllerRange(block_count);
    else
        SEED_RANGE();
    EmitWriteBatchPayloadDescriptors(packed_msf);

    controller_address = InitializeWriteBatchDescriptorRun();
    EmitWriteBatchControlDescriptors(controller_address, 5u, 7u, 1u, packed_msf);
    WriteBatchU32(FW_NEAR_UNKNOWN_14BC, ReadBatchU32(FW_NEAR_UNKNOWN_1598));
    return 1u;
}
