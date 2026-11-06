#include "strategy_profile_initialization.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/firmware_rom_data.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/strategy_profile_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadStrategyU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteStrategyU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadStrategyU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static fw_addr_t StrategyRecordAddress(void)
{
    return PLEXTOR_FIXED_ROM_ADDRESS(PLEXTOR_FIXED_ROM_DATA_CANONICAL_BASE + STRATEGY_MULTIPLY(15ul, (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D)));
}

/* RAM:f8cb86 */
void InitializeMediaStrategyProfileAndWriteSpeedSelection(void)
{
    u8 write_speed_x;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        fw_addr_t record;
        u8 index;
        u16 strategy_flags;
        u16 fallback_word;

        if((s32)ReadStrategyU32(FW_NEAR_CURRENT_FRAME_POSITION) < 0x0004ce78l)
        {
            strategy_flags = ReadStrategyU16(FW_NEAR_UNKNOWN_FAD7);
            WriteStrategyU16(FW_NEAR_UNKNOWN_FAD7, (u16)(strategy_flags | 0x0010u));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 1u && (s32)ReadStrategyU32(FW_NEAR_CURRENT_FRAME_POSITION) <= 0x00015f90l)
            {
                strategy_flags = ReadStrategyU16(FW_NEAR_UNKNOWN_FAD7);
                WriteStrategyU16(FW_NEAR_UNKNOWN_FAD7, (u16)(strategy_flags & 0xffefu));
            }
        }

        record = StrategyRecordAddress();
        for(index = 0u; index < 4u; ++index)
            WriteStrategyU16(FW_NEAR_UNKNOWN_FB61 + (u16)index * 2u, (u16)((u16)FirmwareRead8(record + index) * 10u));
        for(index = 0u; index < 4u; ++index)
        {
            s8 signed_value = (s8)FirmwareRead8(record + 4u + index);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F963 + index, (u8)(s32)signed_value);
        }
        for(index = 0u; index < 4u; ++index)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F968 + index, FirmwareRead8(record + 8u + index));

        record = StrategyRecordAddress();
        write_speed_x = FirmwareRead8(record + 12u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x08u) == 0u)
            write_speed_x = 0x34u;
        if((ReadStrategyU16(FW_NEAR_UNKNOWN_FAD7) & 0x0002u) == 0u)
            write_speed_x = 0x34u;

        record = StrategyRecordAddress();
        fallback_word = ReadStrategyU16(record + 13u);
        if(fallback_word == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5E) == 0u)
                fallback_word = 3000u;
            else
                fallback_word = 0x0af0u;
        }
        WriteStrategyU16(FW_NEAR_UNKNOWN_F9FA, fallback_word);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) == 10u && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 1u)
            write_speed_x = 0x20u;
    }
    else
    {
        u8 index;

        for(index = 0u; index < 4u; ++index)
            WriteStrategyU16(FW_NEAR_UNKNOWN_FB61 + (u16)index * 2u, 0x0028u);
        for(index = 0u; index < 4u; ++index)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F963 + index, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F968 + index, 0u);
        }
        WriteStrategyU16(FW_NEAR_UNKNOWN_FAD1, 0u);
        WriteStrategyU16(FW_NEAR_UNKNOWN_F9FA, 0u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            write_speed_x = 24u;
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
            write_speed_x = 10u;
        else
            write_speed_x = 4u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1159) == 0u)
        STRATEGY_APPLY_SPEED(write_speed_x);
}
