#include "set_cd_speed.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/set_cd_speed_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadSetCdSpeedWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteSetCdSpeedWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u8 SelectRequestedReadSpeed(u16 kilobytes_per_second)
{
    if(kilobytes_per_second < 0x02bdu && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x40u) != 0u)
        return 1u;
    if(kilobytes_per_second < 0x057eu)
        return 4u;
    if(kilobytes_per_second < 0x06dfu)
        return 8u;
    if(kilobytes_per_second < 0x1085u)
        return (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x40u) != 0u ? 10u : 8u;
    if(kilobytes_per_second < 0x1608u)
        return 24u;
    if(kilobytes_per_second < 0x1b8bu)
        return 32u;
    if(kilobytes_per_second < 0x210eu)
        return 40u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 4u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 4u) == 0u)
        return 40u;
    return 52u;
}

static u8 SelectRequestedWriteSpeed(u16 kilobytes_per_second, u8 command_mode_flag)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            return 24u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0x40u) != 0u)
            return kilobytes_per_second < 0x06dfu ? 4u : 10u;
        return 4u;
    }

    if(kilobytes_per_second < 0x057eu)
        return 4u;
    if(kilobytes_per_second < 0x0b01u)
        return 8u;
    if(kilobytes_per_second < 0x0dc3u)
        return 16u;
    if(kilobytes_per_second < 0x1085u)
        return 20u;
    if(kilobytes_per_second < 0x1608u)
        return 24u;
    if(kilobytes_per_second < 0x1b8bu)
        return 32u;
    if(kilobytes_per_second < 0x210eu)
        return 40u;
    if(kilobytes_per_second < 0x23d0u)
        return command_mode_flag != 0u ? 52u : 40u;
    return 52u;
}

/* RAM:fc0789 */
void HandleSetCdSpeedCommand(void)
{
    u16 requested_read_kbps;
    u16 requested_write_kbps;
    u8 requested_read_speed;
    u8 requested_write_speed;
    u8 command_mode_flag;

    if(SET_CD_SPEED_VALIDATE(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E32)) == 0u)
    {
        SET_CD_SPEED_UNSUPPORTED();
        return;
    }

    requested_read_kbps = ReadSetCdSpeedWord(FW_NEAR_UNKNOWN_0B0C);
    requested_write_kbps = ReadSetCdSpeedWord(FW_NEAR_UNKNOWN_0B0E);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x40u) != 0u)
    {
        SET_CD_SPEED_NO_SENSE();
        return;
    }

    command_mode_flag = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C7F) == 6u);
    if(READ_SET_CD_SPEED_BIT0008() != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) != 0u)
        RESET_SET_CD_SPEED_PROGRESS();

    requested_read_speed = SelectRequestedReadSpeed(requested_read_kbps);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C37, requested_read_speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC20, requested_read_speed);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C37, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1F));

    requested_write_speed = SelectRequestedWriteSpeed(requested_write_kbps, command_mode_flag);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7E, requested_write_speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7D, requested_write_speed);

    if((ReadSetCdSpeedWord(FW_NEAR_UNKNOWN_FAD7) & 4u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, requested_write_speed);
    }
    else if((s8)requested_write_speed <= (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, requested_write_speed);
        WriteSetCdSpeedWord(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadSetCdSpeedWord(FW_NEAR_UNKNOWN_FAD7) & 0xfffbu));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD3, requested_write_speed);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37) != FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46))
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45))
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
            else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45) > 0)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
            CLEAR_SET_CD_SPEED_BIT000E();
            RESET_SET_CD_SPEED_QUEUE();
            INITIALIZE_SET_CD_SPEED_COROUTINE(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
            DISPATCH_SET_CD_SPEED_REQUEST5();
            YIELD_SET_CD_SPEED();
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 4u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    }

    SET_CD_SPEED_NO_SENSE();
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x80u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 1u));
        FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, (FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 8u) != 0u ? 60u : 10u);
    }
}
