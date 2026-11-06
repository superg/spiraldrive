#include "controller_profile_sample_generation.h"
#include "controller_profile_sample_interpolation.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_profile_sample_generation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f9e43e */
/* DR4=profile_value, ACC=signed normalization baseline. */
s8 PLEXTOR_NEAR_FUNCTION GenerateAndNormalizeControllerProfileSamples(float profile_value)
{
    s8 baseline = 0x7f;
    s8 difference;
    s8 value;
    u8 index;

    for(index = 0u; index != 18u; ++index)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9B7 + index, (u8)INTERPOLATE_PROFILE_SAMPLE(profile_value, index));
        FirmwareWrite8(FW_HUGE_UNKNOWN_00F9CB + index, (u8)INTERPOLATE_PROFILE_SAMPLE(profile_value, (u8)(index + 18u)));
    }

    for(index = 0u; index != 9u; ++index)
    {
        value = (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9B7 + index);
        if(value < baseline)
            baseline = value;
    }

    for(index = 0u; index != 9u; ++index)
    {
        difference = (s8)(u8)((u8)baseline - FirmwareReadNear8(FW_NEAR_UNKNOWN_F9D4 + index));
        if(difference > 15)
            baseline = -15;
    }
    if(baseline < -15)
        baseline = -15;

    for(index = 0u; index != 9u; ++index)
    {
        value = (s8)(u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9B7 + index) - (u8)baseline);
        if(value < 0)
            value = 0;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9B7 + index, (u8)value);
    }

    for(index = 0u; index != 9u; ++index)
    {
        u8 packed = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9CB + index) << 4);
        difference = (s8)(u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9D4 + index) - (u8)baseline);
        if(difference < -15)
            difference = -15;
        packed = (u8)((packed & 0xf0u) | ((u8)(0u - (u8)difference) & 0x0fu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9CB + index, packed);
    }

    for(index = 0u; index != 9u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9D4 + index, 0u);

    for(index = 0u; index != 5u; ++index)
        FirmwareWrite8(FW_HUGE_UNKNOWN_00F9D4 + index, (u8)LOOKUP_PROFILE_AUXILIARY(profile_value, index));

    return baseline;
}
