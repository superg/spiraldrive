#include "controller_register_profile_application.h"
#include "arithmetic.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_register_profile_application_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define GENERATED_PROFILE_BASE FW_NEAR_UNKNOWN_E806
#define GENERATED_PROFILE_SOURCE_BASE FW_NEAR_UNKNOWN_E827
#define GENERATED_PROFILE_RECORD_SIZE 34u
#define GENERATED_PROFILE_COUNT 50u

static u16 ReadProfileWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteProfileWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static fw_addr_t GeneratedProfileAddress(u8 index)
{
    return GENERATED_PROFILE_BASE + (fw_addr_t)((u16)index * GENERATED_PROFILE_RECORD_SIZE);
}

static fw_addr_t GeneratedProfileSourceAddress(u8 index)
{
    return GENERATED_PROFILE_SOURCE_BASE + (fw_addr_t)((u16)index * GENERATED_PROFILE_RECORD_SIZE);
}

static u8 FindGeneratedProfile(u8 coordinate)
{
    u8 index;
    u8 source_coordinate;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA01) != 0xffu)
    {
        index = FirmwareReadNear8(FW_NEAR_UNKNOWN_FA01);
        return (u8)(FirmwareRead8(GeneratedProfileSourceAddress(index)) == coordinate);
    }

    for(index = 0u; index < GENERATED_PROFILE_COUNT; ++index)
    {
        source_coordinate = FirmwareRead8(GeneratedProfileSourceAddress(index));
        if(coordinate == source_coordinate)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA01, index);
            return 1u;
        }
        if(coordinate < source_coordinate)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA01, (u8)(index - 1u));
            return 1u;
        }
    }
    return 0u;
}

static void ApplyGeneratedProfile(void)
{
    fw_addr_t source = GeneratedProfileAddress(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA01));
    u8 column;
    u8 shadow;
    u8 next_index;

    CLEAR_PROFILE_EA();
    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018081, FirmwareRead8(source));
    for(column = 0u; column != 9u; ++column)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018082 + column, FirmwareRead8(source + 1ul + column));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01808B + column, FirmwareRead8(source + 10ul + column));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018094 + column, FirmwareRead8(source + 19ul + column));
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180AA, FirmwareRead8(source + 28ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A9, FirmwareRead8(source + 29ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A6, FirmwareRead8(source + 30ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A7, FirmwareRead8(source + 31ul));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180A8, FirmwareRead8(source + 32ul));

    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
    SET_PROFILE_EA();
    SET_PROFILE_EC();
    SET_PROFILE_A9_BIT2();

    next_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA01) + 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA01, next_index);
    if(FirmwareRead8(GeneratedProfileSourceAddress(next_index)) == 0xffu)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA01, 0u);
}

static void ApplyProfileAdjustment(u16 computed_value)
{
    s32 product = (s32)MultiplyU32Low((u32)computed_value, (u32)ReadProfileWord(FW_NEAR_UNKNOWN_F9E2));
    s32 quotient = product / 10000L;
    u16 adjusted = (u16)((u16)quotient + ReadProfileWord(FW_NEAR_UNKNOWN_FAD1));

    WriteProfileWord(FW_NEAR_UNKNOWN_FACF, adjusted);
    WriteProfileWord(FW_NEAR_UNKNOWN_FBC3, adjusted);
}

static u16 FixedProfileComputedValueAddress(u8 operating_code)
{
    switch(operating_code)
    {
    case 0x14u:
        return 0xe752u;
    case 0x18u:
        return 0xe754u;
    case 0x20u:
        return 0xe756u;
    case 0x28u:
        return 0xe758u;
    case 0x34u:
        return 0xe75au;
    default:
        return 0u;
    }
}

/* RAM:f9ee39 */
/* ACC=coordinate, R7=operating_code. */
void ApplyControllerRegisterProfile(u8 coordinate, u8 operating_code)
{
    u8 found = 0u;
    u16 fixed_value_address;

    if(operating_code == 0xffu)
    {
        if((s8)coordinate < 0)
            coordinate = (u8)(coordinate + 100u);

        found = FindGeneratedProfile(coordinate);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
        {
            if(found != 0u)
                ApplyGeneratedProfile();

            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u) && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) & 2u) == 0u))
                ApplyProfileAdjustment(ReadProfileWord(FW_NEAR_UNKNOWN_EEAA + (fw_addr_t)((u16)coordinate * 2u)));
        }

        if((found != 0u) && (coordinate < 0x61u))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA01) + FirmwareReadNear8(FW_NEAR_UNKNOWN_FA02) - 1u));
        if(coordinate > 0x60u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, 0x12u);
    }
    else
    {
        fixed_value_address = FixedProfileComputedValueAddress(operating_code);
        if(fixed_value_address != 0u)
        {
            APPLY_FIXED_PROFILE(operating_code);
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u) && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) & 2u) == 0u))
                ApplyProfileAdjustment(ReadProfileWord((fw_addr_t)fixed_value_address));
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, operating_code);
    }

    UPDATE_PROFILE_DERIVED_VALUE();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
        WRITE_PROFILE_E8_VALUE((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA04) + FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5)));
}
