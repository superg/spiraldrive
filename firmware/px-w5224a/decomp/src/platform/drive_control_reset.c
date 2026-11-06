#include "drive_control_reset.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "interrupt_control.h"
#include "leaf_workers.h"
#include "media_interrupt_leaves.h"
#include "operating_code_transition_finalization.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/drive_control_reset_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/drive_control_reset_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fb095f */
void ResolveDriveControlStateToZero(void)
{
    u8 saved_operating_code = RESOLUTION_CAPTURE_R15();
    u8 value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfbu));
    RESOLUTION_RESET_PORTS();
    RESOLUTION_RAMP_WORD_0255();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 8u)
    {
        RESOLUTION_RESET_HARDWARE();
        RESOLUTION_RESET_OPERATING_STATE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 1u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 5u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 1u) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 1u);

    while((value = FirmwareReadNear8(FW_NEAR_UNKNOWN_1300)) != 0u)
    {
        switch(value)
        {
        case 1u:
        case 4u:
        case 6u:
        case 7u:
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 2u);
            RESOLUTION_SET_CONTROLLER_FLAG(0u);
            RESOLUTION_WRITE_PARAMETERS(0u);
            RESOLUTION_CLEAR_EXTERNAL_INTERRUPT1();
            value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xfbu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, value);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, value);
            RESOLUTION_RESET_CHANNELS();
            saved_operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
            RESOLUTION_SET_P1_BITS();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4D, 8u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 1u);
            RESOLUTION_CONFIGURE_PORTS(0u);
            RESOLUTION_LOAD_CHANNEL3(1u);
            RESOLUTION_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180A7, 3u);
            break;

        case 2u:
            RESOLUTION_FINALIZE_CODE(270000l);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, saved_operating_code);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, saved_operating_code);
            RESOLUTION_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180A7, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 3u);
            break;

        case 3u:
            RESOLUTION_SET_P1_BITS();
            RESOLUTION_PULSE_0253(100u);
            RESOLUTION_SET_P1_BITS();
            RESOLUTION_PULSE_0253(300u);
            RESOLUTION_SET_P1_BITS();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 0u);
            break;

        default:
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 0u);
            break;
        }
    }

    RESOLUTION_RESET_HARDWARE();
    RESOLUTION_RESET_OPERATING_STATE();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 1u);
}

/* RAM:fdbbca */
void ResetControllerHardwareRegisters(void)
{
    u8 value;

    DisableGlobalInterrupts();
    value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01804B, 0x2au);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, 0x08u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x82u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018006, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018007, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, 0x88u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x08u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018080, 0u);
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BC, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BB, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BD, 1u);

    value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, value);
    EnableFirmwareInterruptSources();

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806A, 0u);
    if(FirmwareReadBit000E() != 0u)
    {
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0x80u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, value);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, value);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, 0u);
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x7fu);
    WRITE_CONTROLLER_REGISTER_01(1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0x80u);
}

/* RAM:fb07ff */
void ResetDriveControlStateToEight(void)
{
    RESET_CONTROLLER_PORTS();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 8u)
    {
        RESET_CONTROLLER_HARDWARE();
        RESET_CONTROLLER_OPERATING_STATE();
        return;
    }

    WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    RESOLVE_DRIVE_CONTROL_STATE();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) & 2u) == 0u)
        HOME_PICKUP();
    RESET_CONTROLLER_HARDWARE();
    RESET_CONTROLLER_OPERATING_STATE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 8u);
}
