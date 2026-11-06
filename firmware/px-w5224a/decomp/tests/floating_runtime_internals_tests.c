#include <assert.h>
#include <stdio.h>
#include "platform/floating_runtime_internals.h"

typedef union
{
    double value;
    unsigned long long bits;
} HostFloat64Bits;

static unsigned long long ToBits(double value)
{
    HostFloat64Bits representation;

    representation.value = value;
    return representation.bits;
}

static void TestPrepareFloat64Operands(void)
{
    Float64ArithmeticOperands prepared;

    PrepareFloat64OperandsForArithmetic(0xbff8u, 0x4004u, &prepared);
    assert(prepared.left_significand_header == 0x0018u);
    assert(prepared.right_significand_header == 0x0014u);
    assert(prepared.left_doubled_exponent == 0x7fe0u);
    assert(prepared.right_doubled_exponent == 0x8000u);
    assert(prepared.negative == 1u);
}

static void TestFinalizeFloat64Mantissa(void)
{
    double value;

    value = FinalizeFloat64MantissaWithCarry(0x00080000ul, 0x12345678ul, 0x7fe0u, 0u, 0u);
    assert(ToBits(value) == 0x3ff8000012345678ull);
    value = FinalizeFloat64MantissaWithCarry(0x00080000ul, 0xfffffffful, 0x7fe0u, 1u, 1u);
    assert(ToBits(value) == 0xbff8000100000000ull);
    value = FinalizeFloat64MantissaWithCarry(0x000ffffful, 0xfffffffful, 0xffe0u, 0u, 1u);
    assert(ToBits(value) == 0x7ff0000000000000ull);
}

static void TestPrepareFloat32Operands(void)
{
    Float32ArithmeticOperands prepared;

    PrepareFloat32OperandsForArithmetic(0xbfc0u, 0x4020u, &prepared);
    assert(prepared.left_significand_header == 0xc0u);
    assert(prepared.right_significand_header == 0xa0u);
    assert(prepared.left_exponent == 0x7fu);
    assert(prepared.right_exponent == 0x80u);
    assert(prepared.negative == 1u);
}

static void TestNormalizeFloat32Mantissa(void)
{
    assert(NormalizeFloatingPointMantissaWithCarry(0x00123456ul, 0x7fu, 0u) == 0x3f923456ul);
    assert(NormalizeFloatingPointMantissaWithCarry(0x80123456ul, 0x7fu, 1u) == 0xbf923457ul);
    assert(NormalizeFloatingPointMantissaWithCarry(0xfffffffful, 0xffu, 0u) == 0x7f800000ul);
}

int main(void)
{
    TestPrepareFloat64Operands();
    TestFinalizeFloat64Mantissa();
    TestPrepareFloat32Operands();
    TestNormalizeFloat32Mantissa();
    puts("floating runtime internals tests passed");
    return 0;
}
