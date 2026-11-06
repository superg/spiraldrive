#include "floating_floor.h"

typedef union
{
    double value;
    u8 bytes[8];
} Float64Representation;

static u8 ReadFloat64Byte(const Float64Representation *value, u8 index)
{
#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/floating_floor_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteFloat64Byte(Float64Representation *value, u8 index, u8 byte)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/floating_floor_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 IsCanonicalFloat64Nan(const Float64Representation *value)
{
    u8 index;

    for(index = 0u; index != 8u; ++index)
        if(ReadFloat64Byte(value, index) != 0xffu)
            return 0u;
    return 1u;
}

/* RAM:f84a14 */
double FloorFloat64(double value)
{
    Float64Representation result;
    u16 exponent;
    s16 integer_exponent;
    u8 fractional_bits;
    u8 whole_bytes;
    u8 remaining_bits;
    u8 index;
    u8 discarded = 0u;

    result.value = value;
    if(IsCanonicalFloat64Nan(&result) != 0u)
        return -1.0;
    exponent = (u16)(((u16)(ReadFloat64Byte(&result, 0u) & 0x7fu) << 4) | (ReadFloat64Byte(&result, 1u) >> 4));
    if(exponent == 0x07ffu)
        return value;

    integer_exponent = (s16)exponent - 1023;
    if(integer_exponent < 0)
    {
        if((ReadFloat64Byte(&result, 0u) & 0x80u) != 0u && (ReadFloat64Byte(&result, 0u) & 0x7fu) != 0u)
            return -1.0;
        for(index = 1u; index != 8u; ++index)
            if(ReadFloat64Byte(&result, index) != 0u)
                return (ReadFloat64Byte(&result, 0u) & 0x80u) != 0u ? -1.0 : 0.0;
        return value;
    }
    if(integer_exponent >= 52)
        return value;

    fractional_bits = (u8)(52 - integer_exponent);
    whole_bytes = (u8)(fractional_bits >> 3);
    remaining_bits = (u8)(fractional_bits & 7u);
    for(index = 0u; index != whole_bytes; ++index)
    {
        u8 byte_index = (u8)(7u - index);

        discarded |= ReadFloat64Byte(&result, byte_index);
        WriteFloat64Byte(&result, byte_index, 0u);
    }
    if(remaining_bits != 0u)
    {
        u8 byte_index = (u8)(7u - whole_bytes);
        u8 mask = (u8)(0xffu << remaining_bits);
        u8 byte = ReadFloat64Byte(&result, byte_index);

        discarded |= (u8)(byte & (u8)~mask);
        WriteFloat64Byte(&result, byte_index, (u8)(byte & mask));
    }
    if((ReadFloat64Byte(&result, 0u) & 0x80u) != 0u && discarded != 0u)
        result.value -= 1.0;
    return result.value;
}
