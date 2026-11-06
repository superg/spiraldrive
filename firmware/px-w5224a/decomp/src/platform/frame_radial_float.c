#include "frame_radial_float.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/frame_radial_float_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u16 ReadBigEndianU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* RAM:fce56c */
float PLEXTOR_NEAR_FUNCTION MapFramePositionToRadialModelFloat(s32 frame_position)
{
    const float pi = 3.1415927f;
    float scale = (float)(s32)ReadBigEndianU32(FW_NEAR_UNKNOWN_0CF5) / 65.536f;
    float position = (float)frame_position / 750.0f;
    float radicand;
    float root;

    position = position * scale;
    position = position * (float)ReadBigEndianU16(FW_NEAR_UNKNOWN_0CF9);
    position = position / 1000.0f;
    position = position / pi;
    radicand = position + 62500.0f;
    root = RADIAL_SQRT(radicand);

    return (scale * 60.0f) / (root * 0.2f * pi);
}
