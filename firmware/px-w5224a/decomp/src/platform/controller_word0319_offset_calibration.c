#include "controller_word0319_offset_calibration.h"
#include "arithmetic.h"
#include "controller_calibration.h"
#include "controller_calibration_outcome.h"
#include "controller_mapping.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_word0319_offset_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCalibrationWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteCalibrationWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void RestoreControllerProfile(u16 word_0339, u8 channel_2_value)
{
    SUBMIT_WORD(word_0339, 0x0339u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        CLEAR_D1_BIT6();
    WRITE_D2_D3();
    SUBMIT_WORD(0x0420u, 0x0133u);
    SUBMIT_WORD(0x0420u, 0x0134u);
    WRITE_PORT_80E4();
    SUBMIT_WORD(0x0160u, 0x0133u);
    SUBMIT_WORD(0x0160u, 0x0134u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        APPLY_D1();
    WRITE_D1_D2_D3();
    SET_CHANNEL(2u, channel_2_value);
    SET_CHANNEL(3u, MAP_CHANNEL3(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43)));
}

/* RAM:f8d06d */
u8 CalibrateControllerWord0319Offset(void)
{
    u16 baseline_word;
    u16 current_word;
    u16 saved_word_0339;
    u16 correction_word;
    s16 signed_correction;
    u8 initial_pulse_result = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        initial_pulse_result = PULSE_TEST(10u);

    SET_CHANNEL(3u, 0xaau);
    WRITE_PORT(FW_CONTROLLER_UNKNOWN_0180B0, 0u);
    baseline_word = READ_CONTROLLER_WORD(0x0319u);
    WriteCalibrationWord(FW_NEAR_UNKNOWN_F99A, baseline_word);
    current_word = baseline_word;
    saved_word_0339 = READ_CONTROLLER_WORD(0x0339u);
    SUBMIT_WORD(0u, 0x0339u);
    UPDATE_CONFIGURATION();

    if(LOAD_PARAMETER_FC() != 1u)
    {
        RestoreControllerProfile(saved_word_0339, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120A, 10u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) > 0)
    {
        RESET_CHANNEL(2u);
        ENTER_MEASUREMENT();
        signed_correction = (s16)(s8)RUN_COMMAND_17();
        WriteCalibrationWord(FW_NEAR_UNKNOWN_F992, (u16)signed_correction);

        if(signed_correction == 0x007f)
            break;
        if(signed_correction != 0 && signed_correction != 1 && signed_correction != -1)
        {
            signed_correction = DIVIDE_SIGNED(signed_correction, 2u);
            WriteCalibrationWord(FW_NEAR_UNKNOWN_F992, (u16)signed_correction);
        }

        correction_word = (u16)((u16)signed_correction * 0x0100u);
        WriteCalibrationWord(FW_NEAR_UNKNOWN_F992, correction_word);
        current_word = (u16)(current_word + correction_word);
        SUBMIT_WORD(current_word, 0x0319u);
        if(correction_word != 0u)
            continue;

        correction_word = (u16)(current_word - baseline_word);
        WriteCalibrationWord(FW_NEAR_UNKNOWN_F992, correction_word);
        if(IS_INDEXED_STATE(5u) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0xfdu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0xefu));
            WriteCalibrationWord(FW_NEAR_UNKNOWN_FABA, correction_word);
            UPDATE_OUTCOME(2u);
        }
        break;
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120A) < 0)
        WriteCalibrationWord(FW_NEAR_UNKNOWN_F992, 0x007fu);
    if(ReadCalibrationWord(FW_NEAR_UNKNOWN_F992) == 0x007fu)
        SUBMIT_WORD(baseline_word, 0x0319u);

    LEAVE_MEASUREMENT();
    WAIT_DURATION(4000u, 0u);
    RestoreControllerProfile(0u, 0xc1u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && PULSE_TEST(10u) == initial_pulse_result && initial_pulse_result == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) + 0x20u));
        SELECT_PARAMETER_FC(4u);
    }

    return ReadCalibrationWord(FW_NEAR_UNKNOWN_F992) == 0x007fu ? 0u : 1u;
}
