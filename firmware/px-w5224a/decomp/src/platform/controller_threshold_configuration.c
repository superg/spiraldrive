#include "controller_threshold_configuration.h"
#include "controller_mapping.h"
#include "controller_operating_code_ports.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "operating_code_transition_finalization.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_threshold_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadThresholdConfigurationWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static fw_addr_t ProfileWordAddress(fw_addr_t base, u8 profile_index)
{
    if(profile_index == 0u)
        return FirmwareAddressIncrement24(base, FW_ROM_UNKNOWN_FFFFFE);
    return FirmwareAddressIncrement24(base, (u16)((u16)(profile_index - 1u) * 2u));
}

static void ApplyThresholdParameterProfile(u8 operating_code, u32 threshold_value, u8 *common_ports_enabled, u8 *port_80a8_value, u16 *word_036f, u16 *word_037f)
{
    switch(operating_code)
    {
    case 1u:
        *port_80a8_value = 0u;
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_threshold_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        break;
    case 4u:
        *port_80a8_value = 0x12u;
#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_threshold_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        break;
    case 8u:
        *port_80a8_value = 0x13u;
#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/controller_threshold_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        break;
    case 10u:
        *port_80a8_value = 0x14u;
#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/controller_threshold_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x40u) != 0u)
        {
            *common_ports_enabled = 1u;
            *port_80a8_value = 0x12u;
            *word_036f = 0x77a3u;
            *word_037f = 0x445eu;
            SUBMIT_WORD(ReadThresholdConfigurationWord(FW_NEAR_UNKNOWN_0CE9), 0x035fu);
        }
        break;
    case 12u:
        *port_80a8_value = 0x14u;
#define PLEXTOR_BINDING_SECTION 6
#include "bindings/platform/controller_threshold_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        break;
    case 16u:
        *port_80a8_value = 0x14u;
#define PLEXTOR_BINDING_SECTION 7
#include "bindings/platform/controller_threshold_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        break;
    default:
        *common_ports_enabled = 1u;
        *port_80a8_value = 0x14u;
        SET_HIGH_THRESHOLDS(operating_code);
        SELECT_SECONDARY((s32)threshold_value);
#define PLEXTOR_BINDING_SECTION 8
#include "bindings/platform/controller_threshold_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        SUBMIT_WORD(ReadThresholdConfigurationWord(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x34u ? FW_NEAR_UNKNOWN_0CF1 : FW_NEAR_UNKNOWN_0CF3), 0x035fu);
        break;
    }
}

/* RAM:f8b9e6 */
void ConfigureControllerForCurrentOperatingCodeAndThresholdValue(u32 threshold_value)
{
    u8 operating_code;
    u8 profile_index;
    u8 register_01_value;
    u8 common_ports_enabled = 0u;
    u8 port_80a8_value;
    u16 word_036f;
    u16 word_037f;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 4u)
    {
        SET_P1_BITS();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC15, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15) & 0xbfu));
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 4u)
    {
        SET_P1_BITS();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC15, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15) | 0x40u));
    }
    else
    {
        SET_P1_BITS();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC15, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15) | 0x40u));
    }

    register_01_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 10u)
    {
        UPDATE_STATE_103A();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x40u) != 0u)
            register_01_value = 4u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x14u)
        register_01_value = 0xffu;
    WRITE_REGISTER_01(register_01_value);

    profile_index = MAP_PROFILE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    word_036f = ReadThresholdConfigurationWord(ProfileWordAddress(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF84CE), profile_index));
    profile_index = MAP_PROFILE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    word_037f = ReadThresholdConfigurationWord(ProfileWordAddress(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF84E6), profile_index));

    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    ApplyThresholdParameterProfile(operating_code, threshold_value, &common_ports_enabled, &port_80a8_value, &word_036f, &word_037f);

    STROBE_PORT(FW_CONTROLLER_UNKNOWN_018006, 0xc0u);
    CONFIGURE_COMMON_PORTS(common_ports_enabled);
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180A8, port_80a8_value);
    SUBMIT_WORD(word_036f, 0x036fu);
    SUBMIT_WORD(word_037f, 0x037fu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125D, 0xffu);
    CONFIGURE_FINAL_PORTS(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D) != FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) >> 3) & 1u) == 1u)
        LOAD_PROFILE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    STROBE_PORT(FW_CONTROLLER_UNKNOWN_018007, 8u);
    FINALIZE_TRANSITION((s32)threshold_value);
}
