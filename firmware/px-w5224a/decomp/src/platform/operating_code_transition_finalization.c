#include "operating_code_transition_finalization.h"
#include "controller_mapping.h"
#include "controller_measurement_transition.h"
#include "controller_parameters.h"
#include "controller_transition_wait.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/operating_code_transition_finalization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteOperatingCodeTransitionU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteOperatingCodeTransitionU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void WriteHighOperatingCodeProfileByte(u8 selector, u8 value)
{
    WRITE_OPERATING_CODE_PORT(FW_CONTROLLER_UNKNOWN_0180CA, selector);
    if(selector != 0u)
        WRITE_OPERATING_CODE_PORT(FW_CONTROLLER_UNKNOWN_0180CB, value);
}

/* RAM:f8c004 */
void FinalizeControllerOperatingCodeTransition(s32 transition_value)
{
    u8 current_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u8 applied_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D);

    if(applied_code != current_code)
    {
        SYNCHRONIZE_OPERATING_CODE();

        if(current_code >= 0x18u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        {
            WriteHighOperatingCodeProfileByte(8u, 0u);
            WriteHighOperatingCodeProfileByte(4u, 1u);
            WriteHighOperatingCodeProfileByte(0u, 0u);
        }

        if(transition_value <= 0)
            transition_value = 150;
        WriteOperatingCodeTransitionU32(FW_NEAR_UNKNOWN_125E, (u32)transition_value);
        APPLY_OPERATING_CODE_PROFILE(1u);

        if(current_code >= 0x10u)
            WriteOperatingCodeTransitionU16(FW_NEAR_UNKNOWN_1200, 1000u);
        else if(applied_code < 8u && current_code < 8u)
            WriteOperatingCodeTransitionU16(FW_NEAR_UNKNOWN_1200, 80u);
        else
            WriteOperatingCodeTransitionU16(FW_NEAR_UNKNOWN_1200, 500u);

        WAIT_FOR_OPERATING_CODE_TRANSITION(1u, 0u);

        if(current_code >= 0x18u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        {
            u8 profile_index = MAP_OPERATING_CODE_PROFILE_INDEX(current_code);
            fw_addr_t profile = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF84FE) + (u16)profile_index * 2u;

            WriteHighOperatingCodeProfileByte(8u, FirmwareRead8(profile));
            WriteHighOperatingCodeProfileByte(4u, FirmwareRead8(profile + 1u));
            WriteHighOperatingCodeProfileByte(0u, 0u);
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0xfeu));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4D, current_code);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4E, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4C));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0xefu));
    SYNCHRONIZE_OPERATING_CODE();
}
