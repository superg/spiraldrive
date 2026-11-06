#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadCdEngineBit(u8 bit_address);
extern void ObserveWriteReadCdEngineBit(u8 bit_address, u8 value);
#define READ_CD_BIT(address) ObserveReadCdEngineBit((address))
#define WRITE_READ_CD_BIT(address, value) ObserveWriteReadCdEngineBit((address), (value))
#else
u8 bdata ReadCdEngineBitByte21 _at_ 0x21;
u8 bdata ReadCdEngineBitByte22 _at_ 0x22;
u8 bdata ReadCdEngineBitByte23 _at_ 0x23;
u8 bdata ReadCdEngineBitByte24 _at_ 0x24;
sbit ReadCdEngineBit000D = ReadCdEngineBitByte21 ^ 5;
sbit ReadCdEngineBit000E = ReadCdEngineBitByte21 ^ 6;
sbit ReadCdEngineBit0010 = ReadCdEngineBitByte22 ^ 0;
sbit ReadCdEngineBit0011 = ReadCdEngineBitByte22 ^ 1;
sbit ReadCdEngineBit0013 = ReadCdEngineBitByte22 ^ 3;
sbit ReadCdEngineBit0018 = ReadCdEngineBitByte23 ^ 0;
sbit ReadCdEngineBit001C = ReadCdEngineBitByte23 ^ 4;
sbit ReadCdEngineBit001D = ReadCdEngineBitByte23 ^ 5;
sbit ReadCdEngineBit0021 = ReadCdEngineBitByte24 ^ 1;
static u8 ReadReadCdEngineBit(u8 bit_address)
{
    if(bit_address == 0x0du)
        return (u8)ReadCdEngineBit000D;
    if(bit_address == 0x10u)
        return (u8)ReadCdEngineBit0010;
    if(bit_address == 0x11u)
        return (u8)ReadCdEngineBit0011;
    if(bit_address == 0x1cu)
        return (u8)ReadCdEngineBit001C;
    if(bit_address == 0x1du)
        return (u8)ReadCdEngineBit001D;
    return (u8)ReadCdEngineBit0021;
}
static void WriteReadCdEngineBit(u8 bit_address, u8 value)
{
    if(bit_address == 0x0du)
        ReadCdEngineBit000D = value;
    else if(bit_address == 0x0eu)
        ReadCdEngineBit000E = value;
    else if(bit_address == 0x10u)
        ReadCdEngineBit0010 = value;
    else if(bit_address == 0x13u)
        ReadCdEngineBit0013 = value;
    else if(bit_address == 0x18u)
        ReadCdEngineBit0018 = value;
    else if(bit_address == 0x1cu)
        ReadCdEngineBit001C = value;
    else if(bit_address == 0x1du)
        ReadCdEngineBit001D = value;
    else
        ReadCdEngineBit0021 = value;
}
#define READ_CD_BIT(address) ReadReadCdEngineBit((address))
#define WRITE_READ_CD_BIT(address, value) WriteReadCdEngineBit((address), (value))
#endif
#endif
