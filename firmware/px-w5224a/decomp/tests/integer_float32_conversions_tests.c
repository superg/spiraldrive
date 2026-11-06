#include <assert.h>
#include <stdio.h>
#include "platform/integer_float32_conversions.h"

typedef union
{
    float value;
    u32 bits;
} HostIntegerFloat32Bits;

static float FromBits(u32 bits)
{
    HostIntegerFloat32Bits representation;

    representation.bits = bits;
    return representation.value;
}

static u32 ToBits(float value)
{
    HostIntegerFloat32Bits representation;

    representation.value = value;
    return representation.bits;
}

static void TestInteger8Conversion(void)
{
    assert(ToBits(ConvertInteger8ToFloat32(0)) == 0u);
    assert(ToBits(ConvertInteger8ToFloat32(127)) == 0x42fe0000ul);
    assert(ToBits(ConvertInteger8ToFloat32(-128)) == 0xc3000000ul);
}

static void TestInteger16Conversion(void)
{
    assert(ToBits(ConvertInteger16ToFloat32(0)) == 0u);
    assert(ToBits(ConvertInteger16ToFloat32(32767)) == 0x46fffe00ul);
    assert(ToBits(ConvertInteger16ToFloat32(-32768)) == 0xc7000000ul);
}

static void TestInteger32Conversion(void)
{
    assert(ToBits(ConvertInteger32ToFloat32(0)) == 0u);
    assert(ToBits(ConvertInteger32ToFloat32(16777217l)) == 0x4b800000ul);
    assert(ToBits(ConvertInteger32ToFloat32(2147483647l)) == 0x4f000000ul);
    assert(ToBits(ConvertInteger32ToFloat32((-2147483647l - 1l))) == 0xcf000000ul);
}

static void TestFloat32ToInteger32(void)
{
    assert(ConvertFloat32ToInteger32(0.75f) == 0);
    assert(ConvertFloat32ToInteger32(1.75f) == 1);
    assert(ConvertFloat32ToInteger32(-1.75f) == -1);
    assert((u32)ConvertFloat32ToInteger32(2147483648.0f) == 0x80000000ul);
    assert((u32)ConvertFloat32ToInteger32(4294967040.0f) == 0xffffff00ul);
    assert((u32)ConvertFloat32ToInteger32(4294967296.0f) == 0xfffffffful);
    assert((u32)ConvertFloat32ToInteger32(FromBits(0x7f800000ul)) == 0xfffffffful);
    assert(ConvertFloat32ToInteger32(FromBits(0x00000001ul)) == 0);
}

int main(void)
{
    TestInteger8Conversion();
    TestInteger16Conversion();
    TestInteger32Conversion();
    TestFloat32ToInteger32();
    puts("integer_float32_conversions_tests: ok");
    return 0;
}
