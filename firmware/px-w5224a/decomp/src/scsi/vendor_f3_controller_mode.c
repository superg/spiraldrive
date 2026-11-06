#include "vendor_f3_controller_mode.h"
#include "../media/strategy_profile_initialization.h"
#include "../platform/arithmetic.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_f3_controller_mode_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadVendorF3ModeWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteVendorF3ModeDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fbe343 */
void ExecuteVendorF3Selector0DControllerMode(u8 controller_mode)
{
    u8 controller_flags;

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) < 0)
    {
        VENDOR_F3_SEQUENCE_ERROR();
        return;
    }
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) >> 4) & 3u) == 2u)
    {
        VENDOR_F3_SEQUENCE_ERROR();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1BA, controller_mode);
    VENDOR_F3_WRITE_D6();
    if(controller_mode != 0u)
        VENDOR_F3_CONFIGURE_STATE2();
    VENDOR_F3_RESET_DATA_PATH();
    VENDOR_F3_SET_CONTROLLER_FLAG(1u);
    VENDOR_F3_WAIT_TIMER(0x14u, 0u);
    VENDOR_F3_WRITE_E8E9(0u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 2u);
    VENDOR_F3_WRITE_REGISTER01(2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F05F, 0u);

    if(controller_mode != 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1008, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_113B, 0x4au);
        WriteVendorF3ModeDword(FW_NEAR_CURRENT_FRAME_POSITION, 0x000514c8ul);
    }

    VENDOR_F3_INITIALIZE_STRATEGY();
    VENDOR_F3_CONFIGURE_PROFILE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x40u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
    VENDOR_F3_WRITE_EF00(0u);
    VENDOR_F3_WRITE_EF00(0u);

    VENDOR_F3_SET_GLOBAL_INTERRUPT(0u);
    controller_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_flags);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_flags);

    if(controller_mode == 1u || controller_mode == 3u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0x40u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0x50u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BD, 1u);
    }
    else if(controller_mode == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0x40u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0x50u);
    }
    else if(controller_mode == 2u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0x30u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0x10u);
    }

    controller_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_flags);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_flags);
    VENDOR_F3_SET_GLOBAL_INTERRUPT(1u);
    VENDOR_F3_SET_CONTROLLER_INTERRUPT(1u);
    VENDOR_F3_SET_A9_BIT2(1u);

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x13u);

    if(controller_mode == 1u && ReadVendorF3ModeWord(FW_NEAR_UNKNOWN_F087) != 0xffffu)
        VENDOR_F3_APPLY_RETAINED_WORD(ReadVendorF3ModeWord(FW_NEAR_UNKNOWN_F087));

    VENDOR_F3_SET_P3_RX(1u);
    VENDOR_F3_SET_P3_TX(0u);
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 8u) != 0u && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
        VENDOR_F3_SET_P3_RX(1u);
        VENDOR_F3_SET_P3_TX(0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x10u));
        VENDOR_F3_SET_P3_RX(0u);
        VENDOR_F3_SET_P3_TX(1u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0xcfu) | 0x10u));
    VENDOR_F3_NO_SENSE();
}
