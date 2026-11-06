#include <assert.h>
#include <stdio.h>
#include "platform/float32_runtime_arithmetic.h"

typedef union
{
    float value;
    u32 bits;
} HostFloat32Bits;

static float FromBits(u32 bits)
{
    HostFloat32Bits value;

    value.bits = bits;
    return value.value;
}

static u32 ToBits(float value)
{
    HostFloat32Bits representation;

    representation.value = value;
    return representation.bits;
}

static void TestFiniteArithmetic(void)
{
    assert(ToBits(AddFloat32(1.5f, 2.25f)) == 0x40700000ul);
    assert(ToBits(SubtractFloat32(5.5f, 2.25f)) == 0x40500000ul);
    assert(ToBits(AddFloat32(1.0f, FromBits(0x33800000ul))) == 0x3f800000ul);
    assert(ToBits(SubtractFloat32(1.0f, 1.0f)) == 0x00000000ul);
    assert(ToBits(AddFloat32(FromBits(0x7f7ffffful), FromBits(0x7f7ffffful))) == 0x7f800000ul);
}

static void TestExponentZeroClassification(void)
{
    assert(ToBits(AddFloat32(FromBits(0x80000001ul), 2.0f)) == 0x40000000ul);
    assert(ToBits(AddFloat32(3.0f, FromBits(0x00000001ul))) == 0x40400000ul);
    assert(ToBits(SubtractFloat32(FromBits(0x80000001ul), 2.0f)) == 0xc0000000ul);
    assert(ToBits(SubtractFloat32(3.0f, FromBits(0x80000001ul))) == 0x40400000ul);
}

static void TestExponent255Classification(void)
{
    assert(ToBits(AddFloat32(FromBits(0x7f800000ul), 1.0f)) == 0xfffffffful);
    assert(ToBits(AddFloat32(1.0f, FromBits(0xff800000ul))) == 0xfffffffful);
    assert(ToBits(SubtractFloat32(1.0f, FromBits(0x7fc12345ul))) == 0xfffffffful);
}

static void TestMultiplyFloat32(void)
{
    assert(ToBits(MultiplyFloat32(1.5f, 2.0f)) == 0x40400000ul);
    assert(ToBits(MultiplyFloat32(-1.5f, 2.0f)) == 0xc0400000ul);
    assert(ToBits(MultiplyFloat32(FromBits(0x00000001ul), 2.0f)) == 0u);
    assert(ToBits(MultiplyFloat32(0.0f, FromBits(0x7f800000ul))) == 0xfffffffful);
    assert(ToBits(MultiplyFloat32(FromBits(0xffc12345ul), 2.0f)) == 0xff800000ul);
    assert(ToBits(MultiplyFloat32(FromBits(0x00800000ul), 0.5f)) == 0u);
}

static void TestDivideFloat32(void)
{
    assert(ToBits(DivideFloat32(7.5f, 2.5f)) == 0x40400000ul);
    assert(ToBits(DivideFloat32(-7.5f, 2.5f)) == 0xc0400000ul);
    assert(ToBits(DivideFloat32(0.0f, 0.0f)) == 0xfffffffful);
    assert(ToBits(DivideFloat32(FromBits(0x7f800000ul), FromBits(0xff800000ul))) == 0xfffffffful);
    assert(ToBits(DivideFloat32(-1.0f, 0.0f)) == 0xff800000ul);
    assert(ToBits(DivideFloat32(1.0f, FromBits(0x7f800000ul))) == 0u);
    assert(ToBits(DivideFloat32(FromBits(0x00800000ul), 2.0f)) == 0u);
}

static void TestCompareFloat32(void)
{
    assert(CompareFloat32(-2.0f, -1.0f) == 1u);
    assert(CompareFloat32(-1.0f, -2.0f) == 0u);
    assert(CompareFloat32(1.0f, 2.0f) == 1u);
    assert(CompareFloat32(2.0f, 1.0f) == 0u);
    assert(CompareFloat32(1.0f, 1.0f) == 0u);
    assert(CompareFloat32(-0.0f, 0.0f) == 0u);
    assert(CompareFloat32(0.0f, -0.0f) == 0u);
    assert(CompareFloat32(FromBits(0xff800000ul), -1.0f) == 1u);
    assert(CompareFloat32(FromBits(0x7f800000ul), 1.0f) == 0u);
}

int main(void)
{
    TestFiniteArithmetic();
    TestExponentZeroClassification();
    TestExponent255Classification();
    TestMultiplyFloat32();
    TestDivideFloat32();
    TestCompareFloat32();
    puts("float32_runtime_arithmetic_tests: ok");
    return 0;
}
