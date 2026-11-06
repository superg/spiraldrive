#include "power_control.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/power_control_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fee8a8 */
void EnterPowerDownModeWithInterruptsEnabled(void)
{
    PowerDownEnableInterrupts();
    PowerDownSetMode();
    PowerDownNop();
    PowerDownNop();
}
