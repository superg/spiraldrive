#include "integer_float32_conversions.h"

/* RAM:fd94ce */
/* shared core through fd950c.
 * The explicit signed C interface replaces the runtime's separate ACC sign
 * indication while preserving the source-level conversion. */
float ConvertInteger32ToFloat32(s32 value)
{
    return (float)value;
}

/* RAM:fd94d3 */
/* shared core through fd950c. */
float ConvertInteger16ToFloat32(s16 value)
{
    return (float)value;
}

/* RAM:fd94d8 */
float ConvertInteger8ToFloat32(s8 value)
{
    return (float)value;
}

/* RAM:fd950d */
/* The runtime
 * truncates toward zero into a raw two's-complement dword and returns all
 * ones when the unbiased exponent is greater than 31. */
s32 ConvertFloat32ToInteger32(float value)
{
    union
    {
        float value;
        u32 bits;
    } source;
    u8 exponent;
    u8 shift;
    u32 magnitude;

    source.value = value;
    exponent = (u8)((source.bits >> 23) & 0xffu);
    if(exponent < 0x7fu)
        return 0;

    shift = (u8)(exponent - 0x7fu);
    if(shift > 31u)
        return (s32)-1;

    magnitude = (source.bits & 0x007ffffful) | 0x00800000ul;
    if(shift < 23u)
        magnitude >>= (u8)(23u - shift);
    else
        magnitude <<= (u8)(shift - 23u);

    if((source.bits & 0x80000000ul) != 0u)
        magnitude = (~magnitude) + 1u;
    return (s32)magnitude;
}
