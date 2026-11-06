#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDisableGlobalInterrupts(void);
extern void ObserveEnableFirmwareInterruptSources(void);

void DisableGlobalInterrupts(void)
{
    ObserveDisableGlobalInterrupts();
}

void EnableFirmwareInterruptSources(void)
{
    ObserveEnableFirmwareInterruptSources();
}
#else
sbit GlobalInterruptEnable = 0xaf;
sbit ControllerInterruptEnable = 0xae;
extern void SetSfrA9Bit2Atomic(void);

/* RAM:fee80e */
void DisableGlobalInterrupts(void)
{
    GlobalInterruptEnable = 0;
}

/* RAM:fee811 */
void EnableFirmwareInterruptSources(void)
{
    GlobalInterruptEnable = 1;
    ControllerInterruptEnable = 1;
    SetSfrA9Bit2Atomic();
}
#endif
#endif
