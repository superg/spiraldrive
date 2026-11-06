#include "controller_record_page_update.h"
#include "controller_record_page_reconciliation.h"
#include "firmware_memory.h"
#include "memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_page_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRecordUpdateU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRecordUpdateU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteRecordUpdateU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u32 RecordUpdatePageAddress(u16 group, u16 page, u32 base)
{
    return base + (u32)group * 0x4000ul + (u16)(page * 0x0800u);
}

static void PublishRecordUpdateWindow(u32 page)
{
    WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2BA51C, page + 0x20ul);
    WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2BA520, page + 0x620ul);
}

static void CopyRecordUpdateTriplet(fw_addr_t destination)
{
    FirmwareWrite8(destination, FirmwareReadNear8(FW_NEAR_UNKNOWN_FA28));
    FirmwareWrite8(destination + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_FA29));
    FirmwareWrite8(destination + 2u, FirmwareReadNear8(FW_NEAR_UNKNOWN_FA2A));
}

static void UpdateRecordPageHeaderAfterCompaction(s32 frame_position, u32 page, u8 gap_class)
{
    u32 record_start = page + 0x20ul;
    u32 found;
    u32 offset;
    u32 source;

    found = FIND_RECORD(frame_position, 0x31u);
    WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2B9710, found);
    WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2BA51C, found);
    COMPACT_PAGE(gap_class);
    COPY_FROM_PAGE(page, FW_NEAR_UNKNOWN_FA08, 0x20u);
    WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2BA51C, record_start);

    found = FIND_RECORD(frame_position, 0x20u);
    WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2B9710, found);
    if(found == 0ul)
    {
        WriteRecordUpdateU16(FW_NEAR_UNKNOWN_FA12, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA21, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA22, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA23, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA25, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA26, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA27, 0xffu);
    }
    else
    {
        offset = found - page;
        WriteRecordUpdateU16(FW_NEAR_UNKNOWN_FA12, (u16)(offset >> 8));
        COPY_FROM_PAGE(found + 3ul, FW_NEAR_UNKNOWN_FA28, 3u);
        CopyRecordUpdateTriplet(FW_NEAR_UNKNOWN_FA21);

        found = FIND_RECORD(frame_position, 0x31u);
        WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2B9710, found);
        source = found == 0ul ? page + 0x61dul : found - 3ul;
        COPY_FROM_PAGE(source, FW_NEAR_UNKNOWN_FA28, 3u);
        CopyRecordUpdateTriplet(FW_NEAR_UNKNOWN_FA25);
    }

    found = FIND_RECORD(frame_position, 0x31u);
    WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2B9710, found);
    offset = found - page;
    WriteRecordUpdateU16(FW_NEAR_UNKNOWN_FA14, (u16)(offset >> 8));
    COPY_TO_PAGE(page, FW_NEAR_UNKNOWN_FA08, 0x20u);
}

static void InsertRecordIntoGapZone(s32 frame_position, u16 original_group, u16 original_page)
{
    u16 group = original_group;
    u16 page_index = original_page;
    u8 reverse = 0u;
    u8 attempts;
    u32 page = RecordUpdatePageAddress(group, page_index, FW_HUGE_UNKNOWN_300000);
    u32 record_start = page + 0x20ul;
    u32 record_end = record_start + 0x600ul;

    for(attempts = 0u; attempts < 0x40u; ++attempts)
    {
        u32 found = FIND_RECORD(frame_position, 0x20u);

        WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2B9710, found);
        if(found != 0ul && found < record_end)
        {
            u32 offset = found - record_start;
            u32 destination = record_start + offset;

            FirmwareWrite8(FW_HUGE_UNKNOWN_2B971C, (u8)((u32)frame_position >> 16));
            FirmwareWrite8(FW_HUGE_UNKNOWN_2B971D, (u8)((u32)frame_position >> 8));
            FirmwareWrite8(FW_HUGE_UNKNOWN_2B971E, (u8)frame_position);
            COPY_TO_PAGE(destination, FW_HUGE_UNKNOWN_2B971C, 3u);
            record_start = destination + 6ul;
            offset += 6ul;
            REBUILD_PAGE((u8)group, (u8)page_index);
            if(group == original_group && page_index == original_page)
                return;
            CLEAR_PRESENCE(group, page_index);
            if(reverse == 0u)
                RECONCILE_BACKWARD((u8)group, (u8)page_index);
            else
                RECONCILE_FORWARD((u8)group, (u8)page_index);
            return;
        }

        COPY_FROM_PAGE(page, FW_NEAR_UNKNOWN_FA08, 0x20u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA14) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FA15) == 0x20u)
            CLEAR_PRESENCE(group, page_index);

        if(reverse != 0u)
        {
            if(page_index == 0u)
            {
                --group;
                page_index = 7u;
            }
            else
            {
                --page_index;
            }
        }
        else
        {
            ++page_index;
            if((page_index & 7u) == 0u)
            {
                ++group;
                page_index = 0u;
                if(group > 7u)
                {
                    reverse = 1u;
                    group = original_group;
                    page_index = original_page;
                    if(page_index == 0u)
                    {
                        --group;
                        page_index = 7u;
                    }
                    else
                    {
                        --page_index;
                    }
                }
            }
        }
        page = RecordUpdatePageAddress(group, page_index, FW_HUGE_UNKNOWN_094000);
        record_start = page + 0x20ul;
        record_end = record_start + 0x600ul;
        PublishRecordUpdateWindow(page);
    }
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 2u));
}

static void UpdateDirectRecordZone(s32 frame_position, u8 preserved_record_group_count, u8 gap_class)
{
    u32 found = FIND_RECORD(frame_position, 0x30u);
    u32 record_start = ReadRecordUpdateU32(FW_HUGE_UNKNOWN_2BA51C);
    u32 record_end = ReadRecordUpdateU32(FW_HUGE_UNKNOWN_2BA520);
    u32 offset;
    u32 current;
    u32 tail;
    u16 skipped = 0u;
    u16 index;

    WriteRecordUpdateU32(FW_HUGE_UNKNOWN_2B9710, found);
    if(found == 0ul)
        return;

    offset = found - record_start;
    current = found + FW_ADDRESS_BANK_STRIDE;
    tail = record_end + FW_ADDRESS_BANK_STRIDE;
    if(preserved_record_group_count == 0u)
    {
        for(index = 0u; index < 6u; ++index)
            FirmwareWrite8(FW_HUGE_UNKNOWN_2B971C + index, FirmwareRead8(current + index));
        if((FirmwareRead8(FW_HUGE_UNKNOWN_2B971C) & 0xf0u) != 0x20u)
            return;
        FirmwareWrite8(FW_HUGE_UNKNOWN_2B971C, 0x30u);
        for(index = 0u; index < (u16)((0x600ul - offset) / 6ul); ++index)
            COPY_SIX(current + (u32)index * 6ul, current + (u32)index * 6ul + 6ul);
        current += (u16)(index - 1u) * 6ul;
        COPY_TO_PAGE(current - FW_ADDRESS_BANK_STRIDE, FW_HUGE_UNKNOWN_2B971C, 6u);
    }
    else
    {
        u16 preserved_records = (u16)((u16)preserved_record_group_count * 0x20u);
        u16 preserved_bytes = (u16)(preserved_records * 6u);
        u32 remaining_source;
        u16 remaining_bytes;

        for(index = 0u; index < preserved_records; ++index)
        {
            fw_addr_t workspace = FW_HUGE_UNKNOWN_2B971C + (u32)index * 6ul;

            COPY_SIX(workspace, current + (u32)index * 6ul);
            if((FirmwareRead8(workspace) & 0xf0u) == 0x20u)
                FirmwareWrite8(workspace, 0x30u);
            else if((FirmwareRead8(workspace) & 0xf0u) != 0x30u)
                skipped = (u16)(skipped + 6u);
        }

        remaining_source = current + preserved_bytes;
        remaining_bytes = (u16)(tail - remaining_source);
        current += skipped;
        for(index = 0u; index <= (u16)(remaining_bytes / 6u); ++index)
            COPY_SIX(current + (u32)index * 6ul, remaining_source + (u32)index * 6ul);

        if(remaining_bytes == 0u)
        {
            if(preserved_record_group_count == 8u)
                COPY_DWORDS(current - FW_ADDRESS_BANK_STRIDE, FW_HUGE_UNKNOWN_2B971C, (u16)(preserved_bytes >> 2));
        }
        else
        {
            current += (u16)(index - 1u) * 6ul;
            COPY_DWORDS(current - FW_ADDRESS_BANK_STRIDE, FW_HUGE_UNKNOWN_2B971C + skipped, (u16)((preserved_bytes - skipped) >> 2));
        }
    }

    UpdateRecordPageHeaderAfterCompaction(frame_position, record_start - 0x20ul, gap_class);
}

/* RAM:fa4776 */
void UpdateControllerBufferRecordPageForFramePosition(s32 frame_position, u8 preserved_record_group_count)
{
    u16 zone;
    u16 group;
    u16 page;
    u8 gap_class = 0u;

    if(preserved_record_group_count > 8u)
        return;

    for(zone = 0u; zone < 0x40u; ++zone)
    {
        s32 gap_start = (s32)ReadRecordUpdateU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84790) + (u32)zone * 4ul);
        s32 gap_end = (s32)(ReadRecordUpdateU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84790) + (u32)(zone + 1u) * 4ul) - 0x101ul);
        s32 direct_start = (s32)ReadRecordUpdateU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84590) + (u32)zone * 4ul);

        if(frame_position >= gap_start && frame_position <= gap_end)
        {
            gap_class = 1u;
            break;
        }
        if(frame_position >= direct_start && frame_position <= (s32)((u32)direct_start + 0xfful))
            break;
    }

    group = (zone >> 3) & 7u;
    page = (u16)(zone - group * 8u);
    PublishRecordUpdateWindow(RecordUpdatePageAddress(group, page, FW_HUGE_UNKNOWN_300000));
    if(gap_class != 0u)
        InsertRecordIntoGapZone(frame_position, group, page);
    else
        UpdateDirectRecordZone(frame_position, preserved_record_group_count, gap_class);
}
