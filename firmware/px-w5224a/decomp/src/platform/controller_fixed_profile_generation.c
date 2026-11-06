#include "controller_fixed_profile_generation.h"
#include "controller_parameters.h"
#include "controller_profile_sample_generation.h"
#include "controller_register_profile_generation.h"
#include "firmware_memory.h"

#define FIXED_PROFILE_BASE FW_NEAR_UNKNOWN_E75C
#define FIXED_PROFILE_SIZE 34u
#define FIXED_PROFILE_COMPUTED_VALUES FW_NEAR_UNKNOWN_E752
#define CONTROLLER_SAMPLE_D8 FW_NEAR_UNKNOWN_F9B7
#define CONTROLLER_SAMPLE_D9 FW_NEAR_UNKNOWN_F9CB

static float FixedProfileValue(u8 profile_index)
{
    switch(profile_index)
    {
    case 0u:
        return 20.0f;
    case 1u:
        return 24.0f;
    case 2u:
        return 32.0f;
    case 3u:
        return 40.0f;
    default:
        return 52.0f;
    }
}

static u8 EncodeFixedProfileBaseline(s8 baseline)
{
    u8 encoded = (u8)baseline;

    if(baseline < 0)
        encoded = (u8)((u8)(0u - encoded) | 0x40u);
    return encoded;
}

static void StoreFixedProfileWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void BuildFixedProfile(u8 profile_index, float profile_value)
{
    fw_addr_t destination = FIXED_PROFILE_BASE + (fw_addr_t)((u16)profile_index * FIXED_PROFILE_SIZE);
    u8 index;
    s8 baseline = GenerateAndNormalizeControllerProfileSamples(profile_value);

    FirmwareWrite8(destination++, EncodeFixedProfileBaseline(baseline));
    for(index = 0u; index < 18u; ++index)
        FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D8 + index));
    for(index = 0u; index < 9u; ++index)
        FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + index));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 13u));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 9u));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 10u));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 11u));
    FirmwareWrite8(destination, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 12u));

    StoreFixedProfileWord(FIXED_PROFILE_COMPUTED_VALUES + (fw_addr_t)((u16)profile_index * 2u), ComputeControllerProfileScaledWord(profile_value));
}

/* RAM:f9cd5f */
/* WR6=profile_input. */
void BuildFixedAndGeneratedControllerRegisterProfiles(u16 profile_input)
{
    u8 profile_index;

    InitializeFixedControllerProfileStorage();
    GenerateControllerRegisterProfiles(profile_input);
    for(profile_index = 0u; profile_index < 5u; ++profile_index)
    {
        float profile_value = FixedProfileValue(profile_index);

        BuildFixedProfile(profile_index, profile_value);
    }
}
