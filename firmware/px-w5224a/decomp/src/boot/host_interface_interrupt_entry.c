#include "../platform/firmware_entry.h"
#include "../platform/firmware_memory.h"
#include "../platform/host_interface_interrupt.h"
#include "vector_entries.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/boot/host_interface_interrupt_entry_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f8007f */
void HostInterfaceInterruptBody(void) PLEXTOR_INTERRUPT(1)
{
    HandleHostInterfaceInterrupt();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FA) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FA, 0u);
        HostInterruptRestartAfterReti();
    }
}
