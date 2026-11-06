#include "controller_operating_code_configuration.h"
#include "controller_mapping.h"
#include "controller_operating_code_ports.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_code_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void DispatchParameterProfile(u8 operating_code)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_operating_code_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:f8b8c2 */
void ConfigureControllerForCurrentOperatingCode(u8 reload_operating_profile)
{
    u8 operating_code;
    u8 applied_operating_code;
    u8 channel_value;

    DispatchCurrentOperatingCodeStrobe(FW_CONTROLLER_UNKNOWN_018006, 0x80u);

    /* The native body performs a second volatile read for values at least 4,
     * although all three paths assert the same two P1 outputs. */
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 4u)
        DispatchCurrentOperatingCodeSetP1Bits();
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 4u)
        DispatchCurrentOperatingCodeSetP1Bits();
    else
        DispatchCurrentOperatingCodeSetP1Bits();

    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    DispatchParameterProfile(operating_code);
    DispatchCurrentOperatingCodeCommonPorts(0u);
    DispatchCurrentOperatingCodeWords(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));

    applied_operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D);
    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    if(((applied_operating_code != operating_code) && (reload_operating_profile == 1u)) || (((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) >> 3) & 1u) == 1u))
        DispatchCurrentOperatingCodeLoadProfile(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));

    if(reload_operating_profile == 1u)
    {
        channel_value = DispatchCurrentOperatingCodeMapChannel3(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
        DispatchCurrentOperatingCodeSetChannel(3u, channel_value);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125D, 0xffu);
    DispatchCurrentOperatingCodeFinalPorts(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F));
}
