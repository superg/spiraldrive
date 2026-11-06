#include "controller_record_workspace_page.h"
#include "firmware_memory.h"

static u32 ReadWorkspaceTableU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static u8 ReadWorkspaceTableByte1(u8 segment_index)
{
    u32 value = ReadWorkspaceTableU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84590) + (fw_addr_t)segment_index * 4u);

    return (u8)(value >> 16);
}

static u8 ReadWorkspaceTableByte2(u8 segment_index)
{
    u32 value = ReadWorkspaceTableU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84590) + (fw_addr_t)segment_index * 4u);

    return (u8)(value >> 8);
}

/* RAM:faa502 */
/* ACC/R7, ERET. */
void BuildControllerRecordWorkspacePage(u8 bank_index, u8 page_index)
{
    u16 full_segment_index = (u16)((u16)bank_index * 8u + (u16)page_index);
    u8 segment_index = (u8)full_segment_index;
    u8 active_segment_count;
    u16 offset;

    for(offset = 0u; offset < 0x0800u; ++offset)
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA62F + offset, 0u);

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA62F, 0x4du);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA630, 0x44u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA631, 0x54u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA632, (u8)((u8)(bank_index << 4) | page_index));
    active_segment_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA588);
    if(full_segment_index < (u16)active_segment_count)
    {
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA637, 1u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA638, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA639, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA63A, 0x20u);
    }
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA63E, 0xffu);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA640, 0xffu);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA641, 0xffu);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA642, 0xffu);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA644, 0xffu);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA645, 0xffu);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA646, 0xffu);

    active_segment_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA588);
    if(segment_index < active_segment_count)
    {
        u8 summary_byte1 = ReadWorkspaceTableByte1(segment_index);
        u8 summary_byte2 = ReadWorkspaceTableByte2(segment_index);
        u16 record_index;

        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA648, summary_byte1);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA649, summary_byte2);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA64A, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA64E, 0xffu);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA64C, summary_byte1);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA64D, summary_byte2);
        for(record_index = 0u; record_index < 0x0100u; ++record_index)
        {
            fw_addr_t record = FW_HUGE_UNKNOWN_2BA64F + (fw_addr_t)record_index * 6u;

            FirmwareWrite8(record, 0x20u);
            FirmwareWrite8(record + 1u, 0u);
            FirmwareWrite8(record + 2u, 0u);
            FirmwareWrite8(record + 3u, ReadWorkspaceTableByte1(segment_index));
            FirmwareWrite8(record + 4u, ReadWorkspaceTableByte2(segment_index));
            FirmwareWrite8(record + 5u, (u8)(record_index >> 8));
        }
    }
    else
    {
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA64A, 0xffu);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA649, 0xffu);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA648, 0xffu);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA64E, 0xffu);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA64D, 0xffu);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA64C, 0xffu);
    }
}
