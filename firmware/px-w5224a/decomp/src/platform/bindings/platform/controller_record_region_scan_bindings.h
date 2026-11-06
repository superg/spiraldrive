#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCopyControllerRecordRegionPage(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker);
#define COPY_CONTROLLER_RECORD_REGION_PAGE ObserveCopyControllerRecordRegionPage
#else
extern void CopyControllerBufferPageToUpperBankedRegionForC(fw_addr_t source_address, u8 bank_index, u8 page_index, u8 write_marker);
#define COPY_CONTROLLER_RECORD_REGION_PAGE CopyControllerBufferPageToUpperBankedRegionForC
#endif
#endif
