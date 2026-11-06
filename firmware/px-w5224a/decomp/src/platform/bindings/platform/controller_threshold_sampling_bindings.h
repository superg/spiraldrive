#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveControllerThresholdSamplingInterrupt(u8 event);
#define THRESHOLD_SAMPLING_INTERRUPT(event) ObserveControllerThresholdSamplingInterrupt(event)
#else
sbit ControllerThresholdSamplingGlobalInterrupt = 0xaf;
sbit ControllerThresholdSamplingControllerInterrupt = 0xae;
extern void SetSfrA9Bit2Atomic(void);
static void ConfigureControllerThresholdSamplingInterrupt(u8 event)
{
    if(event == 0u)
        ControllerThresholdSamplingGlobalInterrupt = 0;
    else if(event == 1u)
        ControllerThresholdSamplingGlobalInterrupt = 1;
    else if(event == 2u)
        ControllerThresholdSamplingControllerInterrupt = 1;
    else
        SetSfrA9Bit2Atomic();
}
#define THRESHOLD_SAMPLING_INTERRUPT(event) ConfigureControllerThresholdSamplingInterrupt(event)
#endif
#endif
