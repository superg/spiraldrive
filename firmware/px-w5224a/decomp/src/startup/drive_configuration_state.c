#include "drive_configuration_state.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "../platform/timing.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/drive_configuration_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteBigEndianU16Near(u16 address, u16 value)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/startup/drive_configuration_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u32 ReadBigEndianU32Near(u16 address)
{
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/startup/drive_configuration_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteBigEndianU32Near(u16 address, u32 value)
{
#define PLEXTOR_BINDING_SECTION 4
#include "../platform/bindings/startup/drive_configuration_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void PulseStartupP3Bit0(void)
{
    u8 delay_index;

    SET_P3_BIT0(1u);
    SET_P3_BIT1(0u);
    for(delay_index = 0u; delay_index < 0x24u; ++delay_index)
        DELAY_TIMER_UNITS(0x09a6u);
    SET_P3_BIT0(0u);
    SET_P3_BIT1(0u);
}

static void InitializeRawReadRingState(void)
{
    u32 ring_slot_count;

    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1298, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1294, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1296, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1292, 0u);
    ring_slot_count = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_156A);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    ring_slot_count = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_156A);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_126A, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1562));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_126E, 0ul);
}

/* RAM:fdc655 */
void InitializeDriveConfigurationState(void)
{
    u8 configuration_value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E45, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) | 0x40u));
    configuration_value = (u8)READ_CONFIGURATION_WORD(0x0fu);
    VALIDATE_STARTUP_INPUTS(configuration_value);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF1, 0u);

    if(configuration_value == 0u)
    {
        u8 startup_input_accepted = 0u;
        u8 selector_matches;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xbfu));
        selector_matches = 0u;
        if(READ_P0_BIT6() == 0u && READ_P1_BIT6() == 0u)
        {
            if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C040)) != 0u)
                selector_matches = (u8)(READ_P0_BIT1() != 0u && READ_P0_BIT2() == 0u);
            else
                selector_matches = (u8)(READ_P0_BIT1() == 0u && READ_P0_BIT2() != 0u);
        }
        if(selector_matches != 0u)
        {
            startup_input_accepted = 1u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 0xfau);
            while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) > 0)
            {
                if(READ_P1_BIT6() != 0u)
                {
                    startup_input_accepted = 0u;
                    break;
                }
            }
        }
        if(startup_input_accepted != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF1, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, 1u);
        }
    }
    else if(configuration_value == 0x11u || configuration_value == 0x12u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, 1u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, 0u);
        if(READ_P0_BIT1() == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) | 0x01u));
        if(READ_P0_BIT2() == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) | 0x02u));
        if(READ_P0_BIT6() == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) | 0x04u));
        if(READ_P1_BIT6() == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) | 0x08u));
            PulseStartupP3Bit0();
        }
        if(configuration_value == 0x12u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) | 0x10u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, 0x04u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1309, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, 0x00000096ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBE, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15F6, 0u);
    InitializeRawReadRingState();
}
