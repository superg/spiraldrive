#include "floating_runtime_internals.h"

typedef union
{
    double value;
    u8 bytes[8];
} InternalFloat64Representation;

static void WriteInternalFloat64Byte(InternalFloat64Representation *value, u8 index, u8 byte)
{
#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/floating_runtime_internals_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static double BuildInternalFloat64(u32 high, u32 low)
{
    InternalFloat64Representation result;

    WriteInternalFloat64Byte(&result, 0u, (u8)(high >> 24));
    WriteInternalFloat64Byte(&result, 1u, (u8)(high >> 16));
    WriteInternalFloat64Byte(&result, 2u, (u8)(high >> 8));
    WriteInternalFloat64Byte(&result, 3u, (u8)high);
    WriteInternalFloat64Byte(&result, 4u, (u8)(low >> 24));
    WriteInternalFloat64Byte(&result, 5u, (u8)(low >> 16));
    WriteInternalFloat64Byte(&result, 6u, (u8)(low >> 8));
    WriteInternalFloat64Byte(&result, 7u, (u8)low);
    return result.value;
}

/* RAM:fd91e2 */
/* The source interface makes the native WR0/WR16/WR8/WR24/F0 transform
 * explicit. Lower significand words are unchanged by the canonical helper. */
void PLEXTOR_NEAR_FUNCTION PrepareFloat64OperandsForArithmetic(u16 left_header, u16 right_header, Float64ArithmeticOperands *prepared)
{
    prepared->left_significand_header = (u16)((left_header & 0x000fu) | 0x0010u);
    prepared->right_significand_header = (u16)((right_header & 0x000fu) | 0x0010u);
    prepared->left_doubled_exponent = (u16)((left_header & 0xfff0u) << 1);
    prepared->right_doubled_exponent = (u16)((right_header & 0xfff0u) << 1);
    prepared->negative = (u8)(((left_header ^ right_header) >> 15) & 1u);
}

/* RAM:fd9205 */
/* This is the source-level form of the shared binary64 round/exponent pack
 * tail. The original consumes DR0/DR4, WR8, F0 and the rounding carry. */
double FinalizeFloat64MantissaWithCarry(u32 mantissa_high, u32 mantissa_low, u16 doubled_exponent, u8 negative, u8 round_up)
{
    if(round_up != 0u)
    {
        ++mantissa_low;
        if(mantissa_low == 0u)
        {
            ++mantissa_high;
            mantissa_high &= 0x000ffffful;
            if(mantissa_high == 0u)
            {
                doubled_exponent = (u16)(doubled_exponent + 0x20u);
                if(doubled_exponent == 0u)
                    return BuildInternalFloat64(negative != 0u ? 0xfff00000ul : 0x7ff00000ul, 0ul);
            }
        }
    }

    mantissa_high &= 0x000ffffful;
    mantissa_high |= (u32)((u16)(doubled_exponent >> 1) | (negative != 0u ? 0x8000u : 0u)) << 16;
    return BuildInternalFloat64(mantissa_high, mantissa_low);
}

/* RAM:fd9259 */
/* The source record replaces the native WR0/WR4/WR8/F0 tuple. */
void PLEXTOR_NEAR_FUNCTION PrepareFloat32OperandsForArithmetic(u16 left_header, u16 right_header, Float32ArithmeticOperands *prepared)
{
    prepared->left_significand_header = (u8)((left_header & 0x007fu) | 0x0080u);
    prepared->right_significand_header = (u8)((right_header & 0x007fu) | 0x0080u);
    prepared->left_exponent = (u8)((left_header >> 7) & 0xffu);
    prepared->right_exponent = (u8)((right_header >> 7) & 0xffu);
    prepared->negative = (u8)(((left_header ^ right_header) >> 15) & 1u);
}

/* RAM:fd9272 */
/* mantissa_workspace retains the discarded rounding byte in bits 31..24;
 * the returned word contains the packed IEEE-754 sign/exponent/mantissa. */
u32 NormalizeFloatingPointMantissaWithCarry(u32 mantissa_workspace, u8 exponent, u8 negative)
{
    if((mantissa_workspace & 0x80000000ul) != 0u)
    {
        ++mantissa_workspace;
        mantissa_workspace &= 0x00fffffful;
        if(mantissa_workspace == 0u)
        {
            ++exponent;
            if(exponent == 0u)
                return (negative != 0u ? 0xff800000ul : 0x7f800000ul);
        }
    }
    return (negative != 0u ? 0x80000000ul : 0u) | ((u32)exponent << 23) | (mantissa_workspace & 0x007ffffful);
}
