#include "raw_read_frame_sequence.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/raw_read_capacity.h"
#include "../platform/raw_read_q_control.h"
#include "../platform/raw_sector_capture.h"
#include "../platform/status_mapping.h"
#include "raw_read_runtime.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_frame_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRawReadFrameSequenceDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRawReadFrameSequenceDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u16 ReadRawReadFrameSequenceWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u16 PreviousActiveRawReadTrackRecord(void)
{
    u16 record_index = ReadRawReadFrameSequenceWord(FW_NEAR_UNKNOWN_149B);
    u16 record_address;

    do
    {
        record_index = (u16)(record_index - 1u);
        record_address = (u16)(0xb4bdu + (u16)(record_index << 3));
    } while((FirmwareReadNear8(record_address) & 0x0fu) != 1u);
    return record_index;
}

static u8 RawReadTrackRecordByte3(u16 record_index)
{
    return FirmwareRead8((u16)(0xb4bfu + (u16)(record_index << 3)));
}

/* RAM:fe3b86 */
void AcquireRawReadPrefixAtCurrentPipelineTarget(void)
{
    u16 record_index;
    u8 write_type;
    u8 q_type;
    u8 reverse_relative_msf;
    u8 capture_source_kind;
    u32 target_position;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_frame_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1284, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    ConvertFramePositionToBinaryMsfTuple((s32)ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598), target_tuple_address);
    ConvertThreeBinaryBytesToPackedBcd((u16)target_tuple_address);

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type == 0u || write_type == 1u)
    {
        (void)PREFIX_TARGET_SELECT_CURRENT_CLASS();
    }
    else if(write_type == 2u)
    {
        record_index = PreviousActiveRawReadTrackRecord();
        (void)PREFIX_TARGET_FIND_ADJACENT_CLASS(record_index, 0u);
    }

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type <= 3u && !(write_type == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1273) == 0xaau && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x0fu) == 1u))
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
        {
            record_index = PreviousActiveRawReadTrackRecord();
            q_type = PREFIX_TARGET_DECODE_Q_TYPE(record_index);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, q_type);
        }
        else
        {
            q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, q_type);
        }

        q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);
        if(q_type < 4u)
        {
            reverse_relative_msf = 0u;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
            {
                record_index = PreviousActiveRawReadTrackRecord();
                reverse_relative_msf = (u8)(RawReadTrackRecordByte3(record_index) == 0u);
            }
            if(PREFIX_TARGET_ACQUIRE_Q_UPDATES(3l, reverse_relative_msf) == 0u)
                return;

            target_position = ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598) - 1ul;
            WriteRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598, target_position);
            WriteRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_8000, target_position);
        }
        else
        {
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) != 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u || ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_14B8) != 0ul
                || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) != 1u)
            {
                capture_source_kind = 4u;
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
                {
                    record_index = PreviousActiveRawReadTrackRecord();
                    if(RawReadTrackRecordByte3(record_index) == 0u)
                        capture_source_kind = 6u;
                }
                if(PREFIX_TARGET_ACQUIRE_FIXED(1u, capture_source_kind) == 0u)
                    return;
            }

            capture_source_kind = 4u;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
            {
                record_index = PreviousActiveRawReadTrackRecord();
                if(RawReadTrackRecordByte3(record_index) == 0u)
                    capture_source_kind = 6u;
            }
            if(PREFIX_TARGET_ACQUIRE_FIXED(2u, capture_source_kind) == 0u)
                return;

            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) != 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
            {
                target_position = ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598) - 1ul;
                WriteRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598, target_position);
                WriteRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_8000, target_position);
            }
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B3, 0u);
}

/* RAM:feb0d3 */
u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadFrameSequenceWithQUpdates(s32 frame_count, u8 reverse_relative_msf)
{
    fw_addr_t controller_buffer_address;
    fw_addr_t controller_buffer_tail;
    s32 frame_index;
    u8 byte_index;

    if(frame_count < 0l)
        return 1u;

    UPDATE_RAW_READ_STATE();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, 0u);
    controller_buffer_address = RESUME_RAW_READ_RING();

    for(frame_index = 0l; frame_index < frame_count; frame_index++)
    {
        if(WAIT_FOR_RAW_READ_CAPACITY() == 0u)
        {
            REPORT_RAW_READ_FAILURE(0u);
            return 0u;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) != 1u)
        {
            if(CAPTURE_RAW_READ_SECTOR(controller_buffer_address, 6u) == 0u)
            {
                SET_RAW_READ_SENSE(4u, 0x4400u);
                SET_RAW_READ_INFORMATION(ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598));
                return 0u;
            }
            controller_buffer_tail = FirmwareAddressIncrement24(controller_buffer_address, FW_ADDRESS_BANK_STRIDE);
            for(byte_index = 0u; byte_index < 16u; byte_index++)
                FirmwareWrite8(controller_buffer_tail + byte_index, 0u);
        }

        UPDATE_RAW_READ_Q(0u, reverse_relative_msf);
        ENQUEUE_RAW_READ_DESCRIPTOR();
        ADVANCE_RAW_READ_MSF(reverse_relative_msf);
        WriteRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598, ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598) + 1ul);
        controller_buffer_address = ADVANCE_RAW_READ_RING(controller_buffer_address);
    }
    return 1u;
}

/* RAM:feabbe */
/* All canonical callers constrain prefix_mode to zero, one, or two. */
u8 PLEXTOR_NEAR_FUNCTION AcquireFixedRawReadFramePrefixWindow(u8 prefix_mode, u8 capture_source_kind)
{
    fw_addr_t controller_buffer_address;
    s16 frame_index;
    s16 end_index;
    u32 target_position;
    s32 working_track_start;
    u32 completed_blocks;
    u32 total_blocks;
    u8 reverse_relative_msf;
    u8 table_code;

    UPDATE_FIXED_PREFIX_STATE();
    switch(prefix_mode)
    {
    case 0u:
        frame_index = 0;
        end_index = 5;
        break;
    case 1u:
        frame_index = 5;
        end_index = 7;
        break;
    case 2u:
        frame_index = 0;
        end_index = 1;
        break;
    default:
        frame_index = 0;
        end_index = 0;
        break;
    }

    controller_buffer_address = RESUME_FIXED_PREFIX_RING();
    while(frame_index < end_index)
    {
        if(WAIT_FOR_FIXED_PREFIX_CAPACITY() == 0u)
        {
            REPORT_FIXED_PREFIX_FAILURE(0u);
            return 0u;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1405) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1403) == 1u)
        {
            COPY_FIXED_PREFIX_TUPLE(FW_NEAR_UNKNOWN_1428, 0x1498u);
            INCREMENT_FIXED_PREFIX_TUPLE(FW_NEAR_UNKNOWN_1428);
        }
        else
        {
            COPY_FIXED_PREFIX_TUPLE(FW_NEAR_UNKNOWN_1428, 0x1279u);
        }
        WRITE_FIXED_PREFIX_CONTROLLER_BLOCK(FW_NEAR_UNKNOWN_1428);

        if(prefix_mode == 0u || prefix_mode == 1u)
        {
            table_code = FirmwareRead8((fw_addr_t)(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F848F7) + (u16)frame_index));
            FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(((u8)(table_code << 5) & 0xe0u) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
        }
        else
        {
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, FirmwareReadNear8(FW_NEAR_UNKNOWN_142B));
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) != 1u && CAPTURE_FIXED_PREFIX_SECTOR(controller_buffer_address, capture_source_kind) == 0u)
        {
            SET_FIXED_PREFIX_SENSE(4u, 0x4400u);
            SET_FIXED_PREFIX_INFORMATION(ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598));
            return 0u;
        }

        POPULATE_FIXED_PREFIX_SUBHEADER(controller_buffer_address, capture_source_kind);
        UPDATE_FIXED_PREFIX_Q(0u, (u8)(capture_source_kind == 4u ? 0u : 1u));
        ENQUEUE_FIXED_PREFIX_DESCRIPTOR();

        if((prefix_mode == 1u && capture_source_kind != 6u && capture_source_kind != 5u) || (prefix_mode == 0u && capture_source_kind == 9u) || (prefix_mode == 2u && capture_source_kind == 4u))
        {
            reverse_relative_msf = 0u;
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
        {
            target_position = ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598);
            working_track_start = (s32)ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_DD3D);
            reverse_relative_msf = (u8)(working_track_start < (s32)target_position ? 0u : 1u);
        }
        else
        {
            reverse_relative_msf = 1u;
        }
        ADVANCE_FIXED_PREFIX_MSF(reverse_relative_msf);

        WriteRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598, ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598) + 1ul);
        controller_buffer_address = ADVANCE_FIXED_PREFIX_RING(controller_buffer_address);
        frame_index++;
    }

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && prefix_mode == 1u)
    {
        completed_blocks = ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1459);
        total_blocks = ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1455);
        if(total_blocks == completed_blocks)
            CLEAR_FIXED_PREFIX_COMPLETION_BIT();
        WriteRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_14BC, ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598));
    }
    return 1u;
}

/* RAM:fead99 */
u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadFrameSequenceWithCurrentQPacket(s32 frame_count, u8 capture_source_kind)
{
    fw_addr_t controller_buffer_address;
    s32 frame_index;
    u8 reverse_relative_msf;
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/media/raw_read_frame_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if(frame_count < 0l)
        return 1u;

    UPDATE_RAW_READ_STATE();
    controller_buffer_address = RESUME_RAW_READ_RING();
    for(frame_index = 0l; frame_index < frame_count; frame_index++)
    {
        if(WAIT_FOR_RAW_READ_CAPACITY() == 0u)
        {
            REPORT_RAW_READ_FAILURE(0u);
            return 0u;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1405) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1403) == 1u)
        {
            COPY_RAW_READ_TUPLE(FW_NEAR_UNKNOWN_1428, 0x1498u);
            INCREMENT_RAW_READ_TUPLE(FW_NEAR_UNKNOWN_1428);
        }
        else
        {
            COPY_RAW_READ_TUPLE(FW_NEAR_UNKNOWN_1428, 0x1279u);
        }
        WRITE_RAW_READ_CONTROLLER_BLOCK(FW_NEAR_UNKNOWN_1428);

        if(CAPTURE_RAW_READ_SECTOR(controller_buffer_address, capture_source_kind) == 0u)
        {
            SET_RAW_READ_SENSE(4u, 0x4400u);
            SET_RAW_READ_INFORMATION(ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598));
            return 0u;
        }

        POPULATE_RAW_READ_SUBHEADER(controller_buffer_address, capture_source_kind);
        reverse_relative_msf = (u8)(capture_source_kind == 4u ? 0u : 1u);
        UPDATE_RAW_READ_Q(0u, reverse_relative_msf);
        ENQUEUE_RAW_READ_DESCRIPTOR();
        ADVANCE_RAW_READ_MSF(reverse_relative_msf);
        WriteRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598, ReadRawReadFrameSequenceDword(FW_NEAR_UNKNOWN_1598) + 1ul);
        controller_buffer_address = ADVANCE_RAW_READ_RING(controller_buffer_address);
    }
    return 1u;
}

static u8 ProducerCapacityUsesFirstReadFailureContext(void)
{
    u8 status = FirmwareReadNear8(FW_NEAR_UNKNOWN_1281);

    if(status != 1u && status != 3u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1282) == 1u)
        return 1u;
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1283) == 2u);
}

static void ReportProducerCapacityFailure(u8 failure_code)
{
    if(ProducerCapacityUsesFirstReadFailureContext() != 0u)
        PRODUCER_CAPACITY_LATCH_FAILURE();
    else
        PRODUCER_CAPACITY_REPORT_FAILURE(failure_code);
}

static u16 ReadProducerCapacityU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* RAM:feaeca */
u8 WaitForRawReadProducerCapacityAndCheckStatus(void)
{
    u8 retry_count = 0u;
    u8 retry_limit;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1555) == 1u)
        return 1u;

    retry_limit = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu ? 0u : 2u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_88B6) == 1u) && (ReadProducerCapacityU16(FW_NEAR_UNKNOWN_1296) & 0x000fu) == 0u)
    {
        PRODUCER_CAPACITY_SERVICE_HOST();
        PRODUCER_CAPACITY_YIELD();
    }

    for(;;)
    {
        if(PRODUCER_CAPACITY_SHOULD_WAIT() != 1u)
            return 1u;

        PRODUCER_CAPACITY_SERVICE_HOST();
        PRODUCER_CAPACITY_YIELD();

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u
            && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 0u))
        {
            if(PRODUCER_CAPACITY_READ_BIT0028() == 1u || retry_count > retry_limit)
            {
                ReportProducerCapacityFailure(1u);
                return 0u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u && (PRODUCER_CAPACITY_READ_BIT0028() == 1u || retry_count > retry_limit))
            {
                ReportProducerCapacityFailure(1u);
                PRODUCER_CAPACITY_CLEAR_BIT0028();
                return 0u;
            }
            if(PRODUCER_CAPACITY_RESTART() == 0u)
                return 0u;
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u && (PRODUCER_CAPACITY_READ_BIT0028() == 1u || retry_count > retry_limit))
        {
            ReportProducerCapacityFailure(1u);
            return 0u;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, (u8)(PRODUCER_CAPACITY_READ_BIT0031() != 0u));
        }

        retry_count++;
        if(ReadProducerCapacityU16(FW_NEAR_UNKNOWN_1292) == 0u)
            break;
    }

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 1u)
    {
        PRODUCER_CAPACITY_SERVICE_HOST();
        PRODUCER_CAPACITY_YIELD();
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        PRODUCER_CAPACITY_REPORT_SENSE();
    else
        ReportProducerCapacityFailure(0u);
    return 0u;
}
