#include "integer_division.h"

/* RAM:fd954c */
/* Divisor zero remains outside the source contract because the native DIV AB
 * result is architecturally undefined. */
s8 DivideS8WithRemainder(s8 dividend, s8 divisor, s8 *remainder)
{
    s16 wide_dividend = dividend;
    s16 wide_divisor = divisor;
    s16 quotient = (s16)(wide_dividend / wide_divisor);

    *remainder = (s8)(wide_dividend % wide_divisor);
    return (s8)(u8)quotient;
}

/* Addressless source adapter for reconstructed quotient-only callers. */
s8 PLEXTOR_NEAR_FUNCTION DivideS8ByTwoNative(s8 dividend)
{
    s8 remainder;

    return DivideS8WithRemainder(dividend, 2, &remainder);
}

/* Addressless source adapter for RAM:f89194's 120 / operating-code budget. */
s8 PLEXTOR_NEAR_FUNCTION DivideS8OperatingBudgetNative(u8 operating_code)
{
    s8 remainder;

    return DivideS8WithRemainder(120, (s8)operating_code, &remainder);
}

/* RAM:fd956e */
/* Field order is deliberate: C251 returns remainder in WR4 and quotient in
 * WR6, exactly matching the native dual-register result. */
SignedDivision16Result DivideS16WithRemainder(s16 dividend, s16 divisor)
{
    SignedDivision16Result result;

    result.quotient = (s16)((s32)dividend / (s32)divisor);
    result.remainder = (s16)((s32)dividend % (s32)divisor);
    return result;
}

/* Addressless source adapter for reconstructed quotient-only callers. */
s16 DivideS16Quotient(s16 dividend, s16 divisor)
{
    return DivideS16WithRemainder(dividend, divisor).quotient;
}

/* RAM:fd9602 */
/* Field order is deliberate: C251 returns remainder in DR0 and quotient in
 * DR4, exactly matching the native dual-register result. */
SignedDivision32Result DivideS32WithRemainder(s32 dividend, s32 divisor)
{
    SignedDivision32Result result;

    if((u32)dividend == 0x80000000ul && divisor == -1l)
    {
        result.quotient = (s32)0x80000000ul;
        result.remainder = 0;
    }
    else
    {
        result.quotient = dividend / divisor;
        result.remainder = dividend % divisor;
    }
    return result;
}

/* Addressless source adapter for reconstructed quotient-only callers. */
s32 DivideS32Quotient(s32 dividend, s32 divisor)
{
    return DivideS32WithRemainder(dividend, divisor).quotient;
}

/* RAM:fd95b1 */
/* Avoid C division here because this function is the target's u32 divider.
 * Field order returns remainder in DR0 and quotient in DR4. */
UnsignedDivision32Result DivideU32WithRemainder(u32 dividend, u32 divisor)
{
    UnsignedDivision32Result result;
    u32 remainder = 0ul;
    u32 quotient = 0ul;
    u8 bit_count = 32u;

    if(divisor == 0ul)
    {
        u16 low_word = (u16)dividend;

        if(dividend <= 0xfffful)
        {
            result.remainder = 0ul;
            result.quotient = dividend;
        }
        else
        {
            result.remainder = (u32)low_word;
            result.quotient = ((u32)low_word << 16) | 0xfffful;
        }
        return result;
    }
    do
    {
        u8 carry = (u8)(remainder >> 31);

        remainder = (remainder << 1) | (dividend >> 31);
        dividend <<= 1;
        quotient <<= 1;
        if(carry || remainder >= divisor)
        {
            remainder -= divisor;
            quotient |= 1ul;
        }
    } while(--bit_count != 0u);
    result.remainder = remainder;
    result.quotient = quotient;
    return result;
}

/* Addressless source adapter for reconstructed quotient-only callers. */
u32 DivideU32Quotient(u32 dividend, u32 divisor)
{
    return DivideU32WithRemainder(dividend, divisor).quotient;
}

u32 DivideU32QuotientNative(u32 dividend, u32 divisor)
{
    return DivideU32WithRemainder(dividend, divisor).quotient;
}

u32 DivideU32RemainderNative(u32 dividend, u32 divisor)
{
    return DivideU32WithRemainder(dividend, divisor).remainder;
}

u16 DivideU16QuotientNative(u16 dividend, u16 divisor)
{
    return (u16)(dividend / divisor);
}

s32 DivideS32QuotientNative(s32 dividend, s32 divisor)
{
    return DivideS32WithRemainder(dividend, divisor).quotient;
}

u32 DivideS32MagnitudeRemainderNative(s32 dividend, s32 divisor)
{
    return (u32)DivideS32WithRemainder(dividend, divisor).remainder;
}
