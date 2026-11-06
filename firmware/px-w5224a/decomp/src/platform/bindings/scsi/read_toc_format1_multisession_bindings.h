#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveReadTocFormat1ConvertMsf(fw_addr_t source_address);
#define ConvertMultiSessionMsf(source_address) ObserveReadTocFormat1ConvertMsf(source_address)
#else
static s32 ConvertMultiSessionMsf(fw_addr_t source_address)
{
    u8 tuple[4];

    CopyBytesWith8BitCount((fw_addr_t)tuple, source_address, 4u);
    return ConvertBinaryMsfToAdjustedFramePosition((fw_addr_t)tuple);
}
#endif
#endif
