#include "controller_record_type3_refresh.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_type3_refresh_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadType3RefreshU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteType3RefreshU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u16 Type3BoundaryIndex(fw_addr_t table, u16 segment)
{
    u32 position = ReadType3RefreshU32(table + (fw_addr_t)((u16)(segment * 4u)));

    return (u16)((position - 0x91ul) / 0x27ul);
}

static u8 RefreshType3HalfOpenRange(u16 first_index, u16 end_index)
{
    fw_addr_t state_table = ReadType3RefreshU32(FW_NEAR_UNKNOWN_FAFF);
    u8 changed = 0u;
    u16 index = first_index;

    while((s16)index < (s16)end_index)
    {
        fw_addr_t state = FirmwareAddressIncrementLow16(state_table, index);

        if(FirmwareRead8(state) == 3u)
        {
            TYPE3_UPDATE_BLOCK((s16)index);
            changed = 1u;
        }
        ++index;
    }
    return changed;
}

static u8 RefreshType3InclusiveRange(u16 first_index, u16 last_index)
{
    fw_addr_t state_table = ReadType3RefreshU32(FW_NEAR_UNKNOWN_FAFF);
    u8 changed = 0u;
    u16 index = first_index;

    while((s16)index <= (s16)last_index)
    {
        fw_addr_t state = FirmwareAddressIncrementLow16(state_table, index);

        if(FirmwareRead8(state) == 3u)
        {
            TYPE3_UPDATE_BLOCK((s16)index);
            changed = 1u;
        }
        ++index;
    }
    return changed;
}

static void PublishType3RefreshSummary(void)
{
    s32 lead_out = (s32)ReadType3RefreshU32(FW_NEAR_CURRENT_FRAME_POSITION);

    if(TYPE3_SCAN_BANKS(1u, lead_out, 0ul) == 0u)
    {
        WriteType3RefreshU32(FW_NEAR_SCAN_POSITION_SECOND, (u32)lead_out);
        WriteType3RefreshU32(FW_NEAR_SCAN_POSITION_FIRST, (u32)lead_out);
    }
    else
    {
        u32 first_position = ReadType3RefreshU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT);

        WriteType3RefreshU32(FW_NEAR_SCAN_POSITION_FIRST, first_position);
        WriteType3RefreshU32(FW_NEAR_SCAN_POSITION_SECOND, first_position);
    }
}

/* RAM:fa2e53 */
void RefreshType3ControllerRecordBlocksForIndexRange(u16 first_index, u16 last_index)
{
    u16 first_segment;
    u16 last_segment;
    u16 first_segment_end;
    u16 last_segment_end;
    u8 partial_first;
    u8 partial_last;
    u8 changed = 0u;
    u16 middle_last_segment;
    u16 segment;

    if(last_index < 0x20u)
        return;
    if(first_index < 0x20u)
        first_index = 0x20u;

    first_segment = (u16)(first_index / 0x90u);
    last_segment = (u16)(last_index / 0x90u);
    first_segment_end = Type3BoundaryIndex(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84690), first_segment);
    last_segment_end = Type3BoundaryIndex(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84690), last_segment);
    partial_first = first_index < first_segment_end ? 1u : 0u;
    partial_last = last_index < last_segment_end ? 1u : 0u;

    if(partial_first != 0u && RefreshType3HalfOpenRange(first_index, first_segment_end) != 0u)
        changed = 1u;

    middle_last_segment = partial_last != 0u ? (u16)(last_segment - 1u) : last_segment;
    if((s16)middle_last_segment > 0)
    {
        segment = (u16)(first_segment + 1u);
        while((s16)segment <= (s16)middle_last_segment)
        {
            u16 range_start = Type3BoundaryIndex(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490), segment);
            u16 range_end = Type3BoundaryIndex(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84690), segment);

            if(RefreshType3HalfOpenRange(range_start, range_end) != 0u)
                changed = 1u;
            ++segment;
        }
    }

    if(partial_last != 0u)
    {
        u16 range_start = Type3BoundaryIndex(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490), last_segment);

        if(RefreshType3InclusiveRange(range_start, last_index) != 0u)
            changed = 1u;
    }

    if(changed != 0u)
        PublishType3RefreshSummary();
}
