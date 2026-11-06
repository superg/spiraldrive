#include "buffer_transfer_setup.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/buffer_transfer_setup_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd2df3 */
void PLEXTOR_NEAR_FUNCTION PrepareBufferDataTransfer(void)
{
    RESET_BUFFER_TRANSFER_QUEUE();
    INITIALIZE_BUFFER_TRANSFER_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) >> 2) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 3u)
        DISPATCH_BUFFER_TRANSFER_ACQUISITION();
    YIELD_BUFFER_TRANSFER_COROUTINE0();
}
