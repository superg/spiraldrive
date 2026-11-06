#include "read_track_information_free_blocks.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_track_information_free_blocks_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadTrackFreeDword(u16 address)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_track_information_free_blocks_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u32 ComputeFreeFixedPacketBlocks(u32 frame_span)
{
    u32 packet_size = ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD57);
    u32 packet_count = READ_TRACK_FREE_DIVIDE(frame_span, packet_size + 7ul);

    return READ_TRACK_FREE_MULTIPLY(packet_count, packet_size);
}

static u32 ComputePacketOrLinearFreeBlocks(u32 frame_span_plus_five)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
        return frame_span_plus_five - 7ul;
    return ComputeFreeFixedPacketBlocks(frame_span_plus_five);
}

/* RAM:faf206 */
u32 PLEXTOR_NEAR_FUNCTION ComputeReadTrackInformationFreeBlockCount(void)
{
    u8 track_type;
    u8 write_type;
    u32 block_count = 0ul;

    track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    if(track_type == 5u || track_type == 6u || track_type == 7u || track_type == 4u)
        return 0ul;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) == 0u)
    {
        if(track_type == 3u)
        {
            block_count = ComputePacketOrLinearFreeBlocks(ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD41) - (ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD45) + 5ul) + 5ul);
        }
        else if(track_type == 1u)
        {
            block_count = ComputePacketOrLinearFreeBlocks(ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - (ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD45) + 5ul) + 5ul);
        }
        else if(track_type == 2u)
        {
            block_count = ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD41) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD3D) - 2ul;
        }
        else if(track_type == 0u)
        {
            write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
            if(write_type == 1u)
                block_count = ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD3D) - 2ul;
            else if(write_type == 2u)
                block_count = ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD6D) - 150ul;
            else if(write_type == 3u)
                block_count = ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD69);
            else if(write_type == 0u)
                block_count = ComputePacketOrLinearFreeBlocks(ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD3D) + 5ul);
        }
    }
    else
    {
        write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
        if(write_type == 1u)
        {
            if(track_type == 0u)
                block_count = ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD4D) - 2ul;
            if(track_type == 2u)
                block_count = ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD41) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD4D) - 2ul;
        }
        else if(write_type == 2u)
        {
            block_count = ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD6D) - 150ul;
        }
        else if(write_type == 3u)
        {
            block_count = ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD65);
        }
        else if(write_type == 0u)
        {
            if(track_type == 3u)
                block_count = ComputePacketOrLinearFreeBlocks(ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD41) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD4D) + 5ul);
            else if(track_type == 0u || track_type == 1u)
                block_count = ComputePacketOrLinearFreeBlocks(ReadTrackFreeDword(FW_NEAR_CURRENT_FRAME_POSITION) - ReadTrackFreeDword(FW_NEAR_UNKNOWN_DD4D) + 5ul);
        }
    }

    return (s32)block_count < 0 ? 0ul : block_count;
}
