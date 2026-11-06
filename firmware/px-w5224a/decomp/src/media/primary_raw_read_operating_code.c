#include "primary_raw_read_operating_code.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/primary_raw_read_operating_code_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void ClearPrimaryRawReadWord(fw_addr_t address)
{
    FirmwareWrite8(address, 0u);
    FirmwareWrite8(address + 1u, 0u);
}

static void ClampPrimaryRawReadLimit(void)
{
    u8 selected_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E49);

    if(selected_code < FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, selected_code);
}

static void SelectPrimaryRawReadBackoffCode(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u8 selected_code;

    if(operating_code == 2u || operating_code == 4u || operating_code == 1u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x40u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0u);
            if(READ_OPERATING_CODE_BIT_000B() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, 1u);
            return;
        }
        selected_code = 4u;
    }
    else if(operating_code == 8u)
    {
        selected_code = 4u;
    }
    else if(operating_code == 0x18u)
    {
        selected_code = 8u;
    }
    else if(operating_code == 0x20u)
    {
        selected_code = 0x18u;
    }
    else if(operating_code == 0x28u)
    {
        selected_code = 0x20u;
    }
    else
    {
        selected_code = 0x28u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, selected_code);
    ClampPrimaryRawReadLimit();
}

static void SelectPrimaryRawReadAdvanceCode(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u8 current_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);

    if(operating_code == 2u || operating_code == 1u)
    {
        if(current_speed != 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 4u);
    }
    else if(operating_code == 4u)
    {
        if(current_speed != 4u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 8u);
    }
    else if(operating_code == 8u)
    {
        if(current_speed != 8u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 0x18u);
    }
    else if(operating_code == 0x18u)
    {
        if(current_speed != 0x18u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 0x20u);
    }
    else if(operating_code == 0x20u)
    {
        if(current_speed != 0x20u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 0x28u);
    }
    else if(operating_code != 0x28u || current_speed != 0x28u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, current_speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    }
}

/* RAM:f908e6 */
void PLEXTOR_NEAR_FUNCTION UpdatePrimaryRawReadOperatingCodeSelector(void)
{
    u8 state;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u)
            return;
        if(READ_OPERATING_CODE_BIT_0020() != 0u)
            return;
        ClearPrimaryRawReadWord(FW_NEAR_UNKNOWN_0F3B);
    }
    if(READ_OPERATING_CODE_BIT_001C() != 0u)
        return;
    if(READ_OPERATING_CODE_BIT_001D() != 0u)
        return;
    ClearPrimaryRawReadWord(FW_NEAR_UNKNOWN_0F1C);

    state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F05);
    if(state == 3u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E48) == 0u)
        {
            u8 old_countdown;

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 5u);
            old_countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4A);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, (u8)(old_countdown - 1u));
            if((s8)old_countdown <= 0)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, 5u);
                ClearPrimaryRawReadWord(FW_NEAR_UNKNOWN_0F3B);
                SelectPrimaryRawReadBackoffCode();
            }
        }
    }
    else if(state == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E48) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x40u) != 0u ? 0u : 4u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E48) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, 0u);
    }
    else
    {
        u8 current_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);
        u8 selected_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E49);

        if(selected_code == current_speed)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4A, 0u);
        }
        else
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) + 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 3u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 5u);
            SelectPrimaryRawReadAdvanceCode();
        }
    }

    {
        u8 current_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);
        u8 selected_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E49);

        if(selected_code > current_speed)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
    }
}
