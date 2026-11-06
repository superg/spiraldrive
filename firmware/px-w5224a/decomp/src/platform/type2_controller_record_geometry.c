#include "type2_controller_record_geometry.h"
#include "firmware_memory.h"
#include "mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/type2_controller_record_geometry_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadGeometryU32LowWordFirst(fw_addr_t address)
{
    u16 low = (u16)((u16)FirmwareRead8(address + 2u) << 8);
    u16 high;

    low |= FirmwareRead8(address + 3u);
    high = (u16)((u16)FirmwareRead8(address) << 8);
    high |= FirmwareRead8(address + 1u);
    return ((u32)high << 16) | low;
}

static void WriteGeometryU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteGeometryU32LowWordFirst(fw_addr_t address, u32 value)
{
    FirmwareWriteU16(FirmwareAddressIncrement24(address, 2u), (u16)value);
    FirmwareWriteU16(address, (u16)(value >> 16));
}

static void InitializeGeometryTable(fw_addr_t table)
{
    u8 index;

    for(index = 0u; index < 8u; ++index)
        WriteGeometryU32LowWordFirst(table + (u16)index * 4u, 0x00fffffful);
}

static void BuildGeometrySlotTable(fw_addr_t table, fw_addr_t slot_state, fw_addr_t minimum_address, u8 group_count, s32 first_default)
{
    s32 minimum = first_default;
    s32 default_value = first_default;
    u8 index;

    WriteGeometryU32LowWordFirst(minimum_address, (u32)minimum);
    for(index = 0u; index < group_count; ++index)
    {
        fw_addr_t table_entry = table + (u16)index * 4u;
        fw_addr_t slot = slot_state + (u16)index * 5u;
        s32 value;

        WriteGeometryU32LowWordFirst(table_entry, (u32)default_value);
        default_value = (s32)((u32)default_value - 32ul);
        if(FirmwareRead8(slot) == 0u)
        {
            value = (s32)ReadGeometryU32LowWordFirst(table_entry);
            WriteGeometryU32LowWordFirst(slot + 1u, (u32)value);
        }
        else
        {
            value = (s32)ReadGeometryU32LowWordFirst(slot + 1u);
            WriteGeometryU32LowWordFirst(table_entry, (u32)value);
        }
        if(value <= minimum)
        {
            minimum = value;
            WriteGeometryU32LowWordFirst(minimum_address, (u32)minimum);
        }
    }
}

/* RAM:fa8bdc */
void InitializeType2ControllerRecordGeometry(void)
{
    u32 lead_out;
    u32 remainder;
    u32 high_frame;
    u32 middle_frame;
    u32 low_frame;
    u32 numerator;
    s32 secondary_seed;
    u8 segment_quotient;
    u8 tuple_high;
    u8 tuple_low;
    u8 active_segments;
    u8 group_count;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        return;

    WriteGeometryU32LowWordFirst(FW_NEAR_UNKNOWN_FC44, 0x00800000ul);

    lead_out = ReadGeometryU32LowWordFirst(FW_NEAR_CURRENT_FRAME_POSITION);
    segment_quotient = (u8)DIVIDE_GEOMETRY_S32((s32)(lead_out - 0x0a78ul), 0x15f0l);
    lead_out = ReadGeometryU32LowWordFirst(FW_NEAR_CURRENT_FRAME_POSITION);
    remainder = REMAINDER_GEOMETRY_U32(lead_out - 0x0a78ul, 0x15f0ul);

    if((s32)ReadGeometryU32LowWordFirst(FW_NEAR_CURRENT_FRAME_POSITION) > 360000l)
    {
        segment_quotient = 0x3fu;
        remainder = 0x0db8ul;
    }

    if(remainder < 0x0138ul)
    {
        active_segments = segment_quotient;
        tuple_high = segment_quotient;
        tuple_low = 0x88u;
        high_frame = (u32)segment_quotient * 0x15f0ul + 0x0a78ul;
    }
    else
    {
        active_segments = (u8)(segment_quotient + 1u);
        if(remainder - 0x0138ul < 0x27ul)
        {
            tuple_high = segment_quotient;
            tuple_low = 0x88u;
            high_frame = (u32)segment_quotient * 0x15f0ul + 0x0bb0ul;
        }
        else
        {
            tuple_low = (u8)DIVIDE_GEOMETRY_U32(remainder - 0x0138ul, 0x27ul);
            tuple_high = active_segments;
            high_frame = MULTIPLY_GEOMETRY_U32(segment_quotient, 0x15f0ul);
            high_frame += 0x0bb0ul;
            high_frame += MULTIPLY_GEOMETRY_U32(tuple_low, 0x27ul);
        }
    }

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA588, active_segments);
    WriteGeometryU32LowWordFirst(FW_HUGE_UNKNOWN_2BA58D, high_frame);
    numerator = MULTIPLY_GEOMETRY_U32((u32)tuple_high - 1ul, 0x88ul);
    numerator += tuple_low;
    WriteGeometryU32LowWordFirst(FW_HUGE_UNKNOWN_2BA529, (numerator << 5) - 1ul);

    group_count = (u8)(active_segments >> 3);
    if((active_segments & 7u) != 0u)
        ++group_count;
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA536, group_count);

    WriteGeometryU32LowWordFirst(FW_HUGE_UNKNOWN_2BA591, (u32)-39l);
    middle_frame = high_frame - 39ul;
    WriteGeometryU32LowWordFirst(FW_HUGE_UNKNOWN_2BA595, middle_frame);
    numerator = (middle_frame - 0x91ul) << 5;
    secondary_seed = DIVIDE_GEOMETRY_S32((s32)numerator, 39l);
    low_frame = middle_frame - 0x0138ul;
    WriteGeometryU32LowWordFirst(FW_HUGE_UNKNOWN_2BA599, low_frame);
    WriteGeometryU32LowWordFirst(FW_HUGE_UNKNOWN_2BA589, low_frame);

    InitializeGeometryTable(FW_HUGE_UNKNOWN_2BA537);
    InitializeGeometryTable(FW_HUGE_UNKNOWN_2BA557);
    BuildGeometrySlotTable(FW_HUGE_UNKNOWN_2BA537, FW_NEAR_UNKNOWN_FB03, FW_HUGE_UNKNOWN_2BA577, group_count, -214l);
    BuildGeometrySlotTable(FW_HUGE_UNKNOWN_2BA557, FW_NEAR_UNKNOWN_FB2B, FW_HUGE_UNKNOWN_2BA57B, group_count, (s32)((u32)secondary_seed - 32ul));

    WriteGeometryU16(FW_HUGE_UNKNOWN_2BA5D9, (u16)DIVIDE_GEOMETRY_S32((s32)(low_frame - 0x91ul), 39l));
}
