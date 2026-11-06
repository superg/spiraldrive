#include "read_acquisition_controller_config.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/read_acquisition_controller_config_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u8 SelectReadAcquisitionPortC2(u8 media_code, u8 write_speed, u8 operating_code)
{
    if(media_code == 5u)
    {
        if(write_speed < 11u || operating_code < 17u)
            return 0xe0u;
        if(operating_code < 25u)
            return 0xe1u;
        return 0xe2u;
    }
    if(write_speed < 11u)
        return 0xe0u;
    if(operating_code < 17u)
        return 0xe1u;
    if(operating_code < 25u)
        return 0xe3u;
    return 0xe5u;
}

static u8 ReadAcquisitionSpecialOption(void)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u);
}

/* RAM:fab693 */
void ConfigureReadAcquisitionControllerForCurrentState(void)
{
    u8 operating_code;
    u8 controller_shadow;
    u8 state_shadow;

    CONFIG_INTERRUPT(0u);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C1, 0x14u);
    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E52, operating_code);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C2, SelectReadAcquisitionPortC2(FirmwareReadNear8(FW_NEAR_UNKNOWN_1158), FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47), operating_code));

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C0, 0u);
    if(CONFIG_READ_BIT33() == 1u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180CF, FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) == 2u ? 0xc4u : 0xc0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C3, ReadAcquisitionSpecialOption() ? 0x20u : 0xa0u);
    }
    else
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180CF, FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) == 2u ? 4u : 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C3, 0x60u);
    }

    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    CONFIG_WRITE_REGISTER01(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F));
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1158) == 5u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) == 9u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018031, 0x40u);
    else
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018031, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018008, 0x41u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018009, 0u);

    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
    if(operating_code < 20u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0x80u);
    }
    else if(CONFIG_READ_BIT42() == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0u)
    {
        u8 raw_q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);

        if(raw_q_type < 4u || raw_q_type == 9u)
        {
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, 0xf0u);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0x80u);
        }
        else
        {
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, 0xf1u);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0x42u);
        }
    }
    else
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800A, 0xb0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0x80u);
    }

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806E, 0u);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    if(ReadAcquisitionSpecialOption())
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1503, 0x80u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x80u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1503, 0x16u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x16u);
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0x1eu);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
    state_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAB) & 0xdfu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAB, state_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, state_shadow);
    CONFIG_WRITE_BIT43(0u);
    state_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAB) & 0xefu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAB, state_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, state_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x30u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1157, (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018040) & 0x0fu));
    CONFIG_INTERRUPT(1u);
    CONFIG_INTERRUPT(2u);
    CONFIG_INTERRUPT(3u);
}
