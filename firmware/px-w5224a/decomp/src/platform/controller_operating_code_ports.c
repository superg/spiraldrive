#include "controller_operating_code_ports.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_code_ports_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define OPERATING_PROFILE_TABLE PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8516)
#define OPERATING_PROFILE_SIZE 8u
#define NO_OPERATING_PROFILE 0x80u

static void ClearOperatingTransitionFlag(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x7fu));
}

/* RAM:f8c1a1 */
void PLEXTOR_NEAR_FUNCTION ConfigureControllerPortsForOperatingCode(u8 operating_code)
{
    u8 profile_index = 0u;
    u8 command_value = 0x2au;
    fw_addr_t profile_address;

    switch(operating_code)
    {
    case 1u:
    case 2u:
        profile_index = 0u;
        ClearOperatingTransitionFlag();
        break;
    case 4u:
        profile_index = 1u;
        ClearOperatingTransitionFlag();
        break;
    case 8u:
        profile_index = (FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 0x04u) != 0u ? 9u : 2u;
        ClearOperatingTransitionFlag();
        break;
    case 10u:
    case 12u:
        profile_index = 3u;
        ClearOperatingTransitionFlag();
        break;
    case 16u:
    case 20u:
        profile_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u ? 5u : 4u;
        ClearOperatingTransitionFlag();
        break;
    case 24u:
    case 32u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        {
            profile_index = 6u;
            ClearOperatingTransitionFlag();
        }
        else
        {
            UPDATE_PROGRESS_WORD();
            profile_index = NO_OPERATING_PROFILE;
        }
        break;
    case 40u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        {
            profile_index = 7u;
            ClearOperatingTransitionFlag();
        }
        else
        {
            UPDATE_PROGRESS_WORD();
            profile_index = NO_OPERATING_PROFILE;
        }
        break;
    default:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        {
            command_value = 0x3au;
            profile_index = 8u;
            ClearOperatingTransitionFlag();
        }
        else
        {
            UPDATE_PROGRESS_WORD();
            profile_index = NO_OPERATING_PROFILE;
        }
        break;
    }

    STROBE_PORT(FW_CONTROLLER_UNKNOWN_01804B, command_value);
    if(profile_index == NO_OPERATING_PROFILE)
        return;

    profile_address = FirmwareAddressIncrementLow16(OPERATING_PROFILE_TABLE, (u16)((u16)profile_index * OPERATING_PROFILE_SIZE));
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_018094, FirmwareRead8(profile_address));
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_018095, FirmwareRead8(FirmwareAddressIncrementLow16(profile_address, 1u)));
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_018096, FirmwareRead8(FirmwareAddressIncrementLow16(profile_address, 2u)));
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_018097, operating_code >= 0x20u ? FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB7) : FirmwareRead8(FirmwareAddressIncrementLow16(profile_address, 3u)));
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_018092, FirmwareRead8(FirmwareAddressIncrementLow16(profile_address, 4u)));
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_018091, FirmwareRead8(FirmwareAddressIncrementLow16(profile_address, 5u)));
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_018093, FirmwareRead8(FirmwareAddressIncrementLow16(profile_address, 6u)));
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_018090, FirmwareRead8(FirmwareAddressIncrementLow16(profile_address, 7u)));
}
