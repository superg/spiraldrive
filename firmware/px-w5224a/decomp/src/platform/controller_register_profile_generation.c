#include "controller_register_profile_generation.h"
#include "controller_parameters.h"
#include "controller_profile_sample_generation.h"
#include "firmware_memory.h"
#include "floating_width_conversions.h"
#include "radial_model_profile_coordinate.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_register_profile_generation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define GENERATED_PROFILE_BASE FW_NEAR_UNKNOWN_E806
#define GENERATED_PROFILE_SIZE 34u
#define PROFILE_COMPUTED_VALUES FW_NEAR_UNKNOWN_EEAA
#define CONTROLLER_SAMPLE_D8 FW_NEAR_UNKNOWN_F9B7
#define CONTROLLER_SAMPLE_D9 FW_NEAR_UNKNOWN_F9CB

static void StoreGeneratedProfileWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u8 EncodeGeneratedProfileBaseline(s8 baseline)
{
    u8 encoded = (u8)baseline;

    if(baseline < 0)
        encoded = (u8)((u8)(0u - encoded) | 0x40u);
    return encoded;
}

static void StoreGeneratedProfile(u8 record_index, u8 source_index, float coordinate)
{
    fw_addr_t destination = GENERATED_PROFILE_BASE + (fw_addr_t)((u16)record_index * GENERATED_PROFILE_SIZE);
    u8 index;
    s8 baseline = GenerateAndNormalizeControllerProfileSamples(coordinate);

    FirmwareWrite8(destination, EncodeGeneratedProfileBaseline(baseline));
    ++destination;
    for(index = 0u; index < 18u; ++index)
        FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D8 + index));
    for(index = 0u; index < 9u; ++index)
        FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + index));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 13u));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 9u));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 10u));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 11u));
    FirmwareWrite8(destination++, FirmwareRead8(CONTROLLER_SAMPLE_D9 + 12u));
    FirmwareWrite8(destination, source_index);
}

/* RAM:f9d2d6 */
/* WR6=profile_input. */
void GenerateControllerRegisterProfiles(u16 profile_input)
{
    float previous_coordinate = 255.0f;
    u8 record_count = 0u;
    u8 source_index;

    InitializeGeneratedControllerProfileStorage();
    for(source_index = 0u; source_index < 100u; ++source_index)
    {
        float coordinate = ComputeRadialModelProfileCoordinate(source_index, profile_input);
        float floored_coordinate = (float)FLOOR_PROFILE_COORDINATE(coordinate);

        if(source_index == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA02, (u8)(s32)floored_coordinate);
        if(floored_coordinate != previous_coordinate && record_count < 50u)
        {
            previous_coordinate = floored_coordinate;
            StoreGeneratedProfile(record_count, source_index, floored_coordinate);
            ++record_count;
        }
        StoreGeneratedProfileWord(PROFILE_COMPUTED_VALUES + (fw_addr_t)((u16)source_index * 2u), ComputeControllerProfileScaledWord(coordinate));
    }
}
