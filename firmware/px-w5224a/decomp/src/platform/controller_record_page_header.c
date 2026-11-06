#include "controller_record_page_reconciliation.h"
#include "firmware_memory.h"
#include "memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_page_header_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteHeaderU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteHeaderU32LowWordFirst(fw_addr_t address, u32 value)
{
    FirmwareWriteU16(FirmwareAddressIncrement24(address, 2u), (u16)value);
    FirmwareWriteU16(address, (u16)(value >> 16));
}

static void CopyHeaderTriplet(fw_addr_t destination)
{
    FirmwareWrite8(destination, FirmwareReadNear8(FW_NEAR_UNKNOWN_FA28));
    FirmwareWrite8(destination + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_FA29));
    FirmwareWrite8(destination + 2u, FirmwareReadNear8(FW_NEAR_UNKNOWN_FA2A));
}

/* RAM:fa538a */
/* ACC/R7, ERET. */
void RebuildControllerBufferRecordPageHeader(u8 group_index, u8 page_index)
{
    u32 controller_page = FW_HUGE_UNKNOWN_094000 + (u32)group_index * 0x4000ul + (u16)page_index * 0x0800u;
    u32 record_start = controller_page + 0x20ul;
    u32 found;
    u32 source;

    WriteHeaderU32LowWordFirst(FW_HUGE_UNKNOWN_2BA51C, record_start);
    COMPACT_RECORD_PAGE(1u);
    COPY_RECORD_PAGE_FROM(controller_page, FW_NEAR_UNKNOWN_FA08, 0x20u);
    WriteHeaderU32LowWordFirst(FW_HUGE_UNKNOWN_2BA51C, record_start);

    found = FIND_RECORD_PAGE_ENTRY(0ul, 0x20u);
    WriteHeaderU32LowWordFirst(FW_HUGE_UNKNOWN_2B9710, found);
    if(found == 0ul)
    {
        WriteHeaderU16(FW_NEAR_UNKNOWN_FA12, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA21, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA22, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA23, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA25, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA26, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA27, 0xffu);
        CLEAR_RECORD_PAGE_PRESENCE(group_index, page_index);
    }
    else
    {
        u16 compacted_offset = (u16)(found - record_start + 0x20ul);

        WriteHeaderU16(FW_NEAR_UNKNOWN_FA12, compacted_offset);
        COPY_RECORD_PAGE_FROM(found + 3ul, FW_NEAR_UNKNOWN_FA28, 3u);
        CopyHeaderTriplet(FW_NEAR_UNKNOWN_FA21);

        found = FIND_RECORD_PAGE_ENTRY(0ul, 0x31u);
        WriteHeaderU32LowWordFirst(FW_HUGE_UNKNOWN_2B9710, found);
        source = found == 0ul ? record_start + 0x5fdul : found - 3ul;
        COPY_RECORD_PAGE_FROM(source, FW_NEAR_UNKNOWN_FA28, 3u);
        CopyHeaderTriplet(FW_NEAR_UNKNOWN_FA25);
    }

    COPY_RECORD_PAGE_FROM(record_start, FW_NEAR_UNKNOWN_FA28, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA19, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA28) & 0x0fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA1A, FirmwareReadNear8(FW_NEAR_UNKNOWN_FA29));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA1B, FirmwareReadNear8(FW_NEAR_UNKNOWN_FA2A));

    found = FIND_RECORD_PAGE_ENTRY(0ul, 0x20u);
    WriteHeaderU32LowWordFirst(FW_HUGE_UNKNOWN_2B9710, found);
    if(found == 0ul)
    {
        found = FIND_RECORD_PAGE_ENTRY(0ul, 0x31u);
        WriteHeaderU32LowWordFirst(FW_HUGE_UNKNOWN_2B9710, found);
        source = found == 0ul ? record_start + 0x5faul : found - 6ul;
    }
    else
    {
        source = found - 6ul;
    }
    COPY_RECORD_PAGE_FROM(source, FW_NEAR_UNKNOWN_FA28, 3u);
    CopyHeaderTriplet(FW_NEAR_UNKNOWN_FA1D);

    COPY_RECORD_PAGE_TO(controller_page, FW_NEAR_UNKNOWN_FA08, 0x20u);
}
