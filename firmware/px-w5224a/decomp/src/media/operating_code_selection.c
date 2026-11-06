#include "operating_code_selection.h"
#include "../platform/firmware_memory.h"
#include "../platform/operating_code_transition_finalization.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/operating_code_selection_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static s32 ReadBigEndianS32ForOperatingCode(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

/* RAM:fc8b2a */
void SelectAndApplyOperatingCodeForMediaState(u8 use_settling_wait)
{
    u8 selection;
    u8 constraint_seed;
    u8 current_code;
    u8 applied_code;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 4u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4D, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xfbu));

    selection = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44);
    constraint_seed = (u8)(selection - 1u);
    switch(selection)
    {
    case 0u:
        constraint_seed = 1u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
        break;
    case 1u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E45) == 1u)
        {
            ApplyBoundedOperatingCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46), 0x34u);
            constraint_seed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
        }
        else
        {
            constraint_seed = 1u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
        }
        break;
    case 4u:
    case 8u:
    case 0x18u:
    case 0x20u:
    case 0x28u:
    case 0x34u:
        constraint_seed = selection;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, selection);
        break;
    case 5u:
    {
        u8 upper_bound = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);
        u8 selector_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E49);

        ApplyBoundedOperatingCode(selector_value, upper_bound);
        constraint_seed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
        break;
    }
    default:
        break;
    }

    current_code = ApplyOperatingCodeConstraint(constraint_seed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, current_code);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, current_code);
    applied_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D);
    if(current_code != applied_code || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) >> 3) & 1u) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            u8 value = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5);

            if(current_code < 0x20u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1307, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1307) & 0xfeu));
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1307, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1307) | 1u));
                value = (u8)(value + 9u);
            }
            ApplyOperatingCodeE8E9(value);
        }
        ApplyOperatingCodeController(1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xf7u));
    }

    if(use_settling_wait == 1u)
    {
        if(current_code != applied_code)
            ApplyOperatingCodeSettlingWait();
    }
    else
    {
        ApplyOperatingCodeFinalization(ReadBigEndianS32ForOperatingCode(FW_NEAR_UNKNOWN_0EF5));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4D, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
}
