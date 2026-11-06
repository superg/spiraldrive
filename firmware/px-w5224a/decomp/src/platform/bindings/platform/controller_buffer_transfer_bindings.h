#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveControllerBufferTransferCompletion(void);
#define CONTROLLER_TRANSFER_COMPLETE() ObserveControllerBufferTransferCompletion()
#else
#define CONTROLLER_TRANSFER_COMPLETE() FirmwareRead8(FW_HUGE_UNKNOWN_2BA5C0)
#endif
#endif
