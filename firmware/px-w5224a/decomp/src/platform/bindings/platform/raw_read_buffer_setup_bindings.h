#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawReadControllerBufferFill(fw_addr_t buffer_address, u32 byte_count, u8 value);
#define FILL_RAW_READ_BUFFER(address, count, value) ObserveRawReadControllerBufferFill((address), (count), (value))
#else
#define FILL_RAW_READ_BUFFER(address, count, value) FillControllerBufferRange((address), (count), (value))
#endif
#endif
