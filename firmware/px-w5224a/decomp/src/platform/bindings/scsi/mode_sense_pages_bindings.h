#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveModeSensePagePayloadLength(u8 page_code);
extern fw_addr_t ObserveModeSensePageImageSelection(u8 page_code, u8 page_control);
extern void ObserveModeSensePage1ARefresh(void);
extern void ObserveModeSensePage2ARefresh(void);
extern u32 ObserveModeSensePageCopy(fw_addr_t source_address, u32 response_address, u32 byte_count);
#define MODE_SENSE_PAGE_LENGTH(page) ObserveModeSensePagePayloadLength(page)
#define SELECT_MODE_SENSE_PAGE_IMAGE(page, control) ObserveModeSensePageImageSelection((page), (control))
#define REFRESH_MODE_SENSE_PAGE_1A() ObserveModeSensePage1ARefresh()
#define REFRESH_MODE_SENSE_PAGE_2A() ObserveModeSensePage2ARefresh()
#define COPY_MODE_SENSE_PAGE(source, response, count) ObserveModeSensePageCopy((source), (response), (count))
#else
extern void PLEXTOR_NEAR_FUNCTION UpdateModePage2ACapabilitiesRuntimeImage(void);
#define MODE_SENSE_PAGE_LENGTH(page) GetModePagePayloadLength(page)
#define SELECT_MODE_SENSE_PAGE_IMAGE(page, control) SelectModePageImageAddressForPageControl((page), (control))
#define REFRESH_MODE_SENSE_PAGE_1A() UpdateModePage1ARuntimeImageForCurrentMediaState()
#define REFRESH_MODE_SENSE_PAGE_2A() UpdateModePage2ACapabilitiesRuntimeImage()
#define COPY_MODE_SENSE_PAGE(source, response, count) CopyToControllerBufferWindow((source), (response), (count))
#endif
#endif
