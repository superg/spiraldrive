#include <assert.h>
#include <stdio.h>
#include "platform/floating_width_conversions.h"

typedef union
{
    float value;
    u32 bits;
} HostFloat32WidthBits;

typedef union
{
    double value;
    unsigned long long bits;
} HostFloat64WidthBits;

static float Float32FromBits(u32 bits)
{
    HostFloat32WidthBits value;

    value.bits = bits;
    return value.value;
}

static u32 Float32ToBits(float value)
{
    HostFloat32WidthBits representation;

    representation.value = value;
    return representation.bits;
}

static double Float64FromBits(unsigned long long bits)
{
    HostFloat64WidthBits value;

    value.bits = bits;
    return value.value;
}

static unsigned long long Float64ToBits(double value)
{
    HostFloat64WidthBits representation;

    representation.value = value;
    return representation.bits;
}

static void TestFloat32ToFloat64(void)
{
    assert(Float64ToBits(ConvertFloat32ToFloat64(1.5f)) == 0x3ff8000000000000ull);
    assert(Float64ToBits(ConvertFloat32ToFloat64(-2.25f)) == 0xc002000000000000ull);
    assert(Float64ToBits(ConvertFloat32ToFloat64(Float32FromBits(0x80000001ul))) == 0ull);
    assert(Float64ToBits(ConvertFloat32ToFloat64(Float32FromBits(0x7f800000ul))) == 0x7ff0000000000000ull);
    assert(Float64ToBits(ConvertFloat32ToFloat64(Float32FromBits(0xff800000ul))) == 0xfff0000000000000ull);
    assert(Float64ToBits(ConvertFloat32ToFloat64(Float32FromBits(0x7fc12345ul))) == 0xffffffffffffffffull);
}

static void TestFloat64ToFloat32(void)
{
    assert(Float32ToBits(ConvertFloat64ToFloat32(1.5)) == 0x3fc00000ul);
    assert(Float32ToBits(ConvertFloat64ToFloat32(-2.25)) == 0xc0100000ul);
    assert(Float32ToBits(ConvertFloat64ToFloat32(Float64FromBits(0x8000000000000001ull))) == 0u);
    assert(Float32ToBits(ConvertFloat64ToFloat32(Float64FromBits(0x7ff0000000000000ull))) == 0xfffffffful);
    assert(Float32ToBits(ConvertFloat64ToFloat32(Float64FromBits(0xfff8123456789abcull))) == 0xfffffffful);
    assert(Float32ToBits(ConvertFloat64ToFloat32(0x1p-127)) == 0u);
    assert(Float32ToBits(ConvertFloat64ToFloat32(0x1p+128)) == 0x7f800000ul);
}

static void TestNegateFloat64(void)
{
    assert(Float64ToBits(NegateFloat64(1.0)) == 0xbff0000000000000ull);
    assert(Float64ToBits(NegateFloat64(-1.0)) == 0x3ff0000000000000ull);
    assert(Float64ToBits(NegateFloat64(0.0)) == 0ull);
    assert(Float64ToBits(NegateFloat64(Float64FromBits(0x0000000000000001ull))) == 0x0000000000000001ull);
    assert(Float64ToBits(NegateFloat64(Float64FromBits(0x8000000000000000ull))) == 0ull);
    assert(Float64ToBits(NegateFloat64(Float64FromBits(0xffff123456789abcull))) == 0xffff123456789abcull);
}

static void TestFloat64ToInteger32(void)
{
    assert(ConvertFloat64ToInteger32(0.75) == 0);
    assert(ConvertFloat64ToInteger32(1.75) == 1);
    assert(ConvertFloat64ToInteger32(-1.75) == -1);
    assert((u32)ConvertFloat64ToInteger32(2147483648.0) == 0x80000000ul);
    assert((u32)ConvertFloat64ToInteger32(4294967295.0) == 0xfffffffful);
    assert((u32)ConvertFloat64ToInteger32(4294967296.0) == 0xfffffffful);
    assert((u32)ConvertFloat64ToInteger32(Float64FromBits(0x7ff0000000000000ull)) == 0xfffffffful);
    assert(ConvertFloat64ToInteger32(Float64FromBits(0x0000000000000001ull)) == 0);
}

static void TestIntegerToFloat64(void)
{
    assert(Float64ToBits(ConvertInteger8ToFloat64(-128)) == 0xc060000000000000ull);
    assert(Float64ToBits(ConvertInteger8ToFloat64(127)) == 0x405fc00000000000ull);
    assert(Float64ToBits(ConvertInteger16ToFloat64(-32768)) == 0xc0e0000000000000ull);
    assert(Float64ToBits(ConvertInteger16ToFloat64(32767)) == 0x40dfffc000000000ull);
    assert(Float64ToBits(ConvertInteger32ToFloat64((-2147483647l - 1l))) == 0xc1e0000000000000ull);
    assert(Float64ToBits(ConvertInteger32ToFloat64(2147483647l)) == 0x41dfffffffc00000ull);
}

int main(void)
{
    TestFloat32ToFloat64();
    TestFloat64ToFloat32();
    TestNegateFloat64();
    TestFloat64ToInteger32();
    TestIntegerToFloat64();
    puts("floating_width_conversions_tests: ok");
    return 0;
}
