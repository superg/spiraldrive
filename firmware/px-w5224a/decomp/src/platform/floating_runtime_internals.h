#ifndef PLEXTOR_FLOATING_RUNTIME_INTERNALS_H
#define PLEXTOR_FLOATING_RUNTIME_INTERNALS_H

#include "mcs251.h"

typedef struct
{
    u16 left_significand_header;
    u16 right_significand_header;
    u16 left_doubled_exponent;
    u16 right_doubled_exponent;
    u8 negative;
} Float64ArithmeticOperands;

typedef struct
{
    u8 left_significand_header;
    u8 right_significand_header;
    u8 left_exponent;
    u8 right_exponent;
    u8 negative;
} Float32ArithmeticOperands;

void PLEXTOR_NEAR_FUNCTION PrepareFloat64OperandsForArithmetic(u16 left_header, u16 right_header, Float64ArithmeticOperands *prepared);
double FinalizeFloat64MantissaWithCarry(u32 mantissa_high, u32 mantissa_low, u16 doubled_exponent, u8 negative, u8 round_up);
void PLEXTOR_NEAR_FUNCTION PrepareFloat32OperandsForArithmetic(u16 left_header, u16 right_header, Float32ArithmeticOperands *prepared);
u32 NormalizeFloatingPointMantissaWithCarry(u32 mantissa_workspace, u8 exponent, u8 negative);

#endif
