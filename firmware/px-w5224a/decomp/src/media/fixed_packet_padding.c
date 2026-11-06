#include "fixed_packet_padding.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/fixed_packet_padding_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPaddingDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePaddingDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:feb282 */
u8 AcquireFixedPacketPaddingRawSectors(void)
{
    u32 padding_count;
    u32 padding_index;
    u32 controller_buffer_cursor;
    u32 target_position;

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
        PADDING_YIELD();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    controller_buffer_cursor = PADDING_RESUME_RING();
    padding_count = ReadPaddingDword(FW_NEAR_UNKNOWN_0C4B) - ReadPaddingDword(FW_NEAR_UNKNOWN_14B8);

    for(padding_index = 0ul; padding_index < padding_count; ++padding_index)
    {
        PADDING_COPY_TUPLE(FW_NEAR_UNKNOWN_1428, 0x1279u);
        PADDING_WRITE_CONTROLLER_BLOCK(FW_NEAR_UNKNOWN_1428);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, FirmwareReadNear8(FW_NEAR_UNKNOWN_142B));
        if(PADDING_CAPTURE(controller_buffer_cursor, 4u) == 0u)
        {
            PADDING_SET_SENSE(4u, 0x4400u);
            PADDING_SET_INFORMATION(ReadPaddingDword(FW_NEAR_UNKNOWN_1598));
            return 0u;
        }

        PADDING_POPULATE_SUBHEADER(controller_buffer_cursor, 4u);
        PADDING_UPDATE_Q(0u, 0u);
        PADDING_ENQUEUE();
        PADDING_ADVANCE_MSF(0u);
        target_position = ReadPaddingDword(FW_NEAR_UNKNOWN_1598) + 1ul;
        WritePaddingDword(FW_NEAR_UNKNOWN_1598, target_position);
        controller_buffer_cursor = PADDING_ADVANCE_RING(controller_buffer_cursor);
    }
    return 1u;
}
