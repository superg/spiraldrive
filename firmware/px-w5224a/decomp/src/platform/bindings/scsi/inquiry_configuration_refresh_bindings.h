#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRefreshInquiryResponseEntry(void);
extern void ObserveInquiryConfigurationRead(fw_addr_t offset, fw_addr_t destination, u16 count);
#define READ_CONFIGURATION(offset, destination, count) ObserveInquiryConfigurationRead((offset), (destination), (count))
#else
#define READ_CONFIGURATION(offset, destination, count) ReadDriveConfigurationBytes((offset), (destination), (count))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t configuration_address = FW_NEAR_UNKNOWN_7000;
fw_addr_t validity_address = FW_NEAR_UNKNOWN_7020;

if(ObserveRefreshInquiryResponseEntry() == 0u)
    return;
#else
u8 configuration[32];
u8 validity;
fw_addr_t configuration_address = (fw_addr_t)configuration;
fw_addr_t validity_address = (fw_addr_t)&validity;
#endif
#endif
