#include "raw_read_ring_alignment.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_ring_alignment_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBigEndianU32(fw_addr_t address);
static void WriteBigEndianU32(fw_addr_t address, u32 value);

static u8 DecodePackedPositionByte(u8 value)
{
    return (u8)((value >> 4) * 10u + (value & 0x0fu));
}

/* RAM:fee8f0 */
u8 DecodeControllerPositionFields(u8 check_controller_status)
{
    u8 minute;
    u8 second;
    u8 frame;
    u8 auxiliary;

    if(check_controller_status != 0u && (FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01800F) & 0xf0u) != 0u)
        return 0u;

    minute = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018004);
    second = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018005);
    frame = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018006);
    auxiliary = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018007);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A3, auxiliary);

    if(minute <= 0xf9u && second <= 0x59u && frame <= 0x74u)
    {
        s32 frame_position;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A0, DecodePackedPositionByte(minute));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A1, DecodePackedPositionByte(second));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A2, DecodePackedPositionByte(frame));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_102E) == 1u)
        {
            frame_position = CONVERT_CONTROLLER_POSITION(FW_NEAR_UNKNOWN_15A0);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 1u);
            frame_position = CONVERT_CONTROLLER_POSITION(FW_NEAR_UNKNOWN_15A0);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);
        }
        WriteBigEndianU32(FW_NEAR_UNKNOWN_0F6E, (u32)frame_position);
        return 1u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A0, minute);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A1, second);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A2, frame);
    if(minute >= 0xaau && minute <= 0xadu)
    {
        u32 frame_position = ReadBigEndianU32(FW_NEAR_UNKNOWN_1598);

        if(check_controller_status != 0u)
        {
            if(READ_CONTROLLER_POSITION_BIT000F() != 0u)
                frame_position--;
            else
                frame_position = ReadBigEndianU32(FW_NEAR_UNKNOWN_1235);
        }
        WriteBigEndianU32(FW_NEAR_UNKNOWN_0F6E, frame_position);
        return 1u;
    }

    if(READ_CONTROLLER_POSITION_BIT0013() != 0u || READ_CONTROLLER_POSITION_BIT001C() != 0u || READ_CONTROLLER_POSITION_BIT001D() != 0u)
        return 1u;
    return 0u;
}

static u32 ReadBigEndianU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteBigEndianU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:f93ace */
void PLEXTOR_NEAR_FUNCTION AdvanceMainDataRingAndCheckControllerBufferAlignment(u32 expected_controller_buffer_address)
{
    u32 cursor;
    u32 ring_size;

    DECODE_CONTROLLER_POSITION(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1F) - 1u));

    cursor = ReadBigEndianU32(FW_NEAR_UNKNOWN_126E) + 0x930ul;
    WriteBigEndianU32(FW_NEAR_UNKNOWN_126E, cursor);
    ring_size = ReadBigEndianU32(FW_NEAR_UNKNOWN_155E);
    cursor = ReadBigEndianU32(FW_NEAR_UNKNOWN_126E);
    if(cursor >= ring_size)
    {
        ring_size = ReadBigEndianU32(FW_NEAR_UNKNOWN_155E);
        cursor = ReadBigEndianU32(FW_NEAR_UNKNOWN_126E);
        WriteBigEndianU32(FW_NEAR_UNKNOWN_126E, cursor - ring_size);
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1F) < 3)
    {
        if((expected_controller_buffer_address % 0x930ul) != 0ul || ReadBigEndianU32(FW_NEAR_UNKNOWN_126E) != expected_controller_buffer_address)
            INITIALIZE_CONTROLLER_CURSORS();
    }
}
