#include "format_unit_code11_probe.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/format_unit_code11_probe_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadFormat11ProbeU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteFormat11ProbeU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteFormat11ProbeU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void RestoreFormat11ProbePort3(void)
{
    if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
        PROBE_WRITE_P3_RX(1u);
        PROBE_WRITE_P3_TX(1u);
    }
}

/* RAM:fa3ef4 */
u8 ProbeFormatUnitCode11MediaRanges(void)
{
    s32 reset_target = 0;
    u8 segment;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA07, 0u);
    for(segment = 0u; segment < FirmwareRead8(FW_HUGE_UNKNOWN_2BA588); ++segment)
    {
        s32 segment_start = (s32)ReadFormat11ProbeU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84590) + (fw_addr_t)segment * 4u);
        s32 mapped_limit = (s32)(ReadFormat11ProbeU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION) - 0x100ul);
        u8 slice;

        if(mapped_limit <= segment_start)
            break;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
        for(slice = 0u; slice < 8u; ++slice)
        {
            s32 logical_position;
            u8 status;

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) | 0x80u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1467, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1466, 0u);
            if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
                PROBE_WRITE_P3_RX(0u);
                PROBE_WRITE_P3_TX(0u);
            }
            WriteFormat11ProbeU32(FW_NEAR_UNKNOWN_0FB0, ReadFormat11ProbeU32(FW_NEAR_UNKNOWN_0FB4));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) != 2u)
            {
                PROBE_RESET_RAW_READ();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
            }

            logical_position = (s32)((u32)segment_start + (u16)((u16)slice * 0x20u));
            WriteFormat11ProbeU32(FW_NEAR_UNKNOWN_15DC, (u32)logical_position);
            WriteFormat11ProbeU32(FW_NEAR_UNKNOWN_1598, (u32)PROBE_MAP(logical_position));
            PROBE_RESET_RAW_READ();
            PROBE_INITIALIZE(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
            PROBE_YIELD();
            WriteFormat11ProbeU16(FW_NEAR_UNKNOWN_0F3B, 0x2308u);
            WriteFormat11ProbeU32(FW_NEAR_UNKNOWN_1455, 0x20ul);
            WriteFormat11ProbeU32(FW_NEAR_UNKNOWN_1459, 0ul);
            WriteFormat11ProbeU16(FW_NEAR_UNKNOWN_122F, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_122E, 0x28u);

            do
            {
                PROBE_RECEIVE();
                PROBE_YIELD();
                status = PROBE_POLL();
            } while(status == 0xfeu);

            if(status == 0xffu)
            {
                PROBE_RESET_TARGET(reset_target, 1u);
                RestoreFormat11ProbePort3();
                return 0u;
            }
            if(status == 0u || status == 2u)
            {
                s32 mapped_position;
                s16 table_offset;
                fw_addr_t table;

                PROBE_RESET_TARGET(reset_target, 1u);
                while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u)
                    PROBE_YIELD();
                mapped_position = PROBE_MAP(logical_position);
                table_offset = (s16)((mapped_position - 0x91) / 0x27);
                table = FirmwareAddressIncrementLow16(ReadFormat11ProbeU32(FW_NEAR_UNKNOWN_FAFF), (u16)table_offset);
                if(FirmwareRead8(table) == 1u)
                {
                    u32 refreshed;

                    PROBE_UPDATE_PAGE(logical_position, 1u);
                    refreshed = ReadFormat11ProbeU32(FW_HUGE_UNKNOWN_2BC6F7) + 0x20ul;
                    WriteFormat11ProbeU32(FW_HUGE_UNKNOWN_2BC6F7, refreshed);
                }
            }
            else
            {
                reset_target = logical_position;
            }
            RestoreFormat11ProbePort3();
        }
    }
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 4u));
    return 1u;
}
