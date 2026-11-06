#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDiscReadPositionSetP1TimerPins(void);
#define SET_READ_POSITION_P1_TIMER_PINS() ObserveDiscReadPositionSetP1TimerPins()
#else
sbit DiscReadPositionP1Bit0 = 0x90;
sbit DiscReadPositionP1Bit1 = 0x91;
#define SET_READ_POSITION_P1_TIMER_PINS()                                                                                                                                                              \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        DiscReadPositionP1Bit1 = 1;                                                                                                                                                                    \
        DiscReadPositionP1Bit0 = 1;                                                                                                                                                                    \
    } while(0)
#endif
#endif
