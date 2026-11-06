#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
static void SetWriteBufferBit(u16 bit_address, u8 value)
{
    if(bit_address == 0x0480u)
        FirmwareWriteBit0480(value);
    else if(bit_address == 0x0481u)
        FirmwareWriteBit0481(value);
    else if(bit_address == 0x0546u)
        FirmwareWriteBit0546(value);
    else if(bit_address == 0x0547u)
        FirmwareWriteBit0547(value);
    else if(bit_address == 0x0580u)
        FirmwareWriteBit0580(value);
    else
        FirmwareWriteBit0581(value);
}
#define WRITE_BUFFER_SET_A9_BIT2() FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 4u))
#else
sbit WriteBufferP1Bit0 = 0x90;
sbit WriteBufferP1Bit1 = 0x91;
sbit WriteBufferControllerInterruptEnable = 0xae;
sbit WriteBufferGlobalInterruptEnable = 0xaf;
extern void SetSfrA9Bit2Atomic(void);
sbit WriteBufferP3Rx = 0xb0;
sbit WriteBufferP3Tx = 0xb1;
static void SetWriteBufferBit(u16 bit_address, u8 value)
{
    if(bit_address == 0x0480u)
        WriteBufferP1Bit0 = value;
    else if(bit_address == 0x0481u)
        WriteBufferP1Bit1 = value;
    else if(bit_address == 0x0546u)
        WriteBufferControllerInterruptEnable = value;
    else if(bit_address == 0x0547u)
        WriteBufferGlobalInterruptEnable = value;
    else if(bit_address == 0x0580u)
        WriteBufferP3Rx = value;
    else
        WriteBufferP3Tx = value;
}
#define WRITE_BUFFER_SET_A9_BIT2() SetSfrA9Bit2Atomic()
#endif
#endif
