#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadControllerEventBit43(void);
extern void ObserveClearControllerEventBit43(void);
#define READ_CONTROLLER_EVENT_BIT43() ObserveReadControllerEventBit43()
#define CLEAR_CONTROLLER_EVENT_BIT43() ObserveClearControllerEventBit43()
#else
u8 bdata ControllerEventBitByte28 _at_ 0x28;
sbit ControllerEventBit43 = ControllerEventBitByte28 ^ 3;
#define READ_CONTROLLER_EVENT_BIT43() ((u8)ControllerEventBit43)
#define CLEAR_CONTROLLER_EVENT_BIT43() (ControllerEventBit43 = 0)
#endif
#endif
