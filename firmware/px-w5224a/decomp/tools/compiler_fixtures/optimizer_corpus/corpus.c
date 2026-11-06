typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned long u32;

s8 DecodeControllerEncodedSignedOffset(u8 encoded_offset)
{
    if(encoded_offset == 0xffu)
        return 0;
    if((encoded_offset & 0x80u) != 0u)
        return (s8)(128 - (s16)encoded_offset);
    return (s8)encoded_offset;
}

u8 PackedBcdByteToBinary(u8 value)
{
    return (u8)(((value >> 4) * 10u) + (value & 0x0fu));
}

u8 BinaryByteToPackedBcd(u8 value)
{
    return (u8)(value + ((value / 10u) * 6u));
}

u8 near DecodeAsciiHexNibbleOrZero(u8 character)
{
    if(character >= '0' && character <= '9')
        return (u8)(character - '0');
    if(character >= 'A' && character <= 'F')
        return (u8)(character - ('A' - 10));
    if(character >= 'a' && character <= 'f')
        return (u8)(character - ('a' - 10));
    return 0;
}

u16 near DecodeUppercaseAsciiHexNibbleOrZero(u8 character)
{
    if(character >= 'A' && character <= 'F')
        return (u16)(character - ('A' - 10));
    if(character >= '0' && character <= '9')
        return (u16)(character - '0');
    return 0;
}

u8 EncodeSignedByteAsSignMagnitude(s8 value)
{
    if(value < 0)
        return (u8)((u8)(0u - (u8)value) | 0x80u);
    return (u8)value;
}

u8 MinU8(u8 value, u8 limit)
{
    return value <= limit ? value : limit;
}

u16 MinU16(u16 value, u16 limit)
{
    return value <= limit ? value : limit;
}

u32 MinU32(u32 value, u32 limit)
{
    return value <= limit ? value : limit;
}

s16 AbsoluteValueI16(s16 value)
{
    if(value >= 0)
        return value;
    return (s16)(u16)(0u - (u16)value);
}

u8 MapOperatingCodeToProfileIndex(u8 operating_code)
{
    if(operating_code == 0u)
        return 0u;
    if(operating_code < 2u)
        return 1u;
    if(operating_code < 4u)
        return 2u;
    if(operating_code < 8u)
        return 3u;
    if(operating_code < 10u)
        return 4u;
    if(operating_code < 12u)
        return 5u;
    if(operating_code < 16u)
        return 6u;
    if(operating_code < 20u)
        return 7u;
    if(operating_code < 24u)
        return 8u;
    if(operating_code < 32u)
        return 9u;
    if(operating_code < 40u)
        return 10u;
    if(operating_code < 48u)
        return 11u;
    return 12u;
}

u8 MapOperatingCodeToTransitionClass(u8 operating_code)
{
    if(operating_code < 4u)
        return 0u;
    if(operating_code < 8u)
        return 1u;
    if(operating_code < 0x18u)
        return 2u;
    if(operating_code < 0x20u)
        return 3u;
    if(operating_code < 0x28u)
        return 4u;
    return 5u;
}

u8 near MapControllerRegisterIndexSkippingGap(u8 index)
{
    if(index > 0x0fu)
        index = (u8)(index + 5u);
    if(index > 0x1au)
        index = 0x1au;
    return index;
}

u8 near MapThreeFieldStatusCode(u8 flags, u8 field_r7, u8 field_r6)
{
    if((flags & 0x40u) != 0u)
    {
        switch(field_r7)
        {
        case 0u:
            return 0xffu;
        case 1u:
            return 8u;
        case 2u:
            switch(field_r6)
            {
            case 8u:
                return 12u;
            case 16u:
                return 10u;
            case 32u:
                return 9u;
            }
            break;
        }
    }
    return 0u;
}

u32 near PowerOfTenU32(u8 exponent)
{
    u32 value = 1ul;

    while(exponent != 0u)
    {
        value = (value << 3) + (value << 1);
        exponent--;
    }
    return value;
}

u32 IntegerSquareRootU32(u32 value)
{
    u32 root = 0ul;
    u32 trial_bit = 0x40000000ul;

    while(trial_bit > value)
        trial_bit >>= 2;
    while(trial_bit != 0ul)
    {
        if(value >= root + trial_bit)
        {
            value -= root + trial_bit;
            root = (root >> 1) + trial_bit;
        }
        else
        {
            root >>= 1;
        }
        trial_bit >>= 2;
    }
    return root;
}

u8 near ToggleR3Bit7IfSignedR3Negative(s8 value)
{
    u8 result = (u8)value;

    if(value < 0)
        result ^= 0x80u;
    return result;
}

u8 near ToggleR3Bit7IfSignedWR2Negative(s16 value)
{
    u8 result = (u8)value;

    if(value < 0)
        result ^= 0x80u;
    return result;
}

u8 near ToggleR3Bit7IfSignedDR0Negative(signed long value)
{
    u8 result = (u8)value;

    if(value < 0)
        result ^= 0x80u;
    return result;
}
