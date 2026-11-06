#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadAcquisitionConfigBit0033(void);
extern u8 ObserveReadAcquisitionConfigBit0042(void);
extern void ObserveReadAcquisitionConfigBit0043(u8 value);
extern void ObserveReadAcquisitionConfigInterrupt(u8 event);
extern void ObserveReadAcquisitionConfigRegister01(u8 value);
#define CONFIG_READ_BIT33() ObserveReadAcquisitionConfigBit0033()
#define CONFIG_READ_BIT42() ObserveReadAcquisitionConfigBit0042()
#define CONFIG_WRITE_BIT43(value) ObserveReadAcquisitionConfigBit0043(value)
#define CONFIG_INTERRUPT(event) ObserveReadAcquisitionConfigInterrupt(event)
#define CONFIG_WRITE_REGISTER01(value) ObserveReadAcquisitionConfigRegister01(value)
#else
u8 bdata ReadAcquisitionConfigBitByte26 _at_ 0x26;
u8 bdata ReadAcquisitionConfigBitByte28 _at_ 0x28;
sbit ReadAcquisitionConfigBit0033 = ReadAcquisitionConfigBitByte26 ^ 3;
sbit ReadAcquisitionConfigBit0042 = ReadAcquisitionConfigBitByte28 ^ 2;
sbit ReadAcquisitionConfigBit0043 = ReadAcquisitionConfigBitByte28 ^ 3;
sbit ReadAcquisitionConfigGlobalInterruptEnable = 0xaf;
sbit ReadAcquisitionConfigControllerInterruptEnable = 0xae;
extern void SetSfrA9Bit2Atomic(void);
#define CONFIG_READ_BIT33() ((u8)ReadAcquisitionConfigBit0033)
#define CONFIG_READ_BIT42() ((u8)ReadAcquisitionConfigBit0042)
#define CONFIG_WRITE_BIT43(value) (ReadAcquisitionConfigBit0043 = (value))
#define CONFIG_INTERRUPT(event) ConfigureReadAcquisitionInterrupt(event)
#define CONFIG_WRITE_REGISTER01(value) WriteControllerRegister01Clamped(value)

static void ConfigureReadAcquisitionInterrupt(u8 event)
{
    if(event == 0u)
        ReadAcquisitionConfigGlobalInterruptEnable = 0;
    else if(event == 1u)
        ReadAcquisitionConfigGlobalInterruptEnable = 1;
    else if(event == 2u)
        ReadAcquisitionConfigControllerInterruptEnable = 1;
    else
        SetSfrA9Bit2Atomic();
}
#endif
#endif
