#include "read_track_information_size_blocks.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_track_information_size_blocks_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadTrackSizeDword(u16 address)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_track_information_size_blocks_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u32 ComputeFixedPacketDataBlocks(u32 frame_span_plus_five)
{
    u32 packet_size = ReadTrackSizeDword(FW_NEAR_UNKNOWN_DD57);
    u32 packet_count = READ_TRACK_SIZE_DIVIDE(frame_span_plus_five, packet_size + 7ul);

    return READ_TRACK_SIZE_MULTIPLY(packet_count, packet_size);
}

static u32 EndMinusStartMinusTwo(u16 end_address)
{
    return ReadTrackSizeDword(end_address) - ReadTrackSizeDword(FW_NEAR_UNKNOWN_DD3D) - 2ul;
}

static u32 SpanPlusFive(u16 end_address)
{
    return ReadTrackSizeDword(end_address) - ReadTrackSizeDword(FW_NEAR_UNKNOWN_DD3D) + 5ul;
}

/* RAM:faf43d */
u32 PLEXTOR_NEAR_FUNCTION ComputeReadTrackInformationTrackSizeBlocks(void)
{
    u32 block_count = 0ul;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u)
    {
        u32 span_plus_five = SpanPlusFive(0xdd41u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
            block_count = span_plus_five - 7ul;
        else
            block_count = ComputeFixedPacketDataBlocks(span_plus_five);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC) != FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35))
        {
            block_count = EndMinusStartMinusTwo(0xdd41u);
        }
        else
        {
            u32 span_plus_five = SpanPlusFive(0x1153u);

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) != 0u)
                block_count = ComputeFixedPacketDataBlocks(span_plus_five);
            else
                block_count = span_plus_five - 7ul;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
    {
        u32 span_plus_five = SpanPlusFive(0x1153u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
            block_count = span_plus_five - 7ul;
        else
            block_count = ComputeFixedPacketDataBlocks(span_plus_five);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u)
    {
        block_count = EndMinusStartMinusTwo(0xdd41u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 6u)
    {
        u32 span_plus_five = SpanPlusFive(0xdd51u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
            block_count = span_plus_five - 5ul;
        else
            block_count = ComputeFixedPacketDataBlocks(span_plus_five);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 7u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD5F) != 0u)
            block_count = EndMinusStartMinusTwo(0xdd41u);
    }

    return (s32)block_count > 0 ? block_count : 0ul;
}
