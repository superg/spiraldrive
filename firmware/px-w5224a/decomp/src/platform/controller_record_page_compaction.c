#include "controller_record_page_compaction.h"
#include "controller_buffer_transfer.h"
#include "firmware_memory.h"
#include "memory.h"

#define RECORD_PAGE_SCRATCH FW_HUGE_UNKNOWN_2B971C
#define RECORD_PAGE_KEY_TABLE FW_HUGE_UNKNOWN_2B9D1C

static u32 ReadRecordCompactionU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRecordCompactionU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u32 ReadRecordCompactionPageAddress(void)
{
    u16 low_word = FirmwareReadU16(FW_HUGE_UNKNOWN_2BA51E);
    u16 high_word = FirmwareReadU16(FW_HUGE_UNKNOWN_2BA51C);

    return ((u32)high_word << 16) | low_word;
}

static u32 PackRecordCompactionTriplet(fw_addr_t address)
{
    return ((u32)FirmwareRead8(address) << 16) | ((u32)FirmwareRead8(address + 1u) << 8) | FirmwareRead8(address + 2u);
}

static void StoreRecordCompactionKeys(u16 index, fw_addr_t record_address)
{
    fw_addr_t table_address = RECORD_PAGE_KEY_TABLE + (u16)(index * 8u);

    WriteRecordCompactionU32(table_address, PackRecordCompactionTriplet(record_address));
    WriteRecordCompactionU32(table_address + 4u, PackRecordCompactionTriplet(record_address + 3u));
}

static void SwapRecordCompactionEntries(u16 left, u16 right)
{
    fw_addr_t left_address = RECORD_PAGE_KEY_TABLE + (u16)(left * 8u);
    fw_addr_t right_address = RECORD_PAGE_KEY_TABLE + (u16)(right * 8u);
    u32 left_first = ReadRecordCompactionU32(left_address);
    u32 left_second = ReadRecordCompactionU32(left_address + 4u);
    u32 right_first = ReadRecordCompactionU32(right_address);
    u32 right_second = ReadRecordCompactionU32(right_address + 4u);

    WriteRecordCompactionU32(right_address, left_first);
    WriteRecordCompactionU32(right_address + 4u, left_second);
    WriteRecordCompactionU32(left_address, right_first);
    WriteRecordCompactionU32(left_address + 4u, right_second);
}

static void SortRecordCompactionEntries(u16 count, u8 use_first_key)
{
    while(count != 0u)
    {
        u16 index;
        u16 maximum_index = 0u;
        u32 maximum = 0ul;

        for(index = 0u; index < count; ++index)
        {
            fw_addr_t address = RECORD_PAGE_KEY_TABLE + (u16)(index * 8u) + (use_first_key != 0u ? 0u : 4u);
            u32 key = ReadRecordCompactionU32(address) & 0x0ffffffful;

            if(key > maximum)
            {
                maximum = key;
                maximum_index = index;
            }
        }
        --count;
        if(maximum_index != count)
            SwapRecordCompactionEntries(maximum_index, count);
    }
}

static void RebuildCompactedRecordScratch(u16 count)
{
    u16 index;

    for(index = 0u; index < count; ++index)
    {
        fw_addr_t table_address = RECORD_PAGE_KEY_TABLE + (u16)(index * 8u);
        fw_addr_t record_address = RECORD_PAGE_SCRATCH + (u16)(index * 6u);
        u32 first = ReadRecordCompactionU32(table_address);
        u32 second = ReadRecordCompactionU32(table_address + 4u);

        FirmwareWrite8(record_address, (u8)(first >> 16));
        FirmwareWrite8(record_address + 1u, (u8)(first >> 8));
        FirmwareWrite8(record_address + 2u, (u8)first);
        FirmwareWrite8(record_address + 3u, (u8)(second >> 16));
        FirmwareWrite8(record_address + 4u, (u8)(second >> 8));
        FirmwareWrite8(record_address + 5u, (u8)second);
    }
}

static void WriteCompactedRecordPage(u16 byte_count)
{
    u16 full_chunks = byte_count / 255u;
    u16 chunk;
    u32 destination = ReadRecordCompactionPageAddress();

    if(full_chunks == 0u)
    {
        CopyBytesToAddressPlus1MiB(destination, RECORD_PAGE_SCRATCH, (u8)byte_count);
        return;
    }
    for(chunk = 0u; chunk < full_chunks; ++chunk)
    {
        u16 offset = (u16)(chunk * 255u);

        CopyBytesToAddressPlus1MiB(destination + offset, RECORD_PAGE_SCRATCH + offset, 255u);
    }
    byte_count = (u16)(byte_count - full_chunks * 255u);
    CopyBytesToAddressPlus1MiB(destination + full_chunks * 255u, RECORD_PAGE_SCRATCH + full_chunks * 255u, (u8)byte_count);
}

/* RAM:fc78f1 */
void CompactAndWriteControllerBufferRecordPage(u8 include_type_zero_and_one)
{
    u16 record_offset;
    u16 selected_count = 0u;

    CopyControllerBufferRecordPageToScratch();
    for(record_offset = 0u; record_offset < 0x0800u; ++record_offset)
        FirmwareWrite8(RECORD_PAGE_KEY_TABLE + record_offset, 0u);

    if(include_type_zero_and_one == 0u)
    {
        record_offset = 0u;
        while(record_offset < 0x0600u && (FirmwareRead8(RECORD_PAGE_SCRATCH + record_offset) & 0xf0u) == 0x30u)
        {
            StoreRecordCompactionKeys(selected_count, RECORD_PAGE_SCRATCH + record_offset);
            ++selected_count;
            record_offset = (u16)(record_offset + 6u);
        }
    }
    else
    {
        for(record_offset = 0u; record_offset < 0x0600u; record_offset = (u16)(record_offset + 6u))
        {
            u8 type = (u8)(FirmwareRead8(RECORD_PAGE_SCRATCH + record_offset) & 0xf0u);

            if(type == 0u || type == 0x10u)
            {
                StoreRecordCompactionKeys(selected_count, RECORD_PAGE_SCRATCH + record_offset);
                ++selected_count;
            }
        }
    }

    SortRecordCompactionEntries(selected_count, include_type_zero_and_one);
    RebuildCompactedRecordScratch(selected_count);
    WriteCompactedRecordPage((u16)(selected_count * 6u));
}
