#include "controller_operating_state_initialization.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_state_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadOperatingStateWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteOperatingStateU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fb0ce4 */
u8 InitializeControllerOperatingStateAndRunCalibration(void)
{
    u16 saved_word0204;

    STATE_INIT_RESET_STATE();
    STATE_INIT_RESET_HARDWARE();

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x01u) == 0x01u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB98) == 2u)
            STATE_INIT_CONFIGURE_PARAMETER_STATE2();
        if(STATE_INIT_IS_INDEXED(5u) == 1u)
        {
            STATE_INIT_UPDATE_OUTCOME(0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, 0x7fu);
            STATE_INIT_SET_P3_LOW_HIGH();
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 0u);
        STATE_INIT_SELECT_FB();
    }

    if((ReadOperatingStateWord(FW_NEAR_UNKNOWN_FAAC) & 0x8000u) != 0u)
        STATE_INIT_WAIT_CONFIGURATION_BIT7();

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x01u) == 0x01u)
    {
        if(STATE_INIT_ESTABLISH_STATE6() == 0u)
            return 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 1u)
        {
            STATE_INIT_APPLY_CHANNELS();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 2u);
        }
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 4u);
        STATE_INIT_LOAD_PROFILE(4u);
        STATE_INIT_WRITE_REGISTER01(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
        STATE_INIT_SET_P1_PINS();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1159) == 0u)
            STATE_INIT_PULSE_WORD(50u);
        else
            STATE_INIT_PULSE_WORD(125u);

        saved_word0204 = STATE_INIT_READ_WORD(0x0204u);
        STATE_INIT_SUBMIT_WORD(0u, 0x0204u);
        if(STATE_INIT_RETRY_PORT(1u) == 0u)
        {
            STATE_INIT_SUBMIT_WORD(saved_word0204, 0x0204u);
            return 0u;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1159) != 0u)
        {
            WriteOperatingStateU32(FW_NEAR_UNKNOWN_11FC, 250ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9A8, 0u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 6u);
        STATE_INIT_SET_CHANNEL(1u, 0x94u);
        STATE_INIT_SET_CHANNEL(2u, 0xc1u);
        STATE_INIT_CONFIGURE_RANGE();
        STATE_INIT_CONFIGURE_CODE4();
        STATE_INIT_APPLY_CHANNELS();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 50u);
        while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) >= 0)
        {
        }
        STATE_INIT_SUBMIT_WORD(saved_word0204, 0x0204u);
    }

    if(STATE_INIT_VALIDATE_STATE5() != 1u)
        return 0u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_123D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_123E, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_123F, 0u);
    WriteOperatingStateU32(FW_NEAR_UNKNOWN_0EF5, 0x96ul);

    if(STATE_INIT_RUN_CURRENT_CALIBRATION() != 1u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x01u) != 0u)
        STATE_INIT_RUN_COMMAND03();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x08u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xfeu));
    return 1u;
}
