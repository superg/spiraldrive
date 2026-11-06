#include "controller_record_page_reconciliation.h"
#include "controller_buffer_transfer.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_page_reconciliation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Keep these operations at their canonical call sites.  C251 does not
 * support the C99 inline keyword, and out-of-line reconstruction helpers add
 * RET stack boundaries that are absent from the original functions. */
#define RecordPageAddress(group_index, page_index) \
    (FW_HUGE_UNKNOWN_194000 + (fw_addr_t)(group_index) * 0x4000ul + (u16)(page_index) * 0x0800u)
#define RecordPageControllerAddress(group_index, page_index) \
    (FW_HUGE_UNKNOWN_094000 + (u32)(group_index) * 0x4000ul + (u16)(page_index) * 0x0800u)
#define ReadRecordPageU16(address) FirmwareReadU16(address)
#define ReadRecordPageU24(address) \
    (((u32)FirmwareRead8(address) << 16) | ((u32)FirmwareRead8((address) + 1u) << 8) | FirmwareRead8((address) + 2u))
#define ReadSixByteRecord(address, record) \
    do \
    { \
        u8 record_index; \
        for(record_index = 0u; record_index < 6u; ++record_index) \
            (record)[record_index] = FirmwareRead8((address) + record_index); \
    } while(0)
#define WriteSixByteRecord(address, record) \
    do \
    { \
        u8 record_index; \
        for(record_index = 0u; record_index < 6u; ++record_index) \
            FirmwareWrite8((address) + record_index, (record)[record_index]); \
    } while(0)
#define CopySixByteRecord(destination, source) \
    do \
    { \
        u8 record_index; \
        for(record_index = 0u; record_index < 6u; ++record_index) \
            (destination)[record_index] = (source)[record_index]; \
    } while(0)
#define RecordFirstPosition(record) \
    (((u32)((record)[0] & 0x0fu) << 16) | ((u32)(record)[1] << 8) | (record)[2])
#define WriteHeaderPosition(address, record) \
    do \
    { \
        FirmwareWrite8((address), (u8)((record)[0] & 0x0fu)); \
        FirmwareWrite8((address) + 1u, (record)[1]); \
        FirmwareWrite8((address) + 2u, (record)[2]); \
    } while(0)
#define ReadRecordPayloadEndOffset(page_value, result) \
    do \
    { \
        fw_addr_t record_page = (page_value); \
        (result) = ReadRecordPageU16(record_page + 0x0au); \
        if((result) == 0u) \
        { \
            (result) = ReadRecordPageU16(record_page + 0x0cu); \
            if((result) == 0u) \
                (result) = (u16)(ReadRecordPageU16(record_page + 8u) * 6u + 0x20u); \
        } \
    } while(0)

/* RAM:fa5c30 */
/* ACC/R7, ERET. */
void ReconcileControllerBufferRecordsBackward(u8 group_index, u8 page_index)
{
    u8 original_group = group_index;
    u8 original_page = page_index;
    u8 selected_record[6];
    u8 displaced_record[6];
    u32 selected_position;
    u8 traversed_pages = 0u;
    fw_addr_t page;

    page = RecordPageAddress(group_index, page_index);
    ReadSixByteRecord(page + 0x20u, selected_record);
    selected_position = RecordFirstPosition(selected_record);

    for(;;)
    {
        u32 last_position;
        u32 first_position;

        ++traversed_pages;
        if(traversed_pages > 0x40u)
            return;

        if(page_index == 0u)
        {
            if(group_index == 0u)
                break;
            --group_index;
            page_index = 7u;
        }
        else
        {
            --page_index;
        }

        page = RecordPageAddress(group_index, page_index);
        last_position = ReadRecordPageU24(page + 0x15u);
        if(last_position == 0x00fffffful)
            continue;
        if(last_position > selected_position)
            break;

        first_position = ReadRecordPageU24(page + 0x11u);
        if(first_position > selected_position || (group_index == 0u && page_index == 0u))
        {
            u16 end_offset;

            ReadRecordPayloadEndOffset(page, end_offset);

            ReadSixByteRecord(page + end_offset - 6u, displaced_record);
            WriteSixByteRecord(page + end_offset - 6u, selected_record);
            REBUILD_RECORD_PAGE(group_index, page_index);
            CopySixByteRecord(selected_record, displaced_record);
            break;
        }

        if((traversed_pages & 1u) != 0u)
            YIELD_BACKWARD_RECONCILER();
    }

    while(traversed_pages >= 2u)
    {
        u32 last_position;

        --traversed_pages;
        ++page_index;
        if(page_index == 8u)
        {
            ++group_index;
            page_index = 0u;
        }
        if(group_index == original_group && page_index == original_page)
            break;

        page = RecordPageAddress(group_index, page_index);
        last_position = ReadRecordPageU24(page + 0x15u);
        if(last_position != 0x00fffffful)
        {
            u16 end_offset;

            ReadRecordPayloadEndOffset(page, end_offset);

            ReadSixByteRecord(page + end_offset - 6u, displaced_record);
            if(end_offset > 0x26u)
            {
                u32 controller_page = RecordPageControllerAddress(group_index, page_index);
                u32 word_count = (u16)(end_offset - 0x26u) >> 1;

                TRANSFER_RECORD_PAGE_WORDS(controller_page + 0x20ul, 0x000b4000ul, word_count);
                TRANSFER_RECORD_PAGE_WORDS(0x000b4000ul, controller_page + 0x26ul, word_count);
            }
            ReadSixByteRecord(page + 0x20u, selected_record);
            WriteHeaderPosition(page + 0x11u, selected_record);
            WriteHeaderPosition(page + 0x15u, displaced_record);
            CopySixByteRecord(selected_record, displaced_record);
            if((traversed_pages & 1u) != 0u)
                YIELD_BACKWARD_RECONCILER();
        }
    }

    page = RecordPageAddress(original_group, original_page);
    ReadSixByteRecord(page + 0x20u, selected_record);
    WriteHeaderPosition(page + 0x11u, selected_record);
}

/* RAM:fa56c5 */
/* ACC/R7, ERET. */
void ReconcileControllerBufferRecordsForward(u8 group_index, u8 page_index)
{
    u8 original_group = group_index;
    u8 original_page = page_index;
    u8 selected_record[6];
    u8 displaced_record[6];
    u32 selected_position;
    u8 traversed_pages = 0u;
    fw_addr_t page;
    u16 end_offset;

    page = RecordPageAddress(group_index, page_index);
    ReadRecordPayloadEndOffset(page, end_offset);
    ReadSixByteRecord(page + end_offset - 6u, selected_record);
    selected_position = RecordFirstPosition(selected_record);

    for(;;)
    {
        u16 record_count;
        u32 first_position;
        u32 last_position;

        ++traversed_pages;
        if(traversed_pages > 0x40u)
            return;

        ++page_index;
        if(page_index == 8u)
        {
            ++group_index;
            page_index = 0u;
            if(group_index > 7u)
                break;
        }

        page = RecordPageAddress(group_index, page_index);
        record_count = ReadRecordPageU16(page + 8u);
        first_position = ReadRecordPageU24(page + 0x11u);
        if(record_count == 0u)
            break;
        if(first_position == 0x00fffffful)
            continue;
        if(first_position > selected_position)
            break;

        last_position = ReadRecordPageU24(page + 0x15u);
        if(last_position < selected_position || (group_index == 7u && page_index == 7u))
        {
            ReadSixByteRecord(page + 0x20u, displaced_record);
            WriteSixByteRecord(page + 0x20u, selected_record);
            REBUILD_RECORD_PAGE(group_index, page_index);
            CopySixByteRecord(selected_record, displaced_record);
            break;
        }

        if((traversed_pages & 1u) != 0u)
            YIELD_BACKWARD_RECONCILER();
    }

    while(traversed_pages >= 2u)
    {
        u32 first_position;

        --traversed_pages;
        if(page_index == 0u)
        {
            --group_index;
            page_index = 7u;
        }
        else
        {
            --page_index;
        }
        if(group_index == original_group && page_index == original_page)
            break;

        page = RecordPageAddress(group_index, page_index);
        first_position = ReadRecordPageU24(page + 0x11u);
        if(first_position != 0x00fffffful)
        {
            ReadSixByteRecord(page + 0x20u, displaced_record);
            ReadRecordPayloadEndOffset(page, end_offset);
            if(end_offset > 0x26u)
            {
                u32 controller_page = RecordPageControllerAddress(group_index, page_index);
                u32 word_count = (u16)(end_offset - 0x26u) >> 1;

                TRANSFER_RECORD_PAGE_WORDS(controller_page + 0x26ul, controller_page + 0x20ul, word_count);
            }
            WriteSixByteRecord(page + end_offset - 6u, selected_record);
            WriteHeaderPosition(page + 0x11u, displaced_record);
            WriteHeaderPosition(page + 0x15u, selected_record);
            CopySixByteRecord(selected_record, displaced_record);
            if((traversed_pages & 1u) != 0u)
                YIELD_BACKWARD_RECONCILER();
        }
    }

    page = RecordPageAddress(original_group, original_page);
    ReadRecordPayloadEndOffset(page, end_offset);
    WriteSixByteRecord(page + end_offset - 6u, selected_record);
    WriteHeaderPosition(page + 0x15u, selected_record);
}
