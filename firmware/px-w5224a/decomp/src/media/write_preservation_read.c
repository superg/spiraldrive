#include "write_preservation_read.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "../scsi/track_packet_lba_mapping.h"
#include "fixed_packet_overhead.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/write_preservation_read_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadBigEndianU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ReadPreservationDescriptorFramePosition(u16 descriptor_address)
{
    u16 position_address = ReadBigEndianU16((fw_addr_t)(u16)(descriptor_address + 2u));
    u16 position_high = ReadBigEndianU16(position_address);

    return (s32)(((u32)position_high << 16) | descriptor_address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u8 ToPackedBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WritePackedBcdMsfForFramePosition(fw_addr_t destination, s32 frame_position)
{
    u32 position;
    u16 seconds;
    u16 frames;
    u32 minutes;

    if(frame_position < 0l)
        frame_position += 450000l;
    position = (u32)frame_position;
    frames = (u16)(position % 75ul);
    position /= 75ul;
    seconds = (u16)(position % 60ul);
    minutes = position / 60ul;
    FirmwareWrite8(destination, ToPackedBcd((u8)minutes));
    FirmwareWrite8(destination + 1u, ToPackedBcd((u8)seconds));
    FirmwareWrite8(destination + 2u, ToPackedBcd((u8)frames));
}

/* RAM:fccc53 */
u8 AcquireWritePreservationRawReadAtAddress(s32 target_address, u8 mapping_mode)
{
    s32 acquisition_target = target_address;
    u8 status;

    if(mapping_mode == 1u)
    {
        WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)acquisition_target);
    }
    else
    {
        s32 mapped_address;

        if(mapping_mode == 2u)
            mapped_address = MAP_TRACK_PACKET(target_address);
        else
            mapped_address = MAP_ALTERNATE(target_address);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_0B02, (u32)(mapped_address - 150l));
        acquisition_target = (s32)RESET_RAW_TRANSFER();
        WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)acquisition_target);
    }

    if(acquisition_target < 0l)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    PREPARE_HANDSHAKE_LOW();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u && FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u)
        SET_ACTIVE_BITS();

    RESET_RAW_QUEUE();
    INITIALIZE_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
    YIELD_COROUTINE0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F07, 0u);
    do
    {
        YIELD_COROUTINE0();
        status = POLL_DESCRIPTOR_BATCH();
    } while(status == 0xfeu);

    RESET_PIPELINE(acquisition_target, 2u);
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u)
        YIELD_COROUTINE0();

    CLEAR_ACTIVE_BITS_IF_SET();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    SET_HANDSHAKE_HIGH();
    return status;
}

/* RAM:fee0a0 */
u8 PollWritePreservationDescriptorBatch(void)
{
    u16 descriptor_address = GetLatestQueuedRawSectorDescriptorAddress();
    u16 queued_count;

    if(descriptor_address == 0u)
        return 0xfeu;

    queued_count = ReadBigEndianU16(FW_NEAR_UNKNOWN_1292);
    if(FirmwareReadBit0008() == 0u && queued_count < 0x20u)
    {
        u16 producer_index = (u16)(ReadBigEndianU16(FW_NEAR_UNKNOWN_1296) - 1u);
        s32 frame_position = ReadPreservationDescriptorFramePosition(descriptor_address);

        WriteBigEndianU16(FW_NEAR_UNKNOWN_1296, producer_index);
        WriteBigEndianU16(FW_NEAR_UNKNOWN_1292, (u16)(queued_count - 1u));
        ENQUEUE_PRESERVATION_RANGE((u32)producer_index * 0x930ul + 0x2df0ul, frame_position, (u8)(0x20u - (u8)producer_index));
        return 0u;
    }

    if(queued_count < 0x20u)
        return 0xfeu;

    {
        u16 cursor = ReadBigEndianU16(FW_NEAR_UNKNOWN_1294);
        u16 final_cursor = (u16)(cursor + 0x1fu);
        u8 result = 1u;

        POLL_BATCH_YIELD();
        WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, cursor);
        while(cursor <= final_cursor)
        {
            u16 status_record;

            descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
            status_record = ReadBigEndianU16((fw_addr_t)(u16)(descriptor_address + 0x0eu));
            if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u)) != 0u && status_record != 0x1804u && status_record != 0x1803u)
            {
                s32 frame_position = ReadPreservationDescriptorFramePosition(descriptor_address);

                ENQUEUE_PRESERVATION_RANGE((u32)cursor * 0x930ul + 0x2df0ul, frame_position, 1u);
                if(FirmwareRead8((fw_addr_t)(u16)(status_record + 0x13u)) != 0x0bu)
                    result = 0u;
            }
            cursor = (u16)(cursor + 1u);
            WriteBigEndianU16(FW_NEAR_UNKNOWN_1298, cursor);
        }
        return result;
    }
}

/* RAM:fee1da */
void PLEXTOR_NEAR_FUNCTION EnqueueWritePreservationDescriptorRange(u32 main_data_cursor, s32 frame_position, u8 descriptor_count)
{
    u8 index;

    WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)frame_position);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 0x0au);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, 0x71u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F30, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A3, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 2u);
    WritePackedBcdMsfForFramePosition(FW_NEAR_UNKNOWN_1428, frame_position);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 2u);
    WritePackedBcdMsfForFramePosition(FW_NEAR_UNKNOWN_1275, frame_position - 150l);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
    WritePackedBcdMsfForFramePosition(FW_NEAR_UNKNOWN_1279, frame_position);

    for(index = 0u; index < descriptor_count; ++index)
    {
        s32 current_position = (s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_1598);

        if((s8)FIXED_PACKET_OVERHEAD(current_position) < 1)
        {
            WRITE_PRESERVATION_BLOCK25(FW_NEAR_UNKNOWN_1428);
            SUBMIT_PRESERVATION_DESCRIPTOR(main_data_cursor, 0xaau);
            UPDATE_PRESERVATION_Q_CONTROL();
            ENQUEUE_CURRENT_PRESERVATION_DESCRIPTOR();
            main_data_cursor = ADVANCE_PRESERVATION_RING(main_data_cursor);
        }
        WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)(current_position + 1l));
        IncrementPackedBcdMsf(FW_NEAR_UNKNOWN_1428);
        AdvanceRawReadQPacketMsfTuples(0u);
    }
}
