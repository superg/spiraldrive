#include "format_unit_code11_scan.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/format_unit_code11_scan_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadFormat11ScanU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteFormat11ScanU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteFormat11ScanU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void ClearFormat11ScanTargets(void)
{
    u8 index;

    for(index = 0u; index < 0x20u; ++index)
        WriteFormat11ScanU32(FW_NEAR_UNKNOWN_FA2B + (u16)((u16)index * 4u), 0ul);
}

static void SaveFormat11ScanTransferBit(void)
{
    u8 transfer = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);
    u8 flags = FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS);

    flags &= 0xbfu;
    if((transfer & 0x80u) != 0u)
        flags |= 0x40u;
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, flags);
}

static void RestoreFormat11ScanTransferBit(void)
{
    u8 saved = FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS);
    u8 flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88);

    flags &= 0x7fu;
    if((saved & 0x40u) != 0u)
        flags |= 0x80u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, flags);
}

static void RestoreFormat11ScanPort3(void)
{
    if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
        SCAN_WRITE_P3_RX(1u);
        SCAN_WRITE_P3_TX(1u);
    }
}

static u8 IsFormat11ScanRecordStateOne(s32 mapped_block_start)
{
    s16 table_offset = (s16)((mapped_block_start - 0x91l) / 0x27l);
    fw_addr_t address = FirmwareAddressIncrementLow16(ReadFormat11ScanU32(FW_NEAR_UNKNOWN_FAFF), (u16)table_offset);

    return FirmwareRead8(address) == 1u;
}

static void WaitForFormat11ScanIdle(void)
{
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u)
        SCAN_YIELD();
}

static void IncrementFormat11ScanRefreshCount(void)
{
    WriteFormat11ScanU32(FW_HUGE_UNKNOWN_2BC6F7, ReadFormat11ScanU32(FW_HUGE_UNKNOWN_2BC6F7) + 1ul);
}

static void RefreshFormat11ScanTargets(u8 target_count, s32 reset_target, s32 mapped_block_start)
{
    u8 index;

    SCAN_RESET_TARGET(reset_target, 1u);
    WaitForFormat11ScanIdle();
    if(IsFormat11ScanRecordStateOne(mapped_block_start))
    {
        for(index = 0u; index < target_count; ++index)
        {
            s32 target = (s32)ReadFormat11ScanU32(FW_NEAR_UNKNOWN_FA2B + (u16)((u16)index * 4u));

            SCAN_UPDATE_PAGE(target, 0u);
            IncrementFormat11ScanRefreshCount();
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
}

static void FinishFormat11ScanRecordSummary(void)
{
    s32 lead_out = (s32)ReadFormat11ScanU32(FW_NEAR_CURRENT_FRAME_POSITION);

    if(SCAN_RECORD_BANKS(1u, lead_out, 0ul) == 0u)
    {
        WriteFormat11ScanU32(FW_NEAR_SCAN_POSITION_FIRST, (u32)lead_out);
        WriteFormat11ScanU32(FW_NEAR_SCAN_POSITION_SECOND, (u32)lead_out);
    }
    else
    {
        WriteFormat11ScanU32(FW_NEAR_SCAN_POSITION_FIRST, ReadFormat11ScanU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT));
        WriteFormat11ScanU32(FW_NEAR_SCAN_POSITION_SECOND, ReadFormat11ScanU32(FW_HUGE_UNKNOWN_2B970C));
    }
}

/* RAM:fa36f0 */
void RunFormatUnitCode11MediaScan(void)
{
    s32 scan_extent;
    s32 last_position = 0;
    s32 reset_target = 0;
    s32 block_start = 0;
    s32 mapped_block_start = 0;
    u8 target_count = 0u;
    u8 relation = 0u;
    u8 status = 0u;

    ClearFormat11ScanTargets();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) != 0u)
        SCAN_FINALIZE_WRITE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA52D, (u8)((FirmwareReadNear8(FW_NEAR_FORMAT_STATE) << 6) | 0x10u));

    if(ReadFormat11ScanU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY) == 0ul)
    {
        WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B02, 0ul);
        scan_extent = (s32)(ReadFormat11ScanU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION) - 0x100ul);
    }
    else
    {
        u32 start = ReadFormat11ScanU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY);

        WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B02, start);
        scan_extent = (s32)(ReadFormat11ScanU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION) - start - 0x100ul);
    }
    WriteFormat11ScanU32(FW_HUGE_UNKNOWN_2BC6F7, 0ul);
    WriteFormat11ScanU32(FW_HUGE_UNKNOWN_2BC6FB, 0ul);
    WriteFormat11ScanU32(FW_HUGE_UNKNOWN_2BC6FF, 0ul);
    WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B06, 1ul);
    SCAN_INITIALIZE_READ_VERIFY();
    SCAN_CLEAR_MODE_BITS();
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfcu));
    FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    SaveFormat11ScanTransferBit();
    WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0F00, 0ul);

    if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0x04u) == 0u && SCAN_PROBE() == 0u)
    {
        RestoreFormat11ScanTransferBit();
        SCAN_SERVICE_HOST();
        return;
    }

    WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0F00, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA07, 1u);
    for(;;)
    {
        s32 current_position;
        u32 total_blocks;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) | 0x80u));
        if(SCAN_READ_BIT000A() != 0u)
        {
            s32 logical = (s32)ReadFormat11ScanU32(FW_NEAR_UNKNOWN_0B02);

            WriteFormat11ScanU32(FW_NEAR_UNKNOWN_15DC, (u32)logical);
            WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B02, (u32)(SCAN_MAP_LOGICAL(logical) - 0x96l));
        }
        else
        {
            SCAN_CLEAR_BIT000C();
        }
        current_position = (s32)ReadFormat11ScanU32(FW_NEAR_UNKNOWN_15DC);
        if(current_position % 0x20l == 0l)
        {
            block_start = current_position;
            mapped_block_start = SCAN_MAP_LOGICAL(block_start);
        }

        if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
            SCAN_WRITE_P3_RX(0u);
            SCAN_WRITE_P3_TX(0u);
        }
        WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0FB0, ReadFormat11ScanU32(FW_NEAR_UNKNOWN_0FB4));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) != 2u)
        {
            SCAN_RESET_QUEUE();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
            SCAN_INITIALIZE_COROUTINE2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
            WriteFormat11ScanU32(FW_NEAR_UNKNOWN_1598, SCAN_RESET_TRANSFER());
            SCAN_YIELD();
        }
        else
        {
            SCAN_POSITION_TRANSFER((s32)SCAN_RESET_TRANSFER());
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1467, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1466, 0u);
        WriteFormat11ScanU16(FW_NEAR_UNKNOWN_122F, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_122E, 10u);
        do
        {
            SCAN_RECEIVE();
            SCAN_YIELD();
            status = SCAN_POLL();
        } while(status == 0xfeu);

        total_blocks = ReadFormat11ScanU32(FW_HUGE_UNKNOWN_2BC6FB) + ReadFormat11ScanU32(FW_NEAR_UNKNOWN_0B06);
        WriteFormat11ScanU32(FW_HUGE_UNKNOWN_2BC6FB, total_blocks);
        current_position = (s32)ReadFormat11ScanU32(FW_NEAR_UNKNOWN_15DC);

        if(status == 0u || status == 2u)
        {
            relation = last_position == (s32)((u32)current_position - 1ul) ? 2u : 1u;
        }
        else if(status == 1u)
        {
            relation = 0u;
            reset_target = current_position;
        }
        else
        {
            u32 cursor;
            s32 numerator;

            if(target_count != 0u)
                RefreshFormat11ScanTargets(target_count, reset_target, mapped_block_start);
            FirmwareWrite8(FW_HUGE_UNKNOWN_2BA52D, (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_2BA52D) | 0x10u));
            cursor = ReadFormat11ScanU32(FW_HUGE_UNKNOWN_2BA5A3) + SCAN_MAP_FIXED(total_blocks);
            WriteFormat11ScanU32(FW_HUGE_UNKNOWN_2BA5A3, cursor + 0x91ul);
            numerator = (s32)((cursor + 0x138ul) << 5);
            WriteFormat11ScanU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY, (u32)(numerator / 0x27l));
            SCAN_RESET_TARGET(reset_target, 1u);
            break;
        }

        if(relation != 0u)
        {
            if(relation == 1u)
            {
                if(block_start == current_position)
                {
                    SCAN_RESET_TARGET(reset_target, 1u);
                    WaitForFormat11ScanIdle();
                    if(IsFormat11ScanRecordStateOne(mapped_block_start))
                    {
                        SCAN_UPDATE_PAGE((s32)((u32)current_position - 1ul), 0u);
                        IncrementFormat11ScanRefreshCount();
                    }
                }
                else
                {
                    SCAN_RESET_TARGET(reset_target, 0u);
                    WriteFormat11ScanU32(FW_NEAR_UNKNOWN_FA2B + (u16)((u16)target_count * 4u), (u32)current_position - 1ul);
                    ++target_count;
                }
            }
            else
            {
                SCAN_RESET_TARGET(reset_target, 0u);
            }
            WriteFormat11ScanU32(FW_NEAR_UNKNOWN_FA2B + (u16)((u16)target_count * 4u), (u32)current_position);
            ++target_count;

            if(status == 0u)
            {
                s32 lower_limit = (s32)((u32)block_start + 0x1dul);

                if(lower_limit < current_position)
                {
                    u8 requested = (s32)((u32)block_start + 0x1eul) == current_position ? 1u : 0u;

                    target_count = SCAN_APPEND_TARGETS(requested, target_count);
                    last_position = current_position;
                    relation = 0u;
                }
                else
                {
                    target_count = SCAN_APPEND_TARGETS(1u, target_count);
                    last_position = current_position;
                    WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B02, (u32)current_position + 1ul);
                    relation = 0u;
                    continue;
                }
            }
            else
            {
                last_position = current_position;
                if((s32)((u32)block_start + 0x1eul) >= current_position)
                {
                    WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B02, (u32)current_position + 1ul);
                    WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B06, 1ul);
                    relation = 0u;
                    continue;
                }
                relation = 0u;
            }
        }

        total_blocks = ReadFormat11ScanU32(FW_HUGE_UNKNOWN_2BC6FB);
        if((s32)total_blocks % 0x20l == 0l)
        {
            if(target_count != 0u)
                RefreshFormat11ScanTargets(target_count, reset_target, mapped_block_start);
            target_count = 0u;
            ClearFormat11ScanTargets();
        }
        if((s32)total_blocks < scan_extent)
        {
            WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B02, (u32)current_position + 1ul);
            WriteFormat11ScanU32(FW_NEAR_UNKNOWN_0B06, 1ul);
            continue;
        }

        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x7fu));
        SCAN_RESET_TARGET(reset_target, 1u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA52D, (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_2BA52D) & 0xefu));
        WriteFormat11ScanU32(FW_HUGE_UNKNOWN_2BA5A3, ReadFormat11ScanU32(FW_HUGE_UNKNOWN_2BA59F));
        break;
    }

    RestoreFormat11ScanPort3();
    RestoreFormat11ScanTransferBit();
    if(status == 0xffu)
    {
        SCAN_SERVICE_HOST();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 0x01u));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfeu));
    }
    FinishFormat11ScanRecordSummary();
}
