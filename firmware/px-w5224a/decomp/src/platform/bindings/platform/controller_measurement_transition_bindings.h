#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSetControllerTransitionP1Bits(void);
#define SET_CONTROLLER_TRANSITION_P1_BITS() ObserveSetControllerTransitionP1Bits()
#else
sbit ControllerTransitionP1Bit0 = 0x90;
sbit ControllerTransitionP1Bit1 = 0x91;
#define SET_CONTROLLER_TRANSITION_P1_BITS()                                                                                                                                                            \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ControllerTransitionP1Bit1 = 1;                                                                                                                                                                \
        ControllerTransitionP1Bit0 = 1;                                                                                                                                                                \
    } while(0)
#endif
#endif
