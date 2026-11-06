#include "controller_calibration_configuration.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/controller_calibration_configuration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCalibrationConfigurationU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteCalibrationConfigurationU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:f84d25 */
void LoadControllerCalibrationConfigurationState(void)
{
    u8 profile;
    float scaled_value;

    WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7) & 0xfffdu));
    if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C04B)) == 0xffu)
        WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7) | 2u));
    WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAAC) | 2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xdfu));

    profile = FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F));
    if(profile == 0u)
    {
        WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAAC) & 0xfffdu));
    }
    else if(profile == 0x11u || profile == 0x12u)
    {
        WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAAC) & 0xfffdu));
        WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7) & 0xfffdu));
    }
    else if(profile == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) | 4u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x20u));
        if(CALIBRATION_CONFIGURATION_INDEXED_STATE(4u) == 0u)
            WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7) & 0xfffdu));
    }
    else
    {
        WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadCalibrationConfigurationU16(FW_NEAR_UNKNOWN_FAD7) | 2u));
    }

    CALIBRATION_CONFIGURATION_READ(0x20ul, FW_NEAR_UNKNOWN_FAB4, 0x1au);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB7) == 0xffu)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB7, 0xc0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC2) == 0xffu)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC2, 0x80u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FACA) == 0xffu)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC14, 0u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC14, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FACA) | 1u));

    CALIBRATION_CONFIGURATION_WRITE_E8E9(0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FABC) == 0xffu)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FABC, 100u);
    scaled_value = (float)FirmwareReadNear8(FW_NEAR_UNKNOWN_FABC);
    scaled_value = scaled_value * 234.0f / 100.0f;
    WriteCalibrationConfigurationU16(FW_NEAR_UNKNOWN_0FFE, (u16)(s32)scaled_value);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FACE, FirmwareReadNear8(FW_NEAR_UNKNOWN_FABD));

    CALIBRATION_CONFIGURATION_READ(0x30ul, FW_NEAR_UNKNOWN_FAC4, 4u);
    CALIBRATION_CONFIGURATION_READ(0x3ful, FW_NEAR_UNKNOWN_FAE0, 1u);
    CALIBRATION_CONFIGURATION_WRITE_BIT44((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 4u) != 0u ? 1u : 0u);
    if(CALIBRATION_CONFIGURATION_INDEXED_STATE(4u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) | 0x40u));
}
