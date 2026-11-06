#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveTimerPhase2Bit003E(void);
#define READ_BIT003E() ObserveTimerPhase2Bit003E()
#define WRITE_BIT003D(value) FirmwareWriteBit003D(value)
#define WRITE_BIT003E(value) FirmwareWriteBit003E(value)
#define READ_P3_BIT0() FirmwareReadBit0580()
#define READ_P3_BIT1() FirmwareReadBit0581()
#define WRITE_P3_BIT0(value) FirmwareWriteBit0580(value)
#define WRITE_P3_BIT1(value) FirmwareWriteBit0581(value)
#else
u8 bdata TimerPhase2BitByte27 _at_ 0x27;
sbit TimerPhase2Bit003D = TimerPhase2BitByte27 ^ 5;
sbit TimerPhase2Bit003E = TimerPhase2BitByte27 ^ 6;
sbit TimerPhase2P3Bit0 = 0xb0;
sbit TimerPhase2P3Bit1 = 0xb1;
#define READ_BIT003E() TimerPhase2Bit003E
#define WRITE_BIT003D(value) (TimerPhase2Bit003D = (value))
#define WRITE_BIT003E(value) (TimerPhase2Bit003E = (value))
#define READ_P3_BIT0() TimerPhase2P3Bit0
#define READ_P3_BIT1() TimerPhase2P3Bit1
#define WRITE_P3_BIT0(value) (TimerPhase2P3Bit0 = (value))
#define WRITE_P3_BIT1(value) (TimerPhase2P3Bit1 = (value))
#endif
#endif
