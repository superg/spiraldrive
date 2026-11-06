#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 FirmwareReadBit0023(void);
#define READ_HOST_TRANSFER_BIT() FirmwareReadBit0023()
#define READ_CONTROLLER_QUEUE_BIT() FirmwareReadBit0008()
#define SET_RECOVERY_INTERRUPT_PINS()                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteBit0481(1u);                                                                                                                                                                      \
        FirmwareWriteBit0480(1u);                                                                                                                                                                      \
        ObserveSetExternalInterrupt0Enable();                                                                                                                                                          \
    } while(0)
#define SET_RECOVERY_P3_PATTERN()                                                                                                                                                                      \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteBit0580(1u);                                                                                                                                                                      \
        FirmwareWriteBit0581(0u);                                                                                                                                                                      \
    } while(0)
extern void ObserveSetExternalInterrupt0Enable(void);
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0023Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetMediaState0P1TimerPinsHighAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetExternalInterrupt0EnableAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetMediaState0IndexedP3PatternAtomic(void);
#define READ_HOST_TRANSFER_BIT() ReadBit0023Atomic()
#define READ_CONTROLLER_QUEUE_BIT() FirmwareReadBit0008()
#define SET_RECOVERY_INTERRUPT_PINS()                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        SetMediaState0P1TimerPinsHighAtomic();                                                                                                                                                         \
        SetExternalInterrupt0EnableAtomic();                                                                                                                                                           \
    } while(0)
#define SET_RECOVERY_P3_PATTERN() SetMediaState0IndexedP3PatternAtomic()
#endif
#endif
