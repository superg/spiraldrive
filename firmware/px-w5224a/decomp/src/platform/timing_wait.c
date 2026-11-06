#include "arithmetic.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/timing_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f8f127 */
void WaitTimer1ScaledDuration(u16 duration_units, u8 yield_while_waiting)
{
    u32 remaining = MultiplyU32Low((u32)duration_units, 9ul);

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/timing_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    do
    {
        u16 chunk;
        u16 reload_value;

        if(remaining > 0xfffful)
        {
            chunk = 0xffffu;
            remaining -= 0xfffful;
        }
        else
        {
            chunk = (u16)remaining;
            remaining = 0ul;
        }
        reload_value = (u16)(0xffffu - chunk);
#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/timing_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        FirmwareWriteDirectSfr8(8D, (u8)(reload_value >> 8));
        FirmwareWriteDirectSfr8(8B, (u8)reload_value);
        Timer1ScaledClearOverflow();
        Timer1ScaledSetRun();
        while(Timer1ScaledOverflow() == 0u)
            if(yield_while_waiting == 1u)
                Timer1ScaledYield();
        Timer1ScaledClearRun();
        Timer1ScaledClearOverflow();
    } while(remaining != 0ul);
}
