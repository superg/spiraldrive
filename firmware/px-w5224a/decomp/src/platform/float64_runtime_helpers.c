#include "float64_runtime_helpers.h"
#include <math.h>

typedef union
{
    double value;
    u8 bytes[8];
} Float64RuntimeRepresentation;

static u8 ReadFloat64RuntimeByte(const Float64RuntimeRepresentation *value, u8 index)
{
#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/float64_runtime_helpers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteFloat64RuntimeByte(Float64RuntimeRepresentation *value, u8 index, u8 byte)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/float64_runtime_helpers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u16 ReadFloat64RuntimeHeader(const Float64RuntimeRepresentation *value)
{
    return (u16)(((u16)ReadFloat64RuntimeByte(value, 0u) << 8) | ReadFloat64RuntimeByte(value, 1u));
}

static void WriteFloat64RuntimeHeader(Float64RuntimeRepresentation *value, u16 header)
{
    WriteFloat64RuntimeByte(value, 0u, (u8)(header >> 8));
    WriteFloat64RuntimeByte(value, 1u, (u8)header);
}

static u8 IsFloat64RuntimeZero(const Float64RuntimeRepresentation *value)
{
    u8 index;
    u8 combined = 0u;

    for(index = 0u; index != 8u; ++index)
        combined |= ReadFloat64RuntimeByte(value, index);
    return (u8)(combined == 0u);
}

static double Float64RuntimeRawValue(u16 header, u8 payload)
{
    Float64RuntimeRepresentation result;
    u8 index;

    WriteFloat64RuntimeHeader(&result, header);
    for(index = 2u; index != 8u; ++index)
        WriteFloat64RuntimeByte(&result, index, payload);
    return result.value;
}

static u16 Float64RuntimeExponent(double value)
{
    Float64RuntimeRepresentation representation;

    representation.value = value;
    return (u16)((ReadFloat64RuntimeHeader(&representation) >> 4) & 0x07ffu);
}

static u16 Float64RuntimeSign(double value)
{
    Float64RuntimeRepresentation representation;

    representation.value = value;
    return (u16)(ReadFloat64RuntimeHeader(&representation) & 0x8000u);
}

static double Float64RuntimeNaN(void)
{
    return Float64RuntimeRawValue(0xffffu, 0xffu);
}

static double Float64RuntimeInfinity(u16 sign)
{
    return Float64RuntimeRawValue((u16)(sign | 0x7ff0u), 0u);
}

/* RAM:feee85 */
/* This source-level interface replaces the native compiler helper's mixed
 * stack/register ABI. Its raw exponent-field behavior is kept exactly,
 * including the wrapped header produced after exponent overflow. */
double ScaleFloat64ByPowerOfTwo(double value, s16 scale)
{
    Float64RuntimeRepresentation result;
    u16 header;
    u16 exponent;
    s16 adjusted_exponent;
    u8 index;

    result.value = value;
    if(IsFloat64RuntimeZero(&result) != 0u)
        return value;

    header = ReadFloat64RuntimeHeader(&result);
    exponent = (u16)((header >> 4) & 0x07ffu);
    adjusted_exponent = (s16)((u16)scale + exponent);
    if(adjusted_exponent < 1)
    {
        for(index = 0u; index != 8u; ++index)
            WriteFloat64RuntimeByte(&result, index, 0u);
        return result.value;
    }
    if(adjusted_exponent > 0x07ff)
    {
        header |= 0x7fffu;
        WriteFloat64RuntimeHeader(&result, header);
        for(index = 2u; index != 8u; ++index)
            WriteFloat64RuntimeByte(&result, index, 0xffu);
    }
    header = ReadFloat64RuntimeHeader(&result);
    header = (u16)(((u16)adjusted_exponent << 4) | (header & 0x800fu));
    WriteFloat64RuntimeHeader(&result, header);
    return result.value;
}

/* RAM:feef53 */
/* The helper normalizes only the stored exponent field; subnormal payloads
 * are deliberately not shifted. */
double DecomposeFloat64MantissaAndExponent(double value, s16 *exponent)
{
    Float64RuntimeRepresentation result;
    u16 header;
    u16 stored_exponent;

    result.value = value;
    if(IsFloat64RuntimeZero(&result) != 0u)
    {
        *exponent = 0;
        return value;
    }
    header = ReadFloat64RuntimeHeader(&result);
    stored_exponent = (u16)((header >> 4) & 0x07ffu);
    *exponent = (s16)stored_exponent - 0x03fe;
    header = (u16)((header & 0x800fu) | 0x3fe0u);
    WriteFloat64RuntimeHeader(&result, header);
    return result.value;
}

/* RAM:fd8fc5 */
/* The native runtime reports the strict ordering condition through CY. The
 * maintainable interface makes that condition an explicit Boolean result. */
u8 CompareFloat64(double left_value, double right_value)
{
    return (u8)(left_value < right_value);
}

/* RAM:fd915a */
/* The original treats exponent-zero payloads as already-normalized results,
 * maps every negative encoding (including negative zero) to its fixed NaN,
 * and maps every nonnegative exponent-0x7ff encoding to positive infinity. */
double SquareRootFloat64(double value)
{
    Float64RuntimeRepresentation representation;
    u16 header;
    u16 exponent;

    representation.value = value;
    header = ReadFloat64RuntimeHeader(&representation);
    if((header & 0x8000u) != 0u)
        return Float64RuntimeRawValue(0xffffu, 0xffu);
    exponent = (u16)((header >> 4) & 0x07ffu);
    if(exponent == 0u)
        return value;
    if(exponent == 0x07ffu)
        return Float64RuntimeRawValue(0x7ff0u, 0u);
    return sqrt(value);
}

/* RAM:fd8d05 */
double AddFloat64(double left_value, double right_value)
{
    u16 exponent;

    exponent = Float64RuntimeExponent(right_value);
    if(exponent == 0u)
        return left_value;
    if(exponent == 0x07ffu)
        return Float64RuntimeNaN();

    exponent = Float64RuntimeExponent(left_value);
    if(exponent == 0u)
        return right_value;
    if(exponent == 0x07ffu)
        return Float64RuntimeNaN();

    return left_value + right_value;
}

/* RAM:fd8d01 */
double SubtractFloat64(double left_value, double right_value)
{
    Float64RuntimeRepresentation negated_right;

    negated_right.value = right_value;
    WriteFloat64RuntimeHeader(&negated_right, (u16)(ReadFloat64RuntimeHeader(&negated_right) ^ 0x8000u));
    return AddFloat64(left_value, negated_right.value);
}

/* RAM:fd8e16 */
double MultiplyFloat64(double left_value, double right_value)
{
    u16 left_exponent = Float64RuntimeExponent(left_value);
    u16 right_exponent = Float64RuntimeExponent(right_value);
    u16 result_sign = (u16)(Float64RuntimeSign(left_value) ^ Float64RuntimeSign(right_value));
    double result;

    if(left_exponent == 0u || right_exponent == 0u)
    {
        if(left_exponent == 0x07ffu || right_exponent == 0x07ffu)
            return Float64RuntimeNaN();
        return 0.0;
    }
    if(left_exponent == 0x07ffu || right_exponent == 0x07ffu)
        return Float64RuntimeInfinity(result_sign);

    result = left_value * right_value;
    if(Float64RuntimeExponent(result) == 0u)
        return 0.0;
    return result;
}

/* RAM:fd8f26 */
double DivideFloat64(double left_value, double right_value)
{
    u16 left_exponent = Float64RuntimeExponent(left_value);
    u16 right_exponent = Float64RuntimeExponent(right_value);
    u16 result_sign = (u16)(Float64RuntimeSign(left_value) ^ Float64RuntimeSign(right_value));
    double result;

    if((left_exponent == 0u && right_exponent == 0u) || (left_exponent == 0x07ffu && right_exponent == 0x07ffu))
        return Float64RuntimeNaN();
    if(left_exponent == 0u || right_exponent == 0x07ffu)
        return 0.0;
    if(right_exponent == 0u || left_exponent == 0x07ffu)
        return Float64RuntimeInfinity(result_sign);

    result = left_value / right_value;
    if(Float64RuntimeExponent(result) == 0u)
        return 0.0;
    return result;
}
