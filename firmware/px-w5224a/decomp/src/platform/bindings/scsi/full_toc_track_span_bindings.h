#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveFullTocSpanConvertMsf(fw_addr_t tuple_address);
extern s32 ObserveFullTocSpanCopyAndConvertLeadOut(fw_addr_t descriptor_msf_address);
#define ConvertFullTocSpanMsf(address) ObserveFullTocSpanConvertMsf(address)
#define CopyAndConvertFullTocLeadOut(address) ObserveFullTocSpanCopyAndConvertLeadOut(address)
#else
#define ConvertFullTocSpanMsf(address) ConvertBinaryMsfToAdjustedFramePosition(address)

static s32 CopyAndConvertFullTocLeadOut(fw_addr_t source_address)
{
    u8 lead_out_msf[4];

    CopyBytesWith8BitCount((fw_addr_t)lead_out_msf, source_address, 4u);
    return ConvertBinaryMsfToAdjustedFramePosition((fw_addr_t)lead_out_msf);
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_12CA);
#else
return *((volatile u16 near *)0x12ca);
#endif
#endif
