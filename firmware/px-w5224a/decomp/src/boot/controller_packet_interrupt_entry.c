#include "../platform/eight_channel_capture.h"
#include "vector_entries.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/boot/controller_packet_interrupt_entry_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f800cb */
void ControllerPacketInterruptBody(void) PLEXTOR_INTERRUPT(2)
{
    u8 saved_psw1 = VectorEntryPsw1;

    VectorEntryPsw1 = (u8)(saved_psw1 | 0x10u);
    HandleControllerPacketInterrupt();
    VectorEntryPsw1 = (u8)(saved_psw1 & 0xefu);
}
