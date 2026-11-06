#include "send_cue_sheet.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/send_cue_sheet_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadSendCueTransferLength(void)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/send_cue_sheet_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static fw_addr_t SendCueEntryAddress(u16 entry_index)
{
    return FW_NEAR_UNKNOWN_B4BD + (fw_addr_t)((u16)(entry_index * 8u));
}

static u8 IsSendCueTrackPoint(u8 point)
{
    return (u8)(point > 0u && point < 100u);
}

static u8 IsSendCueDataControl(u8 control)
{
    u8 adr = (u8)(control & 0x0fu);

    return (u8)(adr != 2u && adr != 3u);
}

/* RAM:fadde0 */
void HandleSendCueSheetCommand(void)
{
    u32 transfer_length;
    u16 record_count;
    u16 record_index;
    fw_addr_t entry_address;
    u8 found;

    if(SEND_CUE_VALIDATE(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CDF)) == 0u)
    {
        SEND_CUE_UNSUPPORTED();
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 2u)
    {
        SEND_CUE_SEQUENCE_ERROR();
        return;
    }

    transfer_length = ReadSendCueTransferLength();
    if((transfer_length & 7ul) != 0ul || transfer_length > 0x2800ul)
    {
        SEND_CUE_UNSUPPORTED();
        return;
    }

    for(record_index = 0u; record_index < 0x0a00u; ++record_index)
    {
        fw_addr_t address = FW_NEAR_UNKNOWN_B4BD + (fw_addr_t)(record_index * 4u);

        FirmwareWrite8(address, 0u);
        FirmwareWrite8(address + 1u, 0u);
        FirmwareWrite8(address + 2u, 0u);
        FirmwareWrite8(address + 3u, 0u);
    }

    if(SEND_CUE_RECEIVE(FW_NEAR_UNKNOWN_B4BD, transfer_length) == 4u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 0x04u));
    record_count = (u16)(transfer_length >> 3);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_B4BB, (u8)(record_count >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_B4BC, (u8)record_count);

    if(record_count != 0u)
    {
        record_index = record_count;
        do
        {
            record_index--;
            entry_address = SendCueEntryAddress(record_index);
            if(FirmwareRead8(entry_address + 1u) == 0xaau)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1592, FirmwareRead8(entry_address + 5u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1593, FirmwareRead8(entry_address + 6u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1594, FirmwareRead8(entry_address + 7u));
                break;
            }
        } while(record_index != 0u);
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158F) & 3u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x7fu));
        for(record_index = 0u; record_index < record_count; ++record_index)
        {
            u8 point;
            u8 control;

            entry_address = SendCueEntryAddress(record_index);
            point = FirmwareRead8(entry_address + 1u);
            control = FirmwareRead8(entry_address);
            if(IsSendCueTrackPoint(point) != 0u && IsSendCueDataControl(control) != 0u && (control & 0x40u) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 0x80u));
                break;
            }
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128F, 0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 2u) != 0u)
        {
            u8 speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_158C);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, speed);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, speed);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfdu));
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158C, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        }

        found = 0u;
        for(record_index = 0u; record_index < record_count; ++record_index)
        {
            u8 point;
            u8 control;

            entry_address = SendCueEntryAddress(record_index);
            point = FirmwareRead8(entry_address + 1u);
            control = FirmwareRead8(entry_address);
            if(IsSendCueTrackPoint(point) != 0u && IsSendCueDataControl(control) != 0u && (control & 0x40u) != 0u)
            {
                found = 1u;
                break;
            }
        }
        if(found == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) | 2u));
    }

    SEND_CUE_NO_SENSE();
}
