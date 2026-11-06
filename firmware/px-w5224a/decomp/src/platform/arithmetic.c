#include "arithmetic.h"
#include "firmware_memory.h"
#include "memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/arithmetic_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadPseudoRandomState(void)
{
    return FirmwareReadNearU32(FW_NEAR_UNKNOWN_FBFC);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WritePseudoRandomState(u32 state)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBFC, (u8)(state >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBFD, (u8)(state >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBFE, (u8)(state >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBFF, (u8)state);
}

/* Reconstruction helper; no direct Ghidra function address.
 * This explicit divider keeps the source-only target independent of C251's
 * unreviewed long-division runtime library. */
static u32 DivideU32ByU16Arithmetic(u32 dividend, u16 divisor, u16 *remainder)
{
    u32 quotient = 0;
    u16 partial = 0;
    u8 bit_count = 32;

    do
    {
        partial = (u16)((partial << 1) | (u16)(dividend >> 31));
        dividend <<= 1;
        quotient <<= 1;
        if(partial >= divisor)
        {
            partial = (u16)(partial - divisor);
            quotient |= 1ul;
        }
        bit_count--;
    } while(bit_count != 0);
    *remainder = partial;
    return quotient;
}

/* RAM:fddd27 */
u8 MinU8(u8 value, u8 limit)
{
    return value <= limit ? value : limit;
}

/* RAM:fddd30 */
u16 MinU16(u16 value, u16 limit)
{
    return value <= limit ? value : limit;
}

/* RAM:fddd39 */
u32 MinU32(u32 value, u32 limit)
{
    return value <= limit ? value : limit;
}

/* RAM:f849e3 */
s16 AbsoluteValueI16(s16 value)
{
    if(value >= 0)
        return value;
    /* Unsigned negation retains the original 0x8000 result for INT16_MIN. */
    return (s16)(u16)(0u - (u16)value);
}

/* RAM:f8f38b */
/* The native binary64 divide/decompose/scale sequence selects the power of
 * two immediately below value, except that mantissas above its 1.41421356
 * threshold select the following power. The
 * threshold lies between adjacent
 * binary32 mantissas 0x3504f3 and 0x3504f4. */
float PLEXTOR_NEAR_FUNCTION NaturalLogFloat32(float value)
{
    union
    {
        float value;
        u32 bits;
    } input, scale, result;
    u32 fraction;
    s16 binary_exponent;
    s16 scale_exponent;
    s16 stage;
    float reduced;
    float continued_fraction = 0.0f;

    input.value = value;
    if((input.bits & 0x7ffffffful) == 0ul)
    {
        result.bits = 0xc08bbbaaul;
        return result.value;
    }
    if((input.bits & 0x80000000ul) != 0ul)
    {
        result.bits = 0x7fc00000ul;
        return result.value;
    }
    if((input.bits & 0x7f800000ul) == 0x7f800000ul)
        return value;

    fraction = input.bits & 0x007ffffful;
    binary_exponent = (s16)((input.bits >> 23) & 0xffu) - 127;
    if(((input.bits >> 23) & 0xffu) == 0u)
    {
        binary_exponent = -127;
        while((fraction & 0x00400000ul) == 0ul)
        {
            fraction <<= 1;
            --binary_exponent;
        }
        fraction = (fraction << 1) & 0x007ffffful;
    }

    scale_exponent = binary_exponent;
    if(fraction > 0x003504f3ul)
        ++scale_exponent;
    if(scale_exponent > 127)
        scale.bits = 0x7f800000ul;
    else if(scale_exponent >= -126)
        scale.bits = (u32)(scale_exponent + 127) << 23;
    else
        scale.bits = 1ul << (u8)(scale_exponent + 149);

    reduced = value / scale.value;
    reduced -= 1.0f;
    for(stage = 4; stage > 0; --stage)
    {
        float denominator = (float)(stage * 2 + 1) + continued_fraction;

        denominator = ((float)stage * reduced) / denominator;
        denominator += 2.0f;
        continued_fraction = ((float)stage * reduced) / denominator;
    }
    continued_fraction += 1.0f;
    reduced /= continued_fraction;
    return (float)scale_exponent * 0.6931472f + reduced;
}

/* RAM:f8f342 */
s32 Compute2000TimesLog10OfWordOver32767(u16 value)
{
    float scaled_value = (float)value;

    scaled_value /= 32767.0f;
    scaled_value = NaturalLogFloat32(scaled_value);
    scaled_value /= 2.3025851f;
    scaled_value *= 2000.0f;
    return (s32)scaled_value;
}

/* RAM:f8f8ad */
u32 ScaleUnsignedWordBy83Point5ToU32(u16 value)
{
    /* The reviewed binary64 -> binary32 -> unsigned conversion is exact for
     * all u16 inputs: 83.5 * 65535 is below 2^23 and the final conversion
     * truncates the half-unit for odd values. */
    return (u32)value * 83ul + (u32)(value >> 1);
}

/* RAM:fd95a0 */
u32 MultiplyU32Low(u32 factor_a, u32 factor_b)
{
    u32 product = 0;

    /* The original returns the low word of the mathematical product.  Keep
     * the wrap explicit so C251 does not import an unreviewed long-multiply
     * library routine. */
    while(factor_b != 0)
    {
        if((factor_b & 1ul) != 0)
            product += factor_a;
        factor_a <<= 1;
        factor_b >>= 1;
    }
    return product;
}

/* RAM:fda732 */
u32 PLEXTOR_NEAR_FUNCTION PowerOfTenU32(u8 exponent)
{
    u32 value = 1;

    while(exponent != 0)
    {
        value = (value << 3) + (value << 1);
        exponent--;
    }
    return value;
}

/* RAM:fee6c4 */
u32 IntegerSquareRootU32(u32 value)
{
    u32 root = 0;
    u32 trial_bit = 0x40000000ul;

    while(trial_bit > value)
        trial_bit >>= 2;
    while(trial_bit != 0)
    {
        if(value >= root + trial_bit)
        {
            value -= root + trial_bit;
            root = (root >> 1) + trial_bit;
        }
        else
        {
            root >>= 1;
        }
        trial_bit >>= 2;
    }
    return root;
}

/* RAM:fee5aa */
void UpdateFourByteBitConsensusState(fw_addr_t state_address, u8 new_value)
{
    u8 previous_value;
    fw_addr_t state_byte_1;
    fw_addr_t state_byte_2;
    fw_addr_t state_byte_3;
    u8 reference_value;
    u8 consensus;
    u8 current_value;
    u8 accumulated_value;

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/arithmetic_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    previous_value = FirmwareRead8(state_address);
    state_byte_1 = FirmwareAddressIncrement24(state_address, 1);
    state_byte_2 = FirmwareAddressIncrement24(state_address, 2);
    state_byte_3 = FirmwareAddressIncrement24(state_address, 3);

    FirmwareWrite8(state_address, new_value);
    reference_value = FirmwareRead8(state_byte_1);
    consensus = (u8)(((new_value ^ reference_value) & (previous_value ^ reference_value)) ^ reference_value);
    FirmwareWrite8(state_byte_1, reference_value);
    current_value = FirmwareRead8(state_address);
    accumulated_value = FirmwareRead8(state_byte_2);
    FirmwareWrite8(state_byte_2, (u8)(accumulated_value | (current_value & consensus)));
    current_value = FirmwareRead8(state_address);
    accumulated_value = FirmwareRead8(state_byte_3);
    FirmwareWrite8(state_byte_3, (u8)(accumulated_value | ((u8)~current_value & consensus)));
}

/* RAM:fd7b56 */
/* XRL WR4,WR4 clears the upper half of DR4; WR6 remains the 16-bit seed. */
void ResetPseudoRandomGenerator(u16 seed)
{
    WritePseudoRandomState((u32)seed);
}

/* RAM:fd7b5d */
u16 NextPseudoRandom15Bit(void)
{
    u32 state = MultiplyU32Low(0x41c64e6dul, ReadPseudoRandomState());

    state += 0x3039ul;
    WritePseudoRandomState(state);
    return (u16)((state >> 16) & 0x7ffful);
}

/* RAM:fa7a78 */
void SortWordArrayAscendingInPlace(fw_addr_t array_address, u16 element_count)
{
    s16 last_swap_index = (s16)(element_count - 1u);

    while(last_swap_index >= 0)
    {
        s16 next_last_swap_index = -1;
        u16 index = 1;

        while((s16)index <= last_swap_index)
        {
            fw_addr_t left_address = FirmwareAddressIncrementLow16(array_address, (u16)((index - 1u) << 1));
            fw_addr_t right_address = FirmwareAddressIncrementLow16(array_address, (u16)(index << 1));
            u16 left_value = ReadBigEndianU16(left_address);
            u16 right_value = ReadBigEndianU16(right_address);

            if(right_value < left_value)
            {
                next_last_swap_index = (s16)(index - 1u);
                WriteBigEndianU16(left_address, right_value);
                WriteBigEndianU16(right_address, left_value);
            }
            index = (u16)(index + 1u);
        }
        last_swap_index = next_last_swap_index;
    }
}

/* RAM:fb9a82 */
void SortByteArrayAscendingInPlace(fw_addr_t array_address, u16 element_count)
{
    s16 last_swap_index = (s16)(element_count - 1u);

    while(last_swap_index >= 0)
    {
        s16 next_last_swap_index = -1;
        u16 index = 1;

        while((s16)index <= last_swap_index)
        {
            fw_addr_t left_address = FirmwareAddressIncrementLow16(array_address, index - 1u);
            fw_addr_t right_address = FirmwareAddressIncrementLow16(array_address, index);
            u8 left_value = FirmwareRead8(left_address);
            u8 right_value = FirmwareRead8(right_address);

            if(right_value < left_value)
            {
                next_last_swap_index = (s16)(index - 1u);
                FirmwareWrite8(left_address, right_value);
                FirmwareWrite8(right_address, left_value);
            }
            index = (u16)(index + 1u);
        }
        last_swap_index = next_last_swap_index;
    }
}

/* RAM:fc3417 */
u16 ScaleUnsignedByteBy176Point4Rounded(u8 value)
{
    u16 ignored_remainder;
    u16 final_remainder;
    u32 scaled = MultiplyU32Low((u32)value, 176400ul);

    scaled = DivideU32ByU16Arithmetic(scaled, 100u, &ignored_remainder);
    scaled = DivideU32ByU16Arithmetic(scaled, 10u, &final_remainder);
    return (u16)(scaled + (final_remainder >= 5u ? 1u : 0u));
}

/* RAM:fee695 */
u8 PLEXTOR_NEAR_FUNCTION ToggleR3Bit7IfSignedR3Negative(s8 value)
{
    u8 result = (u8)value;

    if(value < 0)
        result ^= 0x80u;
    return result;
}

/* RAM:fee6a4 */
u8 PLEXTOR_NEAR_FUNCTION ToggleR3Bit7IfSignedWR2Negative(s16 value)
{
    u8 result = (u8)value;

    if(value < 0)
        result ^= 0x80u;
    return result;
}

/* RAM:fee6b4 */
u8 PLEXTOR_NEAR_FUNCTION ToggleR3Bit7IfSignedDR0Negative(s32 value)
{
    u8 result = (u8)value;

    if(value < 0)
        result ^= 0x80u;
    return result;
}

/* Reconstruction helper; no direct Ghidra function address. */
u16 GetCdSpeedKilobytesPerSecond(u8 speed_x)
{
    u16 kilobytes_per_second;

    switch(speed_x)
    {
    case 0:
        kilobytes_per_second = 0x0000u;
        break;
    case 4:
        kilobytes_per_second = 0x02c2u;
        break;
    case 8:
        kilobytes_per_second = 0x0583u;
        break;
    case 10:
        kilobytes_per_second = 0x06e4u;
        break;
    case 16:
        kilobytes_per_second = 0x0b06u;
        break;
    case 20:
        kilobytes_per_second = 0x0dc8u;
        break;
    case 24:
        kilobytes_per_second = 0x108au;
        break;
    case 32:
        kilobytes_per_second = 0x160du;
        break;
    case 40:
        kilobytes_per_second = 0x1b90u;
        break;
    case 52:
        kilobytes_per_second = 0x23d5u;
        break;
    default:
        kilobytes_per_second = ScaleUnsignedByteBy176Point4Rounded(speed_x);
        break;
    }
    return kilobytes_per_second;
}

/* RAM:fc3368 */
void WriteCdSpeedKilobytesPerSecond(u8 speed_x, fw_addr_t output_address)
{
    u16 kilobytes_per_second = GetCdSpeedKilobytesPerSecond(speed_x);

    FirmwareWrite8(output_address, (u8)(kilobytes_per_second >> 8));
    FirmwareWrite8(FirmwareAddressIncrement24(output_address, 1), (u8)kilobytes_per_second);
}
