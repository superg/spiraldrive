typedef unsigned char u8;

u8 near MapThreeFieldStatusCode(u8 flags, u8 field_r7, u8 field_r6)
{
    if((flags & 0x40u) == 0u)
        return 0u;
    if(field_r7 == 0u)
        return 0xffu;
    if(field_r7 == 1u)
        return 8u;
    if(field_r7 != 2u)
        return 0u;
    if(field_r6 == 8u)
        return 12u;
    if(field_r6 == 16u)
        return 10u;
    if(field_r6 == 32u)
        return 9u;
    return 0u;
}
