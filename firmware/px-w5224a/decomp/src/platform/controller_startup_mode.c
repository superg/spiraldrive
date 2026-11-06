#include "controller_startup_mode.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_startup_mode_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd9f6e */
void ConfigureControllerStartupModeRegisters(u8 run_mode_one_handshake)
{
    u8 controller_shadow;
    u8 startup_mode;
    u8 poll_count;

    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805B, 0x37u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 3u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018054, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018058, 8u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018059, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 0u);
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) >> 3) & 1u) != 0u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01804C, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018048, 0x40u);

    startup_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C79);
    switch(startup_mode)
    {
    case 0u:
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 2u);
        break;
    case 1u:
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_00FD) & 0xf9u) == 0x99u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_00FF) & 1u) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xfdu));
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0u);
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 2u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C79, 0u);
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 2u);
            }
        }
        else if(run_mode_one_handshake != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xfdu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xfbu));
            STARTUP_MODE_HANDSHAKE();
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) >> 2) & 1u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xfbu));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 1u);
                FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xf9u));
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 0x81u);
                FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 0x81u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 2u));
            }
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) >> 1) & 1u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FF) | 1u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C79, 0u);
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 2u);
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FF) & 0xfeu));
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0u);
            }
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FF) & 0xfeu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xfdu));
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0u);
        }
        break;
    case 2u:
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 4u);
        poll_count = 0u;
        while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043) & 0x10u) != 0u && poll_count < 6u)
        {
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0x90u);
            STARTUP_MODE_DELAY();
            ++poll_count;
        }
        if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043) & 0x10u) == 0u || (FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043) & 0x20u) == 0u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0xd0u);
        else
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0x10u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0u);
        break;
    case 3u:
        STARTUP_MODE_THREE_TRAP();
        break;
    default:
        break;
    }
}
