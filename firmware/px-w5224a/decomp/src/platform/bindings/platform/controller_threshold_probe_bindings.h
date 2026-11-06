#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveControllerThresholdProbeBit31(void);
#define READ_CONTROLLER_THRESHOLD_BIT31() ObserveControllerThresholdProbeBit31()
#else
u8 bdata ControllerThresholdProbeBitByte26 _at_ 0x26;
sbit ControllerThresholdProbeBit0031 = ControllerThresholdProbeBitByte26 ^ 1;
#define READ_CONTROLLER_THRESHOLD_BIT31() ((u8)ControllerThresholdProbeBit0031)
#endif
#endif
