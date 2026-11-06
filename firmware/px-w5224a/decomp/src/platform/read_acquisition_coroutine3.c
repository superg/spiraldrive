#include "read_acquisition_coroutine3.h"
#include "firmware_memory.h"
#include "read_acquisition_controller_config.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/read_acquisition_coroutine3_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/read_acquisition_coroutine3_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadReadAcquisitionCoroutine3U16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadReadAcquisitionCoroutine3U32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteReadAcquisitionCoroutine3U32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fd9659 */
void RunReadAcquisitionCoroutine3(void)
{
    u8 mode;
    u8 controller_shadow;
    u32 capacity_adjustment;
    u32 nominal_lead_out;

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
        COROUTINE3_SET_BIT29();
    COROUTINE3_CONFIGURE_CONTROLLER();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9A, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DC, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));

    if(((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >> 3) & 1u) == 1u && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
        COROUTINE3_SET_P3_RX_HIGH_TX_LOW();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x10u));
        COROUTINE3_SET_P3_RX_LOW_TX_HIGH();
    }

    COROUTINE3_RESET_MEASUREMENT();
    COROUTINE3_LOAD_BUFFER_SETUP();
    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B);
    switch(mode)
    {
    case 0u:
        COROUTINE3_PREPARE_MODE0();
        break;
    case 1u:
        COROUTINE3_PREPARE_MODE1();
        break;
    case 2u:
        COROUTINE3_PREPARE_MODE2();
        break;
    case 3u:
        COROUTINE3_PREPARE_MODE3();
        break;
    default:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 5u);
        break;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u)
    {
        COROUTINE3_DISABLE_GLOBAL_INTERRUPT();
        controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018080, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C0, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) == 2u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180CF, 4u);
        else
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180CF, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C1, 0x14u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C2, 0xe1u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C3, 0x60u);
        controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
        COROUTINE3_ENABLE_GLOBAL_INTERRUPT();
        COROUTINE3_ENABLE_CONTROLLER_INTERRUPT();
        COROUTINE3_SET_SFR_A9_BIT2();
        COROUTINE3_CLEAR_BIT30();
        COROUTINE3_CLEAR_BIT33();
        COROUTINE3_CLEAR_BIT32();
        COROUTINE3_CLEAR_BIT35();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD6, 0u);
    }

    if(COROUTINE3_READ_BIT30() != 1u)
        COROUTINE3_CLEAR_BIT31();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
    COROUTINE3_SET_P3_RX_HIGH_TX_LOW();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
        COROUTINE3_WRITE_E8E9(FirmwareReadNear8(FW_NEAR_UNKNOWN_14DC));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9A, 0u);

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_14C0) == 1u
        && ReadReadAcquisitionCoroutine3U16(FW_NEAR_UNKNOWN_FBB4) != 0u)
        COROUTINE3_RESTORE_DESCRIPTORS(1u);

    if(COROUTINE3_READ_BIT32() != 1u)
        COROUTINE3_INITIALIZE_REGISTERS();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) == 0u)
            WriteReadAcquisitionCoroutine3U32(FW_NEAR_UNKNOWN_0E3B, 0x00000096ul);
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) == 2u)
            COROUTINE3_MARK_RECORD_RANGE();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 1u));
        }
        else
        {
            COROUTINE3_PREPARE_RECORD_RANGE();
        }
    }

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) >> 1) & 1u) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) == 0xc0u)
    {
        capacity_adjustment = ReadReadAcquisitionCoroutine3U32(FW_NEAR_UNKNOWN_FB90);
        nominal_lead_out = ReadReadAcquisitionCoroutine3U32(FW_NEAR_UNKNOWN_FB8C);
        WriteReadAcquisitionCoroutine3U32(FW_NEAR_CURRENT_FRAME_POSITION, nominal_lead_out + capacity_adjustment - 0x000008caul);
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && (s32)ReadReadAcquisitionCoroutine3U32(FW_NEAR_UNKNOWN_14DE) >= 0x000685ecl)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102E, 1u);

    COROUTINE3_CLEAR_BIT29();
    for(;;)
        COROUTINE3_YIELD();
}
