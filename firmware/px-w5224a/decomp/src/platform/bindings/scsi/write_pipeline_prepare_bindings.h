#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveMapWritePipelineLogicalBlockAddress(s32 logical_address);
#define MAP_WRITE_PIPELINE_ADDRESS(address) ObserveMapWritePipelineLogicalBlockAddress(address)
#else
extern s32 MapLogicalBlockAddressToControllerFramePosition(s32 logical_address);
#define MAP_WRITE_PIPELINE_ADDRESS(address) MapLogicalBlockAddressToControllerFramePosition(address)
#endif
#endif
