#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "platform/arithmetic.h"
#include "platform/firmware_memory.h"

u8 FirmwareRead8(fw_addr_t address)
{
    (void)address;
    return 0u;
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    (void)address;
    (void)value;
}

u16 ReadBigEndianU16(fw_addr_t address)
{
    (void)address;
    return 0u;
}

void WriteBigEndianU16(fw_addr_t address, u16 value)
{
    (void)address;
    (void)value;
}

static float AbsoluteFloat(float value)
{
    return value < 0.0f ? -value : value;
}

static void AssertLogClose(float value, float tolerance)
{
    float actual = NaturalLogFloat32(value);
    float expected = logf(value);

    assert(AbsoluteFloat(actual - expected) <= tolerance);
}

static void TestRepresentativePositiveDomain(void)
{
    static const float values[] = { 1.0f / 32767.0f, 0.001f, 0.25f, 0.5f, 1.0f, 1.4142135f, 1.4142137f, 2.0f, 31.75f, 65535.0f };
    u8 index;

    for(index = 0u; index != sizeof(values) / sizeof(values[0]); ++index)
        AssertLogClose(values[index], 0.000002f);
    assert(NaturalLogFloat32(1.0f) == 0.0f);
}

static void TestExceptionalAndSubnormalValues(void)
{
    union
    {
        float value;
        u32 bits;
    } operand;
    float result;

    operand.bits = 1ul;
    AssertLogClose(operand.value, 0.00002f);

    result = NaturalLogFloat32(0.0f);
    operand.value = result;
    assert(operand.bits == 0xc08bbbaaul);
    result = NaturalLogFloat32(-1.0f);
    assert(isnan(result));
    operand.bits = 0x7f800000ul;
    assert(NaturalLogFloat32(operand.value) == operand.value);
}

static void TestSoleCallerScale(void)
{
    static const u16 values[] = { 1u, 17u, 1024u, 16384u, 32767u, 65535u };
    u8 index;

    for(index = 0u; index != sizeof(values) / sizeof(values[0]); ++index)
    {
        float normalized = (float)values[index] / 32767.0f;
        s32 expected = (s32)(2000.0f * (logf(normalized) / 2.3025851f));

        assert(Compute2000TimesLog10OfWordOver32767(values[index]) == expected);
    }
    assert(Compute2000TimesLog10OfWordOver32767(0u) == -3792l);
}

int main(void)
{
    TestRepresentativePositiveDomain();
    TestExceptionalAndSubnormalValues();
    TestSoleCallerScale();
    puts("natural log float32 tests passed");
    return 0;
}
