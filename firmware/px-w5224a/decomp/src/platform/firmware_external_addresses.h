#ifndef PLEXTOR_FIRMWARE_EXTERNAL_ADDRESSES_H
#define PLEXTOR_FIRMWARE_EXTERNAL_ADDRESSES_H

#include "mcs251.h"

/* Fixed read/execution alias for the persistent 0x60-byte drive-configuration
 * record. Runtime operation 5 programs its corresponding flash bank. */
#define PLEXTOR_CONFIGURATION_FLASH_ADDRESS(canonical) ((fw_addr_t)(canonical))

/* Fixed entry in the separately captured RAM:ff0000 runtime-monitor overlay.
 * These addresses are ROM contracts and must not follow the main-image link. */
#define PLEXTOR_RUNTIME_MONITOR_ADDRESS(canonical) ((fw_addr_t)(canonical))

#endif
