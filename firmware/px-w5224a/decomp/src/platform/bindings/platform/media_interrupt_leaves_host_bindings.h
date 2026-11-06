#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)

#include "platform/firmware_memory.h"
#include "platform/media_interrupt_leaves.h"
#include "platform/timer_debounce.h"

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteP3Bit(u8 bit_mask, u8 value)
{
    u8 p3 = FirmwareReadDirectSfr8(B0);

    if(value != 0u)
        p3 = (u8)(p3 | bit_mask);
    else
        p3 = (u8)(p3 & (u8)~bit_mask);
    FirmwareWriteDirectSfr8(B0, p3);
}

#endif
#endif
