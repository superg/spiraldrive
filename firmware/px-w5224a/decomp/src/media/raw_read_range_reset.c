#include "raw_read_range_reset.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_range_reset_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadResetRangeWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadResetRangeDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteResetRangeWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteResetRangeDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fe825b */
u8 PLEXTOR_NEAR_FUNCTION ResetAndAcquireRawReadRange(s32 start_frame_position, u32 frame_count, u8 q_metadata_mode)
{
    u8 preserve_main_buffer = CAPTURE_INHERITED_R15();
    u32 ring_slot_count = ReadResetRangeDword(FW_NEAR_UNKNOWN_156A);

    WriteResetRangeWord(FW_NEAR_UNKNOWN_1298, 0u);
    WriteResetRangeWord(FW_NEAR_UNKNOWN_1294, 0u);
    WriteResetRangeWord(FW_NEAR_UNKNOWN_1296, 0u);
    WriteResetRangeWord(FW_NEAR_UNKNOWN_1292, 0u);
    WriteResetRangeWord(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteResetRangeWord(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteResetRangeDword(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteResetRangeDword(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteResetRangeDword(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteResetRangeDword(FW_NEAR_UNKNOWN_126A, ReadResetRangeDword(FW_NEAR_UNKNOWN_1562));
    WriteResetRangeDword(FW_NEAR_UNKNOWN_126E, 0ul);
    WriteResetRangeDword(FW_NEAR_UNKNOWN_1598, (u32)start_frame_position);

    if(q_metadata_mode == 0u)
        preserve_main_buffer = 1u;
    if(ACQUIRE_RESET_RANGE(frame_count, q_metadata_mode, preserve_main_buffer) == 0u)
        return 0u;

    if(POLL_RESET_RANGE(1u) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x40u) != 0u)
            REPORT_RESET_RANGE_FAILURE(8u);
        else if(ReadResetRangeWord(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
            REPORT_RESET_RANGE_SENSE_030C09();
        else
            REPORT_RESET_RANGE_FAILURE(2u);
        return 0u;
    }

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 1u)
        YIELD_RESET_RANGE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u)
    {
        REPORT_RESET_RANGE_FAILURE(7u);
        return 0u;
    }
    return 1u;
}
