#include "raw_read_track_record_advance.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/raw_read_q_control.h"
#include "../platform/status_mapping.h"
#include "raw_read_transfer_layout.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_track_record_advance_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadTrackAdvanceU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadTrackAdvanceU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteTrackAdvanceU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteTrackAdvanceU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static fw_addr_t TrackRecordAddress(u16 index)
{
    return FW_NEAR_UNKNOWN_B4BD + (u16)(index << 3);
}

static void CopyCurrentQPacketToController(void)
{
    u8 offset;

    for(offset = 0u; offset < 10u; ++offset)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88AA + offset, FirmwareReadNear8(FW_NEAR_UNKNOWN_1272 + offset));
}

/* RAM:fe25d4 */
void PLEXTOR_NEAR_FUNCTION AdvanceRawReadTrackRecordAndUpdateCurrentQPacket(void)
{
    u16 index;
    u16 boundary_index;
    fw_addr_t record_address;
    s32 distance = 0l;
    u8 track;
    u8 point;
    u8 q_type;

    TRACK_ADVANCE_CONFIGURE();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
        {
            index = ReadTrackAdvanceU16(FW_NEAR_UNKNOWN_149B);
            WriteTrackAdvanceU16(FW_NEAR_UNKNOWN_149D, index);
            for(;;)
            {
                record_address = TrackRecordAddress(index);
                track = FirmwareRead8(record_address + 1u);
                if((FirmwareRead8(record_address) & 0x0fu) == 1u && track == FirmwareReadNear8(FW_NEAR_UNKNOWN_14A5))
                    break;
                index = (u16)(index + 1u);
                WriteTrackAdvanceU16(FW_NEAR_UNKNOWN_149B, index);
            }
        }
        return;
    }

    index = ReadTrackAdvanceU16(FW_NEAR_UNKNOWN_149B);
    while(distance == 0l)
    {
        distance = TRACK_ADVANCE_DISTANCE(index);
        if(distance == 0l)
        {
            do
            {
                index = (u16)(index + 1u);
                WriteTrackAdvanceU16(FW_NEAR_UNKNOWN_149B, index);
                record_address = TrackRecordAddress(index);
            } while((FirmwareRead8(record_address) & 0x0fu) != 1u);
        }
    }

    record_address = TrackRecordAddress(index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, FirmwareRead8(record_address));
    track = FirmwareRead8(record_address + 1u);
    point = FirmwareRead8(record_address + 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, TRACK_ADVANCE_BCD(track));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, TRACK_ADVANCE_BCD(point));

    if(TRACK_ADVANCE_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_14A8)) != FirmwareReadNear8(FW_NEAR_UNKNOWN_1273))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AB, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A9, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AA, 1u);
        CopyCurrentQPacketToController();
    }
    if((FirmwareRead8(record_address) & 0x0fu) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AA, FirmwareReadNear8(FW_NEAR_UNKNOWN_1274));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1274) > 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A9, 1u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1405) == 1u)
    {
        TRACK_ADVANCE_RESTORE_ISRC((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1274) == 0u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1405, 0u);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1403) == 1u)
    {
        TRACK_ADVANCE_RESTORE_MCN((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1274) == 0u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1403, 0u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1274) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 1u);
        TRACK_ADVANCE_SET_MSF((s32)((u32)distance - 1ul), (s32)ReadTrackAdvanceU32(FW_NEAR_UNKNOWN_1598));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1274) == 1u)
        {
            TRACK_ADVANCE_SET_MSF(0l, (s32)ReadTrackAdvanceU32(FW_NEAR_UNKNOWN_1598));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14D0, 1u);
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A8, track);
    q_type = TRACK_ADVANCE_DECODE(index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, q_type);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14A9) == 0u)
        TRACK_ADVANCE_SELECT_ISRC();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A7, FirmwareReadNear8(FW_NEAR_UNKNOWN_127C));

    do
    {
        index = (u16)(index + 1u);
        WriteTrackAdvanceU16(FW_NEAR_UNKNOWN_149B, index);
        boundary_index = index;
        record_address = TrackRecordAddress(index);
    } while((FirmwareRead8(record_address) & 0x0fu) != 1u);
    for(;;)
    {
        record_address = TrackRecordAddress(boundary_index);
        if((FirmwareRead8(record_address) & 0x0fu) == 1u && FirmwareRead8(record_address + 2u) == 1u)
            break;
        boundary_index = (u16)(boundary_index + 1u);
    }
    if(q_type == 0u || q_type > 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CF, 1u);
        WriteTrackAdvanceU32(FW_NEAR_UNKNOWN_14D1, (u32)TRACK_ADVANCE_BOUNDARY(boundary_index) - 150ul);
    }
}
