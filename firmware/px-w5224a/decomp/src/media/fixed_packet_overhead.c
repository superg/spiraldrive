#include "fixed_packet_overhead.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/fixed_packet_overhead_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadFixedPacketU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

/* RAM:f93f07 */
u8 GetFixedPacketOverheadFramesRemaining(s32 frame_position)
{
    u8 track_number;
    u8 last_track_number;
    u32 track_offset;
    u32 track_end;
    u32 packet_size;
    s32 track_start;
    s32 relative_position;
    u32 packet_span;
    u32 remainder;

    if(FIXED_PACKET_STATE_VALID() == 0u)
        return 0xffu;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u)
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u)
        track_number = 1u;
    else
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);

    last_track_number = FIXED_PACKET_LAST_TRACK();
    while(track_number <= last_track_number)
    {
        track_offset = (u16)track_number * 0x13u;
        track_end = (u32)FIXED_PACKET_CONVERT_MSF(FW_NEAR_UNKNOWN_DD94 + track_offset);
        if(track_end == 0ul)
        {
            track_end = ReadFixedPacketU32(FW_NEAR_CURRENT_FRAME_POSITION);
            if(frame_position < (s32)track_end)
                break;
        }
        if((s32)((u32)frame_position + 150ul) < (s32)track_end)
            break;
        ++track_number;
    }

    track_offset = (u16)track_number * 0x13u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD89 + track_offset) & 1u) == 0u)
        return 0xffu;

    packet_size = ReadFixedPacketU32(FW_NEAR_UNKNOWN_DD8A + track_offset);
    if(packet_size == 0ul)
        return 0xffu;

    track_start = FIXED_PACKET_CONVERT_MSF(FW_NEAR_UNKNOWN_DD81 + track_offset);
    relative_position = (s32)((u32)frame_position - (u32)track_start);
    if(relative_position < 0l)
        return 0u;

    packet_span = packet_size + 7ul;
    remainder = FIXED_PACKET_REMAINDER((u32)relative_position, packet_span);
    if(remainder < packet_size || remainder > packet_size + 7ul)
        return 0u;
    return (u8)(7ul - (remainder - packet_size));
}
