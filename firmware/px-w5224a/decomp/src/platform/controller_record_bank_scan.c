#include "controller_record_bank_scan.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

extern u32 DivideU32RemainderNative(u32 dividend, u32 divisor);

static u16 ReadRecordScanU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadRecordScanU24(fw_addr_t address)
{
    u32 value = (u32)FirmwareRead8(address) << 16;

    value |= (u32)FirmwareRead8(address + 1u) << 8;
    value |= FirmwareRead8(address + 2u);
    return value;
}

static u32 ReadRecordScanU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRecordScanU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteRecordScanU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 IsSpecialControllerRecordModeEnabled(void)
{
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u)
        return 1u;
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) != 0u;
}

static void InitializeRecordScanSummary(u32 first_position, u32 second_position)
{
    WriteRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT, first_position);
    WriteRecordScanU32(FW_HUGE_UNKNOWN_2B970C, first_position);
    WriteRecordScanU32(FW_HUGE_UNKNOWN_2B9700, second_position);
    WriteRecordScanU32(FW_HUGE_UNKNOWN_2B9708, second_position);
}

static void UpdateRecordScanSummary(u32 first_position, u32 second_position, u8 reset_continuity_on_lower)
{
    if(first_position < ReadRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT))
    {
        WriteRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT, first_position);
        WriteRecordScanU32(FW_HUGE_UNKNOWN_2B9700, second_position);
        if(reset_continuity_on_lower != 0u)
            WriteRecordScanU16(FW_NEAR_UNKNOWN_FC42, 1u);
    }
    if(ReadRecordScanU32(FW_HUGE_UNKNOWN_2B970C) < first_position)
    {
        WriteRecordScanU32(FW_HUGE_UNKNOWN_2B970C, first_position);
        WriteRecordScanU32(FW_HUGE_UNKNOWN_2B9708, second_position);
    }
}

static u8 IsRecordPairContinuous(u32 previous_first, u32 previous_second, u32 current_first, u32 current_second)
{
    if(current_first == previous_first + 1ul && current_second == previous_second + 1ul)
        return 1u;

    if(current_first != previous_first + 8ul || current_second != previous_second + 8ul)
        return 0u;
    if(DivideU32RemainderNative(previous_first - 150ul, 39ul) != 0ul)
        return 0u;
    return DivideU32RemainderNative(previous_second - 150ul, 39ul) == 0ul;
}

/* RAM:fc6e53 */
u16 ScanControllerRecordBanksForFrameRange(u8 force_bank_scan, s32 start_frame_position, u32 frame_count)
{
    u16 matching_count = 0u;
    u16 continuity_count = 0u;
    u8 normal_record_count = 0u;
    u8 discontinuity_seen = 0u;
    u32 previous_first = 0ul;
    u32 previous_second = 0ul;
    s32 request_start;
    s32 request_end;
    u8 bank_index;

    WriteRecordScanU16(FW_NEAR_UNKNOWN_FC40, 0u);
    WriteRecordScanU16(FW_NEAR_UNKNOWN_FC42, 0u);
    WriteRecordScanU32(FW_HUGE_UNKNOWN_2B9700, 0ul);
    WriteRecordScanU32(FW_HUGE_UNKNOWN_2B970C, 0ul);
    WriteRecordScanU32(FW_HUGE_UNKNOWN_2B9708, 0ul);
    WriteRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT, 0ul);

    request_start = CompressThirtyNineFramePositionToLogicalBlockAddress(start_frame_position);
    request_end = (s32)((u32)request_start + frame_count - 1ul);
    WriteRecordScanU32(FW_HUGE_UNKNOWN_2B9714, (u32)request_start);
    WriteRecordScanU32(FW_HUGE_UNKNOWN_2B9718, (u32)request_end);

    for(bank_index = 0u; bank_index < 0x40u; ++bank_index)
    {
        fw_addr_t bank = FW_HUGE_UNKNOWN_194000 + (fw_addr_t)bank_index * 0x800ul;
        u16 record_count = ReadRecordScanU16(bank + 8u);
        u16 alternate_offset = ReadRecordScanU16(bank + 0x0cu);
        u16 record_offset = 0x20u;
        u32 header_first = ReadRecordScanU24(bank + 0x11u);
        u32 header_second = ReadRecordScanU24(bank + 0x15u);
        u8 scan_bank;

        if(header_first == FW_24BIT_ADDRESS_MASK)
        {
            scan_bank = (u8)(IsSpecialControllerRecordModeEnabled() != 0u && alternate_offset != 0u);
        }
        else
        {
            s32 expanded_second = ExpandLogicalBlockAddressToThirtyNineFramePosition((s32)(header_second & 0xffffful));
            u8 header_rejected = expanded_second < start_frame_position;

            if(header_rejected == 0u && normal_record_count != 0u)
            {
                s32 expanded_first = ExpandLogicalBlockAddressToThirtyNineFramePosition((s32)(header_first & 0xffffful));

                if((u32)expanded_first > ReadRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT))
                    header_rejected = 1u;
            }

            if(header_rejected == 0u)
            {
                scan_bank = 1u;
            }
            else if(IsSpecialControllerRecordModeEnabled() != 0u && alternate_offset != 0u)
            {
                record_offset = alternate_offset;
                record_count = (u16)(record_count - (u16)((u16)(alternate_offset - 0x20u) / 6u));
                scan_bank = 1u;
            }
            else
            {
                scan_bank = 0u;
            }
        }

        if(scan_bank != 0u || force_bank_scan != 0u)
        {
            fw_addr_t record = bank + record_offset;
            u16 record_index;

            for(record_index = 0u; record_index < record_count; ++record_index, record += 6u)
            {
                u32 first_code = ReadRecordScanU24(record);
                u32 second_code = ReadRecordScanU24(record + 3u);

                if((first_code >> 20) == 0u && (first_code | second_code) != 0u)
                {
                    s32 masked_first = (s32)(first_code & 0xffffful);

                    if(request_start <= masked_first && masked_first <= request_end)
                    {
                        u32 current_second = (u32)ExpandLogicalBlockAddressToThirtyNineFramePosition((s32)(second_code & 0xffffful));
                        u32 current_first = (u32)ExpandLogicalBlockAddressToThirtyNineFramePosition(masked_first);

                        ++matching_count;
                        WriteRecordScanU16(FW_NEAR_UNKNOWN_FC40, matching_count);
                        if(normal_record_count == 0u)
                        {
                            if(ReadRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT) == 0ul)
                                InitializeRecordScanSummary(current_first, current_second);
                            else
                                UpdateRecordScanSummary(current_first, current_second, 0u);
                            ++normal_record_count;
                            continuity_count = 1u;
                            WriteRecordScanU16(FW_NEAR_UNKNOWN_FC42, continuity_count);
                        }
                        else
                        {
                            UpdateRecordScanSummary(current_first, current_second, 1u);
                            continuity_count = ReadRecordScanU16(FW_NEAR_UNKNOWN_FC42);
                            if(IsRecordPairContinuous(previous_first, previous_second, current_first, current_second) != 0u)
                            {
                                if(discontinuity_seen == 0u)
                                {
                                    ++continuity_count;
                                    WriteRecordScanU16(FW_NEAR_UNKNOWN_FC42, continuity_count);
                                }
                            }
                            else if(continuity_count != 0u)
                            {
                                discontinuity_seen = 1u;
                            }
                        }
                        previous_first = current_first;
                        previous_second = current_second;
                    }
                }
                else if(IsSpecialControllerRecordModeEnabled() != 0u && (first_code >> 20) == 3u)
                {
                    s32 masked_second = (s32)(second_code & 0xffffful);

                    if(request_start <= masked_second && masked_second <= request_end)
                    {
                        u32 position = (u32)ExpandLogicalBlockAddressToThirtyNineFramePosition(masked_second);

                        ++matching_count;
                        WriteRecordScanU16(FW_NEAR_UNKNOWN_FC40, matching_count);
                        if(normal_record_count == 0u && ReadRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT) == 0ul)
                            InitializeRecordScanSummary(position, position);
                        else if(position < ReadRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT))
                            WriteRecordScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT, position);
                    }
                }
            }
        }
    }

    matching_count = ReadRecordScanU16(FW_NEAR_UNKNOWN_FC40);
    WriteRecordScanU16(FW_NEAR_UNKNOWN_FC40, 0u);
    return matching_count;
}
