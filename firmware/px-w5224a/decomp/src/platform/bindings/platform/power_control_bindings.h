#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#include "platform/firmware_memory.h"
extern void ObservePowerDownEvent(u8 event);
#define PowerDownEnableInterrupts() ObservePowerDownEvent(1u)
#define PowerDownSetMode() ObservePowerDownEvent(2u)
#define PowerDownNop() ObservePowerDownEvent(3u)
#else
#include <intrins.h>
sbit PowerDownGlobalInterruptEnable = 0xaf;
sfr PowerControlRegister = 0x87;
#define PowerDownEnableInterrupts()                                                                                                                                                                    \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        PowerDownGlobalInterruptEnable = 1;                                                                                                                                                            \
    } while(0)
#define PowerDownSetMode()                                                                                                                                                                             \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        PowerControlRegister |= 0x02u;                                                                                                                                                                 \
    } while(0)
#define PowerDownNop() _nop_()
#endif
#endif
