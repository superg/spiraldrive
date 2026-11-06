#include "controller_state_profile.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_state_profile_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static const u8 ControllerStateProfileFirmwareDefaults[21] = { 0x01u, 0x03u, 0x09u, 0x09u, 0x03u, 0x01u, 0x04u, 0x00u, 0x98u, 0x00u, 0xa3u, 0x00u, 0xa6u, 0x00u, 0xa3u, 0x00u, 0x90u, 0x00u, 0x50u,
    0x00u, 0x0au };

/* RAM:fd81a2 */
void ConfigureControllerStateProfileFromFirmwareDefaults(u8 set_range_high_bit, u16 range_value)
{
    u8 index;

    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_018496, 0xc0u);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_018497, 0x80u);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_018498, 0x7fu);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_018499, 0x81u);

    for(index = 0u; index != 3u; ++index)
        WRITE_CONTROLLER_PORT((fw_addr_t)(FW_CONTROLLER_UNKNOWN_01849A + index), (u8)((ControllerStateProfileFirmwareDefaults[index] << 4) | ControllerStateProfileFirmwareDefaults[index + 3u]));
    for(index = 0u; index != 7u; ++index)
    {
        u8 offset = (u8)(7u + index * 2u);
        u16 value = (u16)(((u16)ControllerStateProfileFirmwareDefaults[offset] << 8) | ControllerStateProfileFirmwareDefaults[offset + 1u]);

        SUBMIT_CONTROLLER_WORD(value, (u16)(0x0180u + index));
    }

    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849E, ControllerStateProfileFirmwareDefaults[6]);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180EE, (u8)range_value);
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180EF, (u8)((range_value >> 8) | (set_range_high_bit != 0u ? 0x80u : 0u)));
    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0xc0u);

    while((s8)FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) < 0)
    {
        if((READ_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849D) & 1u) != 0u)
        {
            WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
            break;
        }
    }
    CONFIGURE_OPERATING_RANGE();
}
