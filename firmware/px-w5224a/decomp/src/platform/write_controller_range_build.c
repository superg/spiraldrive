#include "write_controller_range_build.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

static u16 ReadRangeU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadRangeU24(fw_addr_t address)
{
    u32 value = (u32)FirmwareRead8(address) << 16;

    value |= (u32)FirmwareRead8(address + 1u) << 8;
    value |= FirmwareRead8(address + 2u);
    return value;
}

static u32 ReadRangeU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRangeU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteRangeU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void SwapRangeU32(fw_addr_t first, fw_addr_t second)
{
    u32 temporary = ReadRangeU32(first);

    WriteRangeU32(first, ReadRangeU32(second));
    WriteRangeU32(second, temporary);
}

/* RAM:fc69ac */
u16 BuildSortedWriteControllerBufferRanges(s32 start_frame_position, u32 block_count)
{
    u16 accepted_count = 0u;
    u8 output_count = 0u;
    u8 segment_index;
    s32 request_start;
    s32 request_end;
    u32 previous_second_code = 0u;

    for(segment_index = 0u; segment_index < 0x20u; ++segment_index)
    {
        fw_addr_t output = FW_HUGE_UNKNOWN_2B9600 + (fw_addr_t)segment_index * 8u;

        WriteRangeU32(output, 0ul);
        WriteRangeU32(output + 4u, 0ul);
    }
    WriteRangeU16(FW_NEAR_UNKNOWN_FC40, 0u);

    request_start = CompressThirtyNineFramePositionToLogicalBlockAddress(start_frame_position);
    request_end = (s32)((u32)request_start + block_count - 1ul);
    WriteRangeU32(FW_HUGE_UNKNOWN_2B9714, (u32)request_start);
    WriteRangeU32(FW_HUGE_UNKNOWN_2B9718, (u32)request_end);

    for(segment_index = 0u; segment_index < 0x40u; ++segment_index)
    {
        fw_addr_t segment = FW_HUGE_UNKNOWN_194000 + (fw_addr_t)segment_index * 0x800ul;
        u32 segment_first = ReadRangeU24(segment + 0x11u);
        u32 segment_second = ReadRangeU24(segment + 0x15u);
        u16 record_count;
        fw_addr_t record;
        u16 record_index;

        if(segment_first == FW_24BIT_ADDRESS_MASK || request_end < (s32)(segment_first & 0xffffful) || (s32)(segment_second & 0xffffful) < request_start)
            continue;

        record_count = ReadRangeU16(segment + 8u);
        record = segment + 0x20u;
        for(record_index = 0u; record_index < record_count; ++record_index)
        {
            u32 first_code = ReadRangeU24(record);

            record += 3u;
            if((first_code >> 20) < 2u && (first_code | previous_second_code) != 0u)
            {
                u32 second_code = ReadRangeU24(record);
                s32 masked_first = (s32)(first_code & 0xffffful);

                previous_second_code = second_code;
                record += 3u;
                if(request_start <= masked_first && masked_first <= request_end)
                {
                    fw_addr_t output = FW_HUGE_UNKNOWN_2B9600 + (fw_addr_t)output_count * 8u;

                    ++accepted_count;
                    WriteRangeU16(FW_NEAR_UNKNOWN_FC40, accepted_count);
                    WriteRangeU32(output, (u32)ExpandLogicalBlockAddressToThirtyNineFramePosition((s32)(second_code & 0xffffful)));
                    WriteRangeU32(output + 4u, (u32)ExpandLogicalBlockAddressToThirtyNineFramePosition(masked_first));
                    ++output_count;
                }
            }
            else
            {
                record += 3u;
            }
        }
    }

    {
        u8 gap = (u8)(output_count >> 1);

        while(gap != 0u)
        {
            u8 index;

            for(index = gap; index < output_count; ++index)
            {
                u16 lower = (u16)index - gap;

                for(;;)
                {
                    fw_addr_t lower_record = FW_HUGE_UNKNOWN_2B9600 + (fw_addr_t)lower * 8u;
                    fw_addr_t upper_record = FW_HUGE_UNKNOWN_2B9600 + (fw_addr_t)(lower + gap) * 8u;

                    if(ReadRangeU32(lower_record) <= ReadRangeU32(upper_record))
                        break;
                    SwapRangeU32(lower_record, upper_record);
                    SwapRangeU32(lower_record + 4u, upper_record + 4u);
                    if(lower < gap)
                        break;
                    lower = (u16)(lower - gap);
                }
            }
            gap >>= 1;
        }
    }

    accepted_count = ReadRangeU16(FW_NEAR_UNKNOWN_FC40);
    WriteRangeU16(FW_NEAR_UNKNOWN_FC40, 0u);
    return accepted_count;
}
