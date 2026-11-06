#include "../platform/timing.h"
#include "vector_entries.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/boot/timer0_interrupt_entry_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f800f6 */
void Timer0InterruptBody(void) PLEXTOR_INTERRUPT(3)
{
    Timer0TickHandler();
}
