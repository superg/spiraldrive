typedef unsigned char u8;

u8 near MapThreeFieldStatusCode(u8 flags, u8 field_r7, u8 field_r6)
{
    switch(flags & 0x40u)
    {
    case 0u:
        return 0u;
    default:
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
        }
    }
    return 0u;
}
