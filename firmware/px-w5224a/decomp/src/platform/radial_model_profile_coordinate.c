#include "radial_model_profile_coordinate.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/radial_model_profile_coordinate_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadProfileCoordinateWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadProfileCoordinateDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:f9e2da */
/* ACC=profile_index, WR6=radial_model_endpoint, DR4=float32 result. */
float ComputeRadialModelProfileCoordinate(u8 profile_index, u16 radial_model_endpoint)
{
    const float pi = 3.1415927f;
    float scale = (float)ReadProfileCoordinateDword(FW_NEAR_UNKNOWN_0CF5);
    float radial;
    float root;
    s8 signed_index = (s8)profile_index;

    scale = scale / 65.536f;
    radial = scale * (float)ReadProfileCoordinateWord(FW_NEAR_UNKNOWN_0CF9);
    radial = radial / 1000000.0f;
    if(signed_index >= 96)
        signed_index = (s8)(signed_index - 100);
    radial = radial * (float)signed_index;
    radial = radial * 60.0f;
    radial = radial / pi;
    radial = radial + 625.0f;
    root = (float)PROFILE_SQRT(radial);
    root = root * 2.0f;
    root = root * pi;
    root = root * (float)radial_model_endpoint;
    return root / (scale * 60.0f);
}
