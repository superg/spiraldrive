#include "synthetic_write_descriptor_batch.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/synthetic_write_descriptor_batch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/synthetic_write_descriptor_batch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadSyntheticWriteDescriptorWord(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 DecodePackedBcdMsf(fw_addr_t address)
{
    u8 minute = PackedBcdByteToBinary(FirmwareRead8(address));
    u8 second = PackedBcdByteToBinary(FirmwareRead8(address + 1u));
    u8 frame = PackedBcdByteToBinary(FirmwareRead8(address + 2u));

    return ConvertBinaryMsfValuesToAdjustedFramePosition(minute, second, frame);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void EncodePackedBcdMsf(fw_addr_t address, s32 frame_position)
{
    ConvertFramePositionToBinaryMsfTuple(frame_position, address);
    FirmwareWrite8(address, BinaryByteToPackedBcd(FirmwareRead8(address)));
    FirmwareWrite8(address + 1u, BinaryByteToPackedBcd(FirmwareRead8(address + 1u)));
    FirmwareWrite8(address + 2u, BinaryByteToPackedBcd(FirmwareRead8(address + 2u)));
}

/* RAM:fede81 */
void EnqueueThirtyTwoSyntheticWriteDescriptorsForStartAddress(s32 start_address)
{
    u32 controller_buffer_cursor = RESUME_RING() + 0x2df0ul;
    s32 saved_relative_position = DecodePackedBcdMsf(FW_NEAR_UNKNOWN_1275);
    s32 saved_absolute_position = DecodePackedBcdMsf(FW_NEAR_UNKNOWN_1279);
    s32 mapped_start;
    u8 descriptor_index;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u)
        mapped_start = (s32)MAP_FIXED_PACKET(start_address);
    else
        mapped_start = EXPAND_RECORD_OVERHEAD(start_address);
    EncodePackedBcdMsf(FW_NEAR_UNKNOWN_1279, (s32)((u32)mapped_start + 150ul));

    for(descriptor_index = 0u; descriptor_index != 32u; ++descriptor_index)
    {
        COPY_TUPLE(FW_NEAR_UNKNOWN_1279, FW_NEAR_UNKNOWN_1428, 3u);
        WRITE_CONTROLLER_BLOCK(FW_NEAR_UNKNOWN_1428);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, FirmwareReadNear8(FW_NEAR_UNKNOWN_142B));
        SUBMIT_DESCRIPTOR(controller_buffer_cursor, 0xaau);
        POPULATE_SUBHEADER(controller_buffer_cursor, 4u);
        UPDATE_Q(0u, 0u);
        ENQUEUE_DESCRIPTOR();
        ADVANCE_MSF(0u);
        controller_buffer_cursor = ADVANCE_RING(controller_buffer_cursor);
    }

    EncodePackedBcdMsf(FW_NEAR_UNKNOWN_1275, saved_relative_position);
    EncodePackedBcdMsf(FW_NEAR_UNKNOWN_1279, saved_absolute_position);
    COPY_TUPLE(FW_NEAR_UNKNOWN_1279, FW_NEAR_UNKNOWN_1428, 3u);
}

/* RAM:fab406 */
void EnqueueThirtyTwoWritePreservationDescriptors(void)
{
    u32 controller_buffer_cursor;
    s32 frame_position;
    u8 descriptor_index;

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
        YIELD_COROUTINE0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    if(ReadSyntheticWriteDescriptorWord(FW_HUGE_UNKNOWN_2BA5D4) == 0u)
        return;

    controller_buffer_cursor = RESUME_RING() + 0x2df0ul;
    frame_position = (s32)ReadSyntheticWriteDescriptorWord(FW_NEAR_UNKNOWN_0B02);
    frame_position = (s32)MultiplyU32Low((u32)DivideSyntheticWritePosition(frame_position, 39l), 39ul) + 150l;
    EncodePackedBcdMsf(FW_NEAR_UNKNOWN_1279, frame_position);

    for(descriptor_index = 0u; descriptor_index != 32u; ++descriptor_index)
    {
        COPY_TUPLE(FW_NEAR_UNKNOWN_1279, FW_NEAR_UNKNOWN_1428, 3u);
        WRITE_CONTROLLER_BLOCK(FW_NEAR_UNKNOWN_1428);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 2u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, 2u);
        SUBMIT_DESCRIPTOR(controller_buffer_cursor, 0xaau);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0x20u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C71));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C72));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C73));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C74));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
        ENQUEUE_DESCRIPTOR();
        ADVANCE_MSF(0u);
        controller_buffer_cursor = ADVANCE_RING(controller_buffer_cursor);
    }
}
