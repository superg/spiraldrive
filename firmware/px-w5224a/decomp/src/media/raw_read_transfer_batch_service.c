#include "raw_read_transfer_batch_service.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_read_capacity.h"
#include "../platform/status_mapping.h"
#include "../scsi/mmc_write.h"
#include "raw_read_controller_transfer_batch.h"
#include "raw_read_runtime.h"
#include "raw_read_track_record_advance.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_transfer_batch_service_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadTransferBatchU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadTransferBatchU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u16 FindPreviousType1RawTrackRecord(u16 record_index)
{
    do
    {
        record_index = (u16)(record_index - 1u);
    } while((FirmwareRead8((u16)(0xb4bdu + (u16)(record_index << 3))) & 0x0fu) != 1u);
    return record_index;
}

static u8 HandleTransferBatchProducerWait(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u && TRANSFER_TRY_RECOVER() == 0u)
            return 0u;
        if(TRANSFER_FAIL_PREPARATION() == 1u)
            return 0u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u)
    {
        TRANSFER_RESTART_PRODUCER();
    }
    return 1u;
}

static u8 WaitForFixedPacketProducerRelease(void)
{
    for(;;)
    {
        if(TRANSFER_TRY_RELEASE() != 0u)
            break;
        while(TRANSFER_SHOULD_WAIT() == 1u)
        {
            if(HandleTransferBatchProducerWait() == 0u)
                return 0u;
            TRANSFER_SERVICE_HOST();
            TRANSFER_YIELD();
        }
        TRANSFER_YIELD();
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && TRANSFER_FAIL_PREPARATION() == 1u)
        return 0u;
    return 1u;
}

static u8 PrepareTransferBatchTrackState(u16 *record_index)
{
    u8 comparison;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 2u)
        return 1u;
    comparison = TRANSFER_COMPARE((s32)ReadTransferBatchU32(FW_NEAR_UNKNOWN_1598), ReadTransferBatchU16(FW_NEAR_UNKNOWN_149B));
    if(comparison >= 2u)
    {
        TRANSFER_SEQUENCE_ERROR();
        return 0u;
    }
    *record_index = ReadTransferBatchU16(FW_NEAR_UNKNOWN_149B);
    if(comparison == 0u)
    {
        TRANSFER_BUILD_TDI(FW_NEAR_UNKNOWN_143C);
        TRANSFER_ADVANCE_TRACK();
        *record_index = ReadTransferBatchU16(FW_NEAR_UNKNOWN_149B);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14A9) == 0u)
        {
            TRANSFER_REFRESH_ISRC();
            *record_index = ReadTransferBatchU16(FW_NEAR_UNKNOWN_149B);
        }
        *record_index = FindPreviousType1RawTrackRecord(*record_index);
    }
    else
    {
        *record_index = FindPreviousType1RawTrackRecord(*record_index);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, TRANSFER_DECODE_Q(*record_index));
    }
    return 1u;
}

static u8 WaitForTransferBatchCompletion(u16 record_index, fw_addr_t controller_address, u8 *completed_frame_count)
{
    u16 poll_counter = 0x19u;

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) == 1u)
    {
        if(TRANSFER_SHOULD_WAIT() == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u)
            {
                TRANSFER_RESTART_PRODUCER();
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && TRANSFER_FAIL_PREPARATION() == 1u)
            {
                while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) == 1u)
                {
                    TRANSFER_SERVICE_HOST();
                    TRANSFER_YIELD();
                }
                return 0u;
            }
        }

        if(poll_counter > 0x1eu)
        {
            TRANSFER_YIELD();
            poll_counter = 0u;
        }
        else
        {
            poll_counter = (u16)(poll_counter + 1u);
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u && TRANSFER_TRY_RECOVER() == 0u)
        {
            while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) == 1u)
            {
                TRANSFER_SERVICE_HOST();
                TRANSFER_YIELD();
            }
            return 0u;
        }

        if((poll_counter & 7u) == 7u)
        {
            u16 controller_offset = (u16)((u16)FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018016) | ((u16)FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018017) << 8));
            if(controller_offset != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) == 0u)
            {
                u16 remaining = (u16)(ReadTransferBatchU16(FW_NEAR_UNKNOWN_1463) - (u16)*completed_frame_count - 1u);
                u16 threshold = (u16)(ReadTransferBatchU16(FW_NEAR_UNKNOWN_146D) * remaining);
                if(controller_offset < threshold)
                {
                    *completed_frame_count = (u8)(*completed_frame_count + 1u);
                    TRANSFER_ENQUEUE(record_index);
                }
            }
        }
    }

    return TRANSFER_FINALIZE(controller_address, record_index, *completed_frame_count);
}

/* RAM:fe5f15 */
u8 PLEXTOR_NEAR_FUNCTION ServiceRawReadTransferBatch(void)
{
    u8 status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    u16 block_count;
    u16 record_index = 0u;
    u8 completed_frame_count = 0u;
    fw_addr_t ring_address;
    fw_addr_t controller_address;

    if(status != 0u)
    {
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) == 1u)
            TRANSFER_YIELD();
        TRANSFER_SET_SENSE(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + status), ReadTransferBatchU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (fw_addr_t)status * 2u));
        TRANSFER_SET_INFORMATION(ReadTransferBatchU32(FW_NEAR_UNKNOWN_0B02) + ReadTransferBatchU32(FW_NEAR_UNKNOWN_1459));
        return 0u;
    }

    if(TRANSFER_SHOULD_WAIT() == 1u)
    {
        if(HandleTransferBatchProducerWait() == 0u)
            return 0u;
        return 0xfeu;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && TRANSFER_FAIL_PREPARATION() == 1u)
        return 0u;
    if(TRANSFER_TRY_RELEASE() == 0u)
        return 0xfeu;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 1u)
    {
        if((u16)(ReadTransferBatchU16(FW_NEAR_UNKNOWN_129C) - ReadTransferBatchU16(FW_NEAR_UNKNOWN_1463)) <= ReadTransferBatchU16(FW_NEAR_UNKNOWN_1292))
            return 0xfeu;
        if(TRANSFER_READ_BIT0028() != 0u && TRANSFER_READ_BIT0036() != 0u)
            return 0xfeu;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1465, 0u);
    block_count = TRANSFER_COMPUTE_COUNT();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1463, (u8)(block_count >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1464, (u8)block_count);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        if(TRANSFER_PREPARE_FIXED() == 0u)
            return 0u;
        if(WaitForFixedPacketProducerRelease() == 0u)
            return 0u;
    }

    if(PrepareTransferBatchTrackState(&record_index) == 0u)
        return 0u;
    if(block_count == 0u)
        return 0xfeu;

    ring_address = TRANSFER_RESUME_RING();
    TRANSFER_CONFIGURE(ring_address, block_count);
    controller_address = ReadTransferBatchU32(FW_NEAR_UNKNOWN_126E);
    if(WaitForTransferBatchCompletion(record_index, controller_address, &completed_frame_count) == 0u)
        return 0u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && TRANSFER_ADVANCE_PACKET() == 0u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        return 0u;
    if(ReadTransferBatchU16(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
    {
        TRANSFER_QUEUE_SENSE();
        return 0u;
    }
    if(ReadTransferBatchU32(FW_NEAR_UNKNOWN_1459) == ReadTransferBatchU32(FW_NEAR_UNKNOWN_1455))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1590, 2u);
        return 1u;
    }
    return 0xfeu;
}
