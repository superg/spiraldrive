#include "float32_runtime_arithmetic.h"

typedef union
{
    float value;
    u32 bits;
} Float32RuntimeRepresentation;

static u8 Float32RuntimeExponent(float value)
{
    Float32RuntimeRepresentation representation;

    representation.value = value;
    return (u8)((representation.bits >> 23) & 0xffu);
}

static float Float32RuntimeNaN(void)
{
    Float32RuntimeRepresentation representation;

    representation.bits = 0xfffffffful;
    return representation.value;
}

static float Float32RuntimeInfinity(u32 sign)
{
    Float32RuntimeRepresentation representation;

    representation.bits = sign | 0x7f800000ul;
    return representation.value;
}

static u32 Float32RuntimeSign(float value)
{
    Float32RuntimeRepresentation representation;

    representation.value = value;
    return representation.bits & 0x80000000ul;
}

/* RAM:fd92af */
/* The compiler runtime classifies every exponent-zero encoding as zero and
 * every exponent-255 encoding through its fixed all-ones NaN tail. */
float AddFloat32(float left_value, float right_value)
{
    u8 exponent;

    exponent = Float32RuntimeExponent(right_value);
    if(exponent == 0u)
        return left_value;
    if(exponent == 0xffu)
        return Float32RuntimeNaN();

    exponent = Float32RuntimeExponent(left_value);
    if(exponent == 0u)
        return right_value;
    if(exponent == 0xffu)
        return Float32RuntimeNaN();

    return left_value + right_value;
}

/* RAM:fd92ac */
/* The native entry flips
 * the right operand's sign bit and falls through to AddFloat32. */
float SubtractFloat32(float left_value, float right_value)
{
    Float32RuntimeRepresentation negated_right;

    negated_right.value = right_value;
    negated_right.bits ^= 0x80000000ul;
    return AddFloat32(left_value, negated_right.value);
}

/* RAM:fd9367 */
/* Exponent-zero
 * operands and underflow flush to positive zero. Exponent-255 operands use
 * infinity semantics; combining that class with zero returns the fixed NaN. */
float MultiplyFloat32(float left_value, float right_value)
{
    u8 left_exponent = Float32RuntimeExponent(left_value);
    u8 right_exponent = Float32RuntimeExponent(right_value);
    u32 result_sign = Float32RuntimeSign(left_value) ^ Float32RuntimeSign(right_value);
    float result;

    if(left_exponent == 0u || right_exponent == 0u)
    {
        if(left_exponent == 0xffu || right_exponent == 0xffu)
            return Float32RuntimeNaN();
        return 0.0f;
    }
    if(left_exponent == 0xffu || right_exponent == 0xffu)
        return Float32RuntimeInfinity(result_sign);

    result = left_value * right_value;
    if(Float32RuntimeExponent(result) == 0u)
        return 0.0f;
    return result;
}

/* RAM:fd9407 */
/* The runtime
 * treats equal zero/infinity classes as invalid, flushes quotient underflow,
 * and uses signed infinity for division by an exponent-zero operand. */
float DivideFloat32(float left_value, float right_value)
{
    u8 left_exponent = Float32RuntimeExponent(left_value);
    u8 right_exponent = Float32RuntimeExponent(right_value);
    u32 result_sign = Float32RuntimeSign(left_value) ^ Float32RuntimeSign(right_value);
    float result;

    if((left_exponent == 0u && right_exponent == 0u) || (left_exponent == 0xffu && right_exponent == 0xffu))
        return Float32RuntimeNaN();
    if(left_exponent == 0u || right_exponent == 0xffu)
        return 0.0f;
    if(right_exponent == 0u || left_exponent == 0xffu)
        return Float32RuntimeInfinity(result_sign);

    result = left_value / right_value;
    if(Float32RuntimeExponent(result) == 0u)
        return 0.0f;
    return result;
}

/* RAM:fd9488 */
/* The native compiler helper reports strict ordering through CY. All
 * canonical callers are reconstructed C, so the source interface exposes
 * that condition as a normalized byte result. */
u8 CompareFloat32(float left_value, float right_value)
{
    return (u8)(left_value < right_value);
}
