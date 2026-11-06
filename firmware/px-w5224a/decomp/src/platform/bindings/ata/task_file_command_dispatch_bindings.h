#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define CLEAR_GLOBAL_INTERRUPT() FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) & 0x7fu))
#define SET_GLOBAL_INTERRUPT() FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) | 0x80u))
#define SET_CONTROLLER_INTERRUPT() FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) | 0x40u))
#define SET_SFR_A9_BIT2() FirmwareWriteDirectSfr8(A9, (u8)(FirmwareReadDirectSfr8(A9) | 0x04u))
#define SET_P3_BITS0_AND1()                                                                                                                                                                            \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) | 0x01u));                                                                                                                         \
        FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) | 0x02u));                                                                                                                         \
    } while(0)
#define CLEAR_P1_TIMER_PINS()                                                                                                                                                                          \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteDirectSfr8(90, (u8)(FirmwareReadDirectSfr8(90) & 0xfdu));                                                                                                                         \
        FirmwareWriteDirectSfr8(90, (u8)(FirmwareReadDirectSfr8(90) & 0xfeu));                                                                                                                         \
    } while(0)
#define ENABLE_TIMER0_INTERRUPT() FirmwareWriteDirectSfr8(A8, (u8)(FirmwareReadDirectSfr8(A8) | 0x02u))
#else
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
extern void SetP3Bits0And1HighForTimedOutputSequence(void);
extern void ClearP1TimerPinsForPremiumPayload(void);
extern void SetTimer0InterruptEnableAtomic(void);
#define CLEAR_GLOBAL_INTERRUPT() ClearBit0547Atomic()
#define SET_GLOBAL_INTERRUPT() SetBit0547Atomic()
#define SET_CONTROLLER_INTERRUPT() SetBit0546Atomic()
#define SET_SFR_A9_BIT2() SetSfrA9Bit2Atomic()
#define SET_P3_BITS0_AND1() SetP3Bits0And1HighForTimedOutputSequence()
#define CLEAR_P1_TIMER_PINS() ClearP1TimerPinsForPremiumPayload()
#define ENABLE_TIMER0_INTERRUPT() SetTimer0InterruptEnableAtomic()
#endif
#endif
