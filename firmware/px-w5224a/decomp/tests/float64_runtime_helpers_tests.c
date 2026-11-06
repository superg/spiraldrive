#include <assert.h>
#include <stdio.h>
#include "platform/float64_runtime_helpers.h"

typedef union
{
    double value;
    unsigned long long bits;
} HostFloat64Bits;

static double FromBits(unsigned long long bits)
{
    HostFloat64Bits value;

    value.bits = bits;
    return value.value;
}

static unsigned long long ToBits(double value)
{
    HostFloat64Bits bits;

    bits.value = value;
    return bits.bits;
}

static void TestScaleNormalAndZero(void)
{
    assert(ToBits(ScaleFloat64ByPowerOfTwo(1.5, 1)) == 0x4008000000000000ull);
    assert(ToBits(ScaleFloat64ByPowerOfTwo(-1.5, -1)) == 0xbfe8000000000000ull);
    assert(ToBits(ScaleFloat64ByPowerOfTwo(0.0, 99)) == 0ull);
}

static void TestScaleUnderflowAndOverflow(void)
{
    assert(ToBits(ScaleFloat64ByPowerOfTwo(1.0, -1023)) == 0ull);
    assert(ToBits(ScaleFloat64ByPowerOfTwo(-1.0, -1023)) == 0ull);
    assert(ToBits(ScaleFloat64ByPowerOfTwo(1.0, 1025)) == 0x800fffffffffffffull);
    assert(ToBits(ScaleFloat64ByPowerOfTwo(-1.0, 1025)) == 0x800fffffffffffffull);
}

static void TestScaleSpecialPayloads(void)
{
    double infinity = FromBits(0x7ff0000000000000ull);
    double nan_payload = FromBits(0x7ff8123456789abcull);

    assert(ToBits(ScaleFloat64ByPowerOfTwo(infinity, 0)) == 0x7ff0000000000000ull);
    assert(ToBits(ScaleFloat64ByPowerOfTwo(nan_payload, 0)) == 0x7ff8123456789abcull);
    assert(ToBits(ScaleFloat64ByPowerOfTwo(FromBits(0x8000000000000000ull), 0)) == 0ull);
}

static void TestDecomposeNormalAndZero(void)
{
    s16 exponent = 99;
    double mantissa = DecomposeFloat64MantissaAndExponent(8.0, &exponent);

    assert(exponent == 4);
    assert(ToBits(mantissa) == 0x3fe0000000000000ull);
    mantissa = DecomposeFloat64MantissaAndExponent(0.0, &exponent);
    assert(exponent == 0 && ToBits(mantissa) == 0ull);
}

static void TestDecomposeRawSubnormalAndSpecialCases(void)
{
    s16 exponent;
    double mantissa;

    mantissa = DecomposeFloat64MantissaAndExponent(FromBits(0x0000000000000001ull), &exponent);
    assert(exponent == -1022);
    assert(ToBits(mantissa) == 0x3fe0000000000001ull);
    mantissa = DecomposeFloat64MantissaAndExponent(FromBits(0x8000000000000000ull), &exponent);
    assert(exponent == -1022);
    assert(ToBits(mantissa) == 0xbfe0000000000000ull);
    mantissa = DecomposeFloat64MantissaAndExponent(FromBits(0x7ff8123456789abcull), &exponent);
    assert(exponent == 1025);
    assert(ToBits(mantissa) == 0x3fe8123456789abcull);
}

static void TestCompareStrictOrdering(void)
{
    assert(CompareFloat64(-2.0, -1.0) == 1u);
    assert(CompareFloat64(-1.0, -2.0) == 0u);
    assert(CompareFloat64(1.0, 2.0) == 1u);
    assert(CompareFloat64(2.0, 1.0) == 0u);
    assert(CompareFloat64(1.0, 1.0) == 0u);
    assert(CompareFloat64(-0.0, 0.0) == 0u);
    assert(CompareFloat64(0.0, -0.0) == 0u);
    assert(CompareFloat64(FromBits(0xfff0000000000000ull), -1.0) == 1u);
    assert(CompareFloat64(FromBits(0x7ff0000000000000ull), 1.0) == 0u);
}

static void TestSquareRootNormalAndSpecialValues(void)
{
    assert(ToBits(SquareRootFloat64(0.0)) == 0ull);
    assert(ToBits(SquareRootFloat64(4.0)) == 0x4000000000000000ull);
    assert(ToBits(SquareRootFloat64(2.25)) == 0x3ff8000000000000ull);
    assert(ToBits(SquareRootFloat64(FromBits(0x0000000000000001ull))) == 0x0000000000000001ull);
    assert(ToBits(SquareRootFloat64(-0.0)) == 0xffffffffffffffffull);
    assert(ToBits(SquareRootFloat64(-4.0)) == 0xffffffffffffffffull);
    assert(ToBits(SquareRootFloat64(FromBits(0x7ff8123456789abcull))) == 0x7ff0000000000000ull);
    assert(ToBits(SquareRootFloat64(FromBits(0x7ff0000000000000ull))) == 0x7ff0000000000000ull);
}

static void TestArithmeticFiniteValues(void)
{
    assert(ToBits(AddFloat64(1.5, 2.25)) == 0x400e000000000000ull);
    assert(ToBits(SubtractFloat64(5.5, 2.25)) == 0x400a000000000000ull);
    assert(ToBits(MultiplyFloat64(-1.5, 2.0)) == 0xc008000000000000ull);
    assert(ToBits(DivideFloat64(-7.5, 2.5)) == 0xc008000000000000ull);
}

static void TestArithmeticExponentZeroRules(void)
{
    assert(ToBits(AddFloat64(FromBits(0x8000000000000001ull), 2.0)) == 0x4000000000000000ull);
    assert(ToBits(AddFloat64(3.0, FromBits(0x0000000000000001ull))) == 0x4008000000000000ull);
    assert(ToBits(SubtractFloat64(FromBits(0x8000000000000001ull), 2.0)) == 0xc000000000000000ull);
    assert(ToBits(MultiplyFloat64(FromBits(0x0010000000000000ull), 0.5)) == 0ull);
    assert(ToBits(DivideFloat64(FromBits(0x0010000000000000ull), 2.0)) == 0ull);
}

static void TestArithmeticSpecialClasses(void)
{
    assert(ToBits(AddFloat64(FromBits(0x7ff0000000000000ull), 1.0)) == 0xffffffffffffffffull);
    assert(ToBits(MultiplyFloat64(0.0, FromBits(0x7ff0000000000000ull))) == 0xffffffffffffffffull);
    assert(ToBits(MultiplyFloat64(FromBits(0xfff8123456789abcull), 2.0)) == 0xfff0000000000000ull);
    assert(ToBits(DivideFloat64(0.0, 0.0)) == 0xffffffffffffffffull);
    assert(ToBits(DivideFloat64(FromBits(0x7ff0000000000000ull), FromBits(0xfff0000000000000ull))) == 0xffffffffffffffffull);
    assert(ToBits(DivideFloat64(-1.0, 0.0)) == 0xfff0000000000000ull);
    assert(ToBits(DivideFloat64(1.0, FromBits(0x7ff0000000000000ull))) == 0ull);
}

int main(void)
{
    TestScaleNormalAndZero();
    TestScaleUnderflowAndOverflow();
    TestScaleSpecialPayloads();
    TestDecomposeNormalAndZero();
    TestDecomposeRawSubnormalAndSpecialCases();
    TestCompareStrictOrdering();
    TestSquareRootNormalAndSpecialValues();
    TestArithmeticFiniteValues();
    TestArithmeticExponentZeroRules();
    TestArithmeticSpecialClasses();
    puts("float64_runtime_helpers_tests: ok");
    return 0;
}
