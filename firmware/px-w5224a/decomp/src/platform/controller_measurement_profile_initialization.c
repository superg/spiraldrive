#include "controller_measurement_profile_initialization.h"
#include "firmware_memory.h"

static u16 ReadProfileInitializationWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadProfileInitializationDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteProfileInitializationWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteProfileInitializationDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static fw_addr_t CurrentMeasurementProfileAddress(void)
{
    return ReadProfileInitializationDword(FW_NEAR_UNKNOWN_F1BB);
}

static void SelectControllerMeasurementProfile(u8 profile_index)
{
    u8 speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
    fw_addr_t profile_base = 0ul;

    if(speed >= 0x10u)
        speed = 0x10u;

    switch(speed)
    {
    case 2u:
    case 4u:
        profile_base = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u ? PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83084) : PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F831AD);
        break;
    case 8u:
    case 0x10u:
        profile_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F833BD);
        break;
    case 10u:
        profile_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F832B5);
        break;
    default:
        break;
    }

    if(profile_base != 0ul)
        WriteProfileInitializationDword(FW_NEAR_UNKNOWN_F1BB, profile_base + (fw_addr_t)((u16)profile_index * 11u));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 8u) != 0u)
        WriteProfileInitializationDword(FW_NEAR_UNKNOWN_F1BB,
            FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u ? PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8342B) : PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83439));
}

/* RAM:f95381 */
void PLEXTOR_NEAR_FUNCTION InitializeControllerMeasurementSeries00And01FromProfile(void)
{
    u8 series00_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A);
    u8 profile_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F);
    u8 mode;
    u8 index;

    SelectControllerMeasurementProfile(profile_index);
    WriteProfileInitializationWord(FW_NEAR_UNKNOWN_F089, ReadProfileInitializationWord(CurrentMeasurementProfileAddress() + 4u));

    mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u);
    if(mode == 4u)
    {
        for(index = 0u; index < 15u; ++index)
        {
            fw_addr_t profile = CurrentMeasurementProfileAddress();
            u16 series00 = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_EF77) + (u16)FirmwareRead8(profile) + (u16)((u16)index * (u16)FirmwareRead8(profile + 3u)));

            WriteProfileInitializationWord(FW_NEAR_UNKNOWN_EF89 + (fw_addr_t)((u16)index * 2u), series00);
            profile = CurrentMeasurementProfileAddress();
            WriteProfileInitializationWord(FW_NEAR_UNKNOWN_EFA7 + (fw_addr_t)((u16)index * 2u),
                (u16)(ReadProfileInitializationWord(profile + 1u) + ReadProfileInitializationWord(FW_NEAR_UNKNOWN_F074)));
        }
    }
    else
    {
        for(index = 0u; index < 15u; ++index)
        {
            fw_addr_t profile = CurrentMeasurementProfileAddress();
            u16 scaled_index = (u16)((u16)index * (u16)FirmwareRead8(profile + 3u));
            s16 series00 = (s16)((u16)FirmwareRead8(profile) + (u16)series00_offset + scaled_index + (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_EF77));
            s16 series01;

            if(series00 < 0)
                series00 = 0;
            else if(series00 > 126)
                series00 = 126;
            WriteProfileInitializationWord(FW_NEAR_UNKNOWN_EF89 + (fw_addr_t)((u16)index * 2u), (u16)series00);

            profile = CurrentMeasurementProfileAddress();
            series01 = (s16)(ReadProfileInitializationWord(profile + 1u) + (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_EF78) + ReadProfileInitializationWord(FW_NEAR_UNKNOWN_F074));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 1u)
            {
                s32 adjustment = (s32)((double)(s8)FirmwareRead8(profile + 7u) * 0.1 * (double)index);

                series01 = (s16)((u16)series01 + (u16)adjustment);
            }
            if(series01 < 0)
                series01 = 0;
            else if(series01 > 24000)
                series01 = 24000;
            WriteProfileInitializationWord(FW_NEAR_UNKNOWN_EFA7 + (fw_addr_t)((u16)index * 2u), (u16)series01);
        }
    }

    mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u);
    if(mode == 2u || mode == 3u || mode == 6u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) | 0x40u));
    else if(mode == 1u || mode == 4u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xbfu));
}
