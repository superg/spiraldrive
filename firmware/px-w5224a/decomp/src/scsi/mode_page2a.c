#include "mode_page2a.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/mode_page2a_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. */
static u32 ReadModePage2ABigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:fc31c4 */
void PLEXTOR_NEAR_FUNCTION UpdateModePage2ACapabilitiesRuntimeImage(void)
{
    u8 special_media_range = 0u;
    u8 maximum_write_speed_x = 52u;
    u8 reported_write_speed_x;
    s32 last_lead_out;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 1u)
    {
        last_lead_out = (s32)ReadModePage2ABigEndianU32(FW_NEAR_CURRENT_FRAME_POSITION);
        if(last_lead_out > 0x00020f58l && last_lead_out < 0x0004ce78l)
            special_media_range = 1u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C35) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6B1, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E6B1) | 0x02u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6B1, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_E6B1) & 0xfdu));

    WRITE_MODE_PAGE_2A_SPEED(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C43), FW_NEAR_UNKNOWN_E6B3);
    WRITE_MODE_PAGE_2A_SPEED(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37), FW_NEAR_UNKNOWN_E6B9);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x02u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        NORMALIZE_MODE_PAGE_2A_WRITE_SPEED();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
        {
            maximum_write_speed_x = 24u;
        }
        else
        {
            switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80))
            {
            case 0u:
            case 1u:
                maximum_write_speed_x = 0u;
                break;
            case 2u:
                maximum_write_speed_x = 4u;
                break;
            default:
                maximum_write_speed_x = 10u;
                break;
            }
        }
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x02u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u)
        {
            NORMALIZE_MODE_PAGE_2A_WRITE_SPEED();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
            maximum_write_speed_x = 8u;
        }
        else if(special_media_range != 0u)
        {
            maximum_write_speed_x = 8u;
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0x01u) != 0u)
    {
        maximum_write_speed_x = 4u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_158D) == 2u)
            maximum_write_speed_x = 8u;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 0x01u) != 0u)
        maximum_write_speed_x = 4u;

    WRITE_MODE_PAGE_2A_SPEED(maximum_write_speed_x == 0u ? 4u : maximum_write_speed_x, FW_NEAR_UNKNOWN_E6BD);
    POPULATE_MODE_PAGE_2A_DESCRIPTORS(maximum_write_speed_x, 0x1eu);

    reported_write_speed_x = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C7E);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u || special_media_range != 0u))
    {
        u8 eligible_speed_x = SELECT_MODE_PAGE_2A_LOWEST_SPEED();

        if(eligible_speed_x < reported_write_speed_x)
            reported_write_speed_x = eligible_speed_x;
    }
    if(maximum_write_speed_x < reported_write_speed_x)
        reported_write_speed_x = maximum_write_speed_x;
    WRITE_MODE_PAGE_2A_SPEED(reported_write_speed_x, FW_NEAR_UNKNOWN_E6BF);
    WRITE_MODE_PAGE_2A_SPEED(reported_write_speed_x, FW_NEAR_UNKNOWN_E6C7);
}

/* Reconstruction helper; no direct Ghidra address. */
static u16 ReadModePage2ARecordWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; no direct Ghidra address. */
static void AppendModePage2APerformanceDescriptor(fw_addr_t record, u8 destination_offset)
{
    fw_addr_t destination = FW_NEAR_UNKNOWN_E6AB + destination_offset;
    u8 descriptor_flag = (u8)((FirmwareRead8(record + 2u) & 0x03u) == 1u);

    FirmwareWrite8(destination, 0u);
    FirmwareWrite8(destination + 1u, descriptor_flag);
    FirmwareWrite8(destination + 2u, FirmwareRead8(record + 3u));
    FirmwareWrite8(destination + 3u, FirmwareRead8(record + 4u));
}

/* RAM:fc3460 */
void PLEXTOR_NEAR_FUNCTION PopulateModePage2AWriteSpeedPerformanceDescriptors(u8 maximum_speed_x, u8 count_offset)
{
    const fw_addr_t table_base = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8C54);
    u8 destination_offset = (u8)(count_offset + 2u);
    u8 descriptor_count = 0u;
    u8 record_count;
    u8 record_index;
    u8 clear_offset;
    u8 media_state;

    for(clear_offset = count_offset; clear_offset < 0x40u; ++clear_offset)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6AB + clear_offset, 0u);

    record_count = FirmwareRead8(table_base);
    media_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    for(record_index = 0u; record_index < record_count; ++record_index)
    {
        fw_addr_t record = table_base + 5u + (u16)record_index * 5u;
        u8 include = 0u;

        if(media_state == 2u)
        {
            u8 record_mask;

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
                record_mask = 4u;
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) < 2u)
                record_mask = 0u;
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) < 4u)
                record_mask = 1u;
            else
                record_mask = 2u;
            if((FirmwareRead8(record) & record_mask) != 0u && ReadModePage2ARecordWord(record + 3u) / 0x00b0u <= maximum_speed_x)
                include = 1u;
        }
        else if(media_state == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 0u)
            {
                if((FirmwareRead8(record + 2u) & 0x80u) != 0u)
                    include = 1u;
            }
            else if((FirmwareRead8(record + 2u) & 0x80u) != 0u && ReadModePage2ARecordWord(record + 3u) / 0x00b0u <= maximum_speed_x && (FirmwareRead8(record) & 0x20u) == 0u)
            {
                include = 1u;
            }
        }
        else if((FirmwareRead8(record + 2u) & 0x80u) != 0u)
        {
            include = 1u;
        }

        if(include != 0u)
        {
            AppendModePage2APerformanceDescriptor(record, destination_offset);
            destination_offset = (u8)(destination_offset + 4u);
            ++descriptor_count;
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6AB + count_offset, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6AC + count_offset, descriptor_count);
}
