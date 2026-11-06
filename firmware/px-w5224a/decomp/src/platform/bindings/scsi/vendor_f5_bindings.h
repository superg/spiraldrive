#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t local_response_address = FW_NEAR_UNKNOWN_7000;
#else
u8 local_response[2];
fw_addr_t local_response_address = (fw_addr_t)local_response;
#endif
#endif
