#include "../platform/leaf_workers.h"
#include "vector_entries.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/boot/sfr_e1_bit7_interrupt_entry_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f80123 */
void SfrE1Bit7InterruptBody(void) PLEXTOR_INTERRUPT(4)
{
    u8 interrupted_psw = VectorEntryPsw;

    HandleSfrE1Bit7Interrupt();
    VectorEntryPsw1 = interrupted_psw;
}
