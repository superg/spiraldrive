#include "../platform/controller_event_dispatch.h"
#include "vector_entries.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/boot/controller_event_interrupt_entry_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f80054 */
void ControllerEventInterruptBody(void) PLEXTOR_INTERRUPT(0)
{
    u8 saved_psw1 = VectorEntryPsw1;

    VectorEntryPsw1 = (u8)(saved_psw1 | 0x08u);
    HandleControllerEventInterrupt();
    VectorEntryPsw1 = (u8)(saved_psw1 & 0xf7u);
}
