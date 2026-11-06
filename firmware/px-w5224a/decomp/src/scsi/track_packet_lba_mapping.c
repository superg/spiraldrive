#include "track_packet_lba_mapping.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/track_packet_lba_mapping_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadTrackPacketU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteTrackPacketU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static fw_addr_t TrackPacketFieldAddress(u8 track_number, fw_addr_t field_base)
{
    return field_base + (u16)track_number * 0x13u;
}

/* RAM:fd38b8 */
s32 MapLogicalBlockAddressThroughTrackPacketLayout(s32 logical_block_address)
{
    s32 mapped_address = (s32)((u32)logical_block_address + 150ul);
    u8 track_number;
    u8 last_track_number;
    s32 track_end;
    u32 packet_size;

    if(FirmwareReadBit000B() != 1u)
    {
        FirmwareWriteBit000C(0u);
        return mapped_address;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u)
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u)
        track_number = 1u;
    else
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);

    last_track_number = GET_TRACK_PACKET_LAST_TRACK();
    while(track_number <= last_track_number)
    {
        track_end = CONVERT_TRACK_PACKET_MSF(TrackPacketFieldAddress(track_number, FW_NEAR_UNKNOWN_DD94));
        if(track_end == 0)
        {
            track_end = (s32)ReadTrackPacketU32(FW_NEAR_CURRENT_FRAME_POSITION);
            if(mapped_address < track_end)
                break;
        }
        if((s32)((u32)mapped_address + 150ul) < track_end)
            break;
        track_number = (u8)(track_number + 1u);
    }

    if((FirmwareRead8(TrackPacketFieldAddress(track_number, FW_NEAR_UNKNOWN_DD89)) & 1u) == 0u)
    {
        FirmwareWriteBit000C(0u);
        return mapped_address;
    }

    packet_size = ReadTrackPacketU32(TrackPacketFieldAddress(track_number, FW_NEAR_UNKNOWN_DD8A));
    if(packet_size == 0ul)
    {
        FirmwareWriteBit000C(0u);
        return mapped_address;
    }

    track_end = CONVERT_TRACK_PACKET_MSF(TrackPacketFieldAddress(track_number, FW_NEAR_UNKNOWN_DD84));
    if(track_end < mapped_address)
    {
        FirmwareWriteBit000C(0u);
        return mapped_address;
    }

    {
        s32 track_start = CONVERT_TRACK_PACKET_MSF(TrackPacketFieldAddress(track_number, FW_NEAR_UNKNOWN_DD81));
        s32 delta = (s32)((u32)mapped_address - (u32)track_start);

        if(delta < 0)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u && READ_TRACK_PACKET_EXCEPTION_BIT() != 1u)
                FirmwareWriteBit000C(0u);
        }
        else
        {
            u32 packet_count = DIVIDE_TRACK_PACKET((u32)delta, packet_size);
            u32 adjustment = MULTIPLY_TRACK_PACKET(7ul, packet_count);

            WriteTrackPacketU32(FW_NEAR_UNKNOWN_0F0C, adjustment);
            mapped_address = (s32)((u32)mapped_address + adjustment);
            FirmwareWriteBit000C(1u);
        }
    }
    return mapped_address;
}
