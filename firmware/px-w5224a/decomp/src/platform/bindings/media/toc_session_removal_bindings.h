#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRemoveTocResetAll(void);
extern void ObserveRemoveTocResetSummaries(void);
extern void ObserveRemoveTocCopyTen(fw_addr_t source, fw_addr_t destination);
extern void ObserveRemoveTocCopyPayload(fw_addr_t source, fw_addr_t destination);
extern s32 ObserveRemoveTocConvertTuple(u8 minute, u8 second, u8 frame);
extern s32 ObserveRemoveTocConvertAddress(fw_addr_t tuple_address);
extern u8 ObserveRemoveTocPackedBcdToBinary(u8 value);
#define REMOVE_TOC_RESET_ALL() ObserveRemoveTocResetAll()
#define REMOVE_TOC_RESET_SUMMARIES() ObserveRemoveTocResetSummaries()
#define REMOVE_TOC_COPY_TEN(source, destination) ObserveRemoveTocCopyTen((source), (destination))
#define REMOVE_TOC_COPY_PAYLOAD(source, destination) ObserveRemoveTocCopyPayload((source), (destination))
#define REMOVE_TOC_CONVERT_TUPLE(minute, second, frame) ObserveRemoveTocConvertTuple((minute), (second), (frame))
#define REMOVE_TOC_CONVERT_ADDRESS(address) ObserveRemoveTocConvertAddress(address)
#define REMOVE_TOC_BCD_TO_BINARY(value) ObserveRemoveTocPackedBcdToBinary(value)
#else
#include "platform/encoding.h"
#include "platform/memory.h"
#include "platform/toc_workspace.h"
#define REMOVE_TOC_RESET_ALL() ResetFullTocDescriptorWorkspace()
#define REMOVE_TOC_RESET_SUMMARIES() ResetDecodedTocSummaryTables()
#define REMOVE_TOC_COPY_TEN(source, destination) CopyTenBytesFar((destination), (source))
#define REMOVE_TOC_COPY_PAYLOAD(source, destination) CopyBinaryTocPayloadToFullTocDescriptorBody((source), (destination))
#define REMOVE_TOC_CONVERT_ADDRESS(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define REMOVE_TOC_BCD_TO_BINARY(value) PackedBcdByteToBinary(value)

static s32 ConvertRemovedTocTuple(u8 minute, u8 second, u8 frame)
{
    u8 tuple[3];

    tuple[0] = minute;
    tuple[1] = second;
    tuple[2] = frame;
    return ConvertBinaryMsfToAdjustedFramePosition((fw_addr_t)tuple);
}
#define REMOVE_TOC_CONVERT_TUPLE(minute, second, frame) ConvertRemovedTocTuple((minute), (second), (frame))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return (u16)(((u16)FirmwareReadNear8(address) << 8) | FirmwareReadNear8((u16)(address + 1u)));
#else
return *((volatile u16 near *)address);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteNear8(address, (u8)(value >> 8));
FirmwareWriteNear8((u16)(address + 1u), (u8)value);
#else
*((volatile u16 near *)address) = value;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteNear8(address, (u8)(value >> 24));
FirmwareWriteNear8((u16)(address + 1u), (u8)(value >> 16));
FirmwareWriteNear8((u16)(address + 2u), (u8)(value >> 8));
FirmwareWriteNear8((u16)(address + 3u), (u8)value);
#else
*((volatile u32 near *)address) = value;
#endif
#endif
