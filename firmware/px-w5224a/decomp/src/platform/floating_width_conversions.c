#include "floating_width_conversions.h"

typedef union
{
    float value;
    u32 bits;
} Float32WidthRepresentation;

typedef union
{
    double value;
    u8 bytes[8];
} Float64WidthRepresentation;

static u8 ReadFloat64WidthByte(const Float64WidthRepresentation *value, u8 index)
{
#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/floating_width_conversions_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteFloat64WidthByte(Float64WidthRepresentation *value, u8 index, u8 byte)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/floating_width_conversions_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u16 Float64WidthHeader(const Float64WidthRepresentation *value)
{
    return (u16)(((u16)ReadFloat64WidthByte(value, 0u) << 8) | ReadFloat64WidthByte(value, 1u));
}

static double Float64WidthSpecial(u8 negative, u8 nan)
{
    Float64WidthRepresentation result;
    u8 index;

    for(index = 0u; index != 8u; ++index)
        WriteFloat64WidthByte(&result, index, nan != 0u ? 0xffu : 0u);
    if(nan == 0u)
    {
        WriteFloat64WidthByte(&result, 0u, negative != 0u ? 0xffu : 0x7fu);
        WriteFloat64WidthByte(&result, 1u, 0xf0u);
    }
    return result.value;
}

static float Float32WidthSpecial(u8 negative, u8 nan)
{
    Float32WidthRepresentation result;

    result.bits = nan != 0u ? 0xfffffffful : ((negative != 0u ? 0x80000000ul : 0ul) | 0x7f800000ul);
    return result.value;
}

/* RAM:fd90db */
double ConvertFloat32ToFloat64(float value)
{
    Float32WidthRepresentation source;
    u8 exponent;

    source.value = value;
    exponent = (u8)((source.bits >> 23) & 0xffu);
    if(exponent == 0u)
        return 0.0;
    if(exponent == 0xffu)
    {
        if((source.bits & 0x007ffffful) != 0u)
            return Float64WidthSpecial(0u, 1u);
        return Float64WidthSpecial((u8)(source.bits >> 31), 0u);
    }
    return (double)value;
}

/* RAM:fd911d */
float ConvertFloat64ToFloat32(double value)
{
    Float64WidthRepresentation source;
    Float32WidthRepresentation result;
    u16 header;
    u16 exponent;

    source.value = value;
    header = Float64WidthHeader(&source);
    exponent = (u16)((header >> 4) & 0x07ffu);
    if(exponent == 0u)
        return 0.0f;
    if(exponent == 0x07ffu)
        return Float32WidthSpecial(0u, 1u);

    result.value = (float)value;
    exponent = (u16)((result.bits >> 23) & 0xffu);
    if(exponent == 0u)
        return 0.0f;
    if(exponent == 0xffu)
        return Float32WidthSpecial((u8)(header >> 15), 0u);
    return result.value;
}

/* RAM:fd924b */
double NegateFloat64(double value)
{
    Float64WidthRepresentation result;
    u16 header;

    result.value = value;
    header = Float64WidthHeader(&result);
    if(header != 0u && header != 0xffffu)
        WriteFloat64WidthByte(&result, 0u, (u8)(ReadFloat64WidthByte(&result, 0u) ^ 0x80u));
    return result.value;
}

/* RAM:fd9079 */
s32 ConvertFloat64ToInteger32(double value)
{
    Float64WidthRepresentation source;
    u16 header;
    u16 exponent;
    u8 unbiased_exponent;
    u32 magnitude;

    source.value = value;
    header = Float64WidthHeader(&source);
    exponent = (u16)((header >> 4) & 0x07ffu);
    if(exponent < 0x03ffu)
        return 0;
    if((u16)(exponent - 0x03ffu) >= 32u)
        return (s32)-1;

    unbiased_exponent = (u8)(exponent - 0x03ffu);
    magnitude = 0x80000000ul | ((u32)(header & 0x000fu) << 27) | ((u32)ReadFloat64WidthByte(&source, 2u) << 19) | ((u32)ReadFloat64WidthByte(&source, 3u) << 11)
              | ((u32)ReadFloat64WidthByte(&source, 4u) << 3) | ((u32)ReadFloat64WidthByte(&source, 5u) >> 5);
    magnitude >>= (u8)(31u - unbiased_exponent);
    if((header & 0x8000u) != 0u)
        magnitude = (~magnitude) + 1u;
    return (s32)magnitude;
}

/* RAM:fd902a */
/* shared core through fd9078. */
double ConvertInteger32ToFloat64(s32 value)
{
    return (double)value;
}

/* RAM:fd902f */
/* shared core through fd9078. */
double ConvertInteger16ToFloat64(s16 value)
{
    return (double)value;
}

/* RAM:fd9034 */
double ConvertInteger8ToFloat64(s8 value)
{
    return (double)value;
}
