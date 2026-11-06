#ifndef PLEXTOR_INTEGER_DIVISION_H
#define PLEXTOR_INTEGER_DIVISION_H

#include "mcs251.h"

s8 DivideS8WithRemainder(s8 dividend, s8 divisor, s8 *remainder);
s8 PLEXTOR_NEAR_FUNCTION DivideS8ByTwoNative(s8 dividend);
s8 PLEXTOR_NEAR_FUNCTION DivideS8OperatingBudgetNative(u8 operating_code);

typedef struct
{
    s16 remainder;
    s16 quotient;
} SignedDivision16Result;

SignedDivision16Result DivideS16WithRemainder(s16 dividend, s16 divisor);
s16 DivideS16Quotient(s16 dividend, s16 divisor);

typedef struct
{
    s32 remainder;
    s32 quotient;
} SignedDivision32Result;

SignedDivision32Result DivideS32WithRemainder(s32 dividend, s32 divisor);
s32 DivideS32Quotient(s32 dividend, s32 divisor);

typedef struct
{
    u32 remainder;
    u32 quotient;
} UnsignedDivision32Result;

UnsignedDivision32Result DivideU32WithRemainder(u32 dividend, u32 divisor);
u32 DivideU32Quotient(u32 dividend, u32 divisor);
u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
u32 DivideU32RemainderNative(u32 dividend, u32 divisor);
u16 DivideU16QuotientNative(u16 dividend, u16 divisor);
s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
u32 DivideS32MagnitudeRemainderNative(s32 dividend, s32 divisor);

#endif
