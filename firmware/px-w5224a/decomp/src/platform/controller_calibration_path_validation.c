#include "controller_calibration_path_validation.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_calibration_path_validation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCalibrationPathWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u8 FailCalibrationPath(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x04u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0x16u);
    return 0u;
}

/* RAM:fb10f1 */
u8 ConfigureControllerCalibrationPathAndValidatePort80C7(void)
{
    u8 sample;

    PATH_WRITE_E8E9(0u);
    if(PATH_RUN_CALIBRATION() == 0u)
        return FailCalibrationPath();

    PATH_CAPTURE_SAMPLES();
    PATH_STROBE(FW_CONTROLLER_UNKNOWN_018006, 0x80u);
    if((ReadCalibrationPathWord(FW_NEAR_UNKNOWN_FAAC) & 0x0008u) != 0u)
        return 0u;

    PATH_SET_CHANNEL(0u, 0x99u);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B9, 0u);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B7, 0u);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B5, 2u);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B3, 0x1cu);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B4, 0xf4u);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B2, 6u);
    PATH_WRITE_F6(0xd0u);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B0, 0x20u);
    PATH_WAIT_F8();
    PATH_WAIT_TIMER(100u, 0u);
    PATH_SET_FLAG(1u);
    PATH_WAIT_TIMER(20u, 0u);

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) < 0)
        PATH_WRITE_E8E9(0u);
    else
        PATH_WRITE_E8E9(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));

    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B5, 1u);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B0, 0x10u);
    PATH_WAIT_F8();
    PATH_RESET_CHANNEL(0u);
    PATH_WRITE_E8E9(0u);
    PATH_WRITE_F6(0xc0u);
    PATH_WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180C4, 0x20u);
    PATH_WAIT_F8();
    sample = PATH_READ_PORT(FW_CONTROLLER_UNKNOWN_0180C7);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9A6, sample);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB98) == 0u && sample < 0x50u)
    {
        if(sample <= 0x0du)
            return 0u;
        PATH_CONFIGURE_STATE2();
        if(PATH_RUN_CALIBRATION() == 0u)
            return FailCalibrationPath();
    }
    return 1u;
}
