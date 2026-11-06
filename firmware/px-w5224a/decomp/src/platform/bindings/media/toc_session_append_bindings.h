#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveAppendTocCopyTen(fw_addr_t source, fw_addr_t destination);
extern void ObserveAppendTocCopyDecode(fw_addr_t source, fw_addr_t destination);
extern void ObserveAppendTocCopyBinary(fw_addr_t source, fw_addr_t destination);
extern u8 ObserveAppendTocPackedBcdToBinary(u8 value);
extern s32 ObserveAppendTocConvertAddress(fw_addr_t tuple_address);
extern s32 ObserveAppendTocConvertTuple(u8 minute, u8 second, u8 frame);
#define APPEND_TOC_COPY_TEN(source, destination) ObserveAppendTocCopyTen((source), (destination))
#define APPEND_TOC_COPY_DECODE(source, destination) ObserveAppendTocCopyDecode((source), (destination))
#define APPEND_TOC_COPY_BINARY(source, destination) ObserveAppendTocCopyBinary((source), (destination))
#define APPEND_TOC_BCD(value) ObserveAppendTocPackedBcdToBinary(value)
#define APPEND_TOC_CONVERT_ADDRESS(address) ObserveAppendTocConvertAddress(address)
#define APPEND_TOC_CONVERT_TUPLE(minute, second, frame) ObserveAppendTocConvertTuple((minute), (second), (frame))
#else
#include "platform/encoding.h"
#include "platform/memory.h"
#include "platform/raw_q_record_decode.h"
#define APPEND_TOC_COPY_TEN(source, destination) CopyTenBytesFar((destination), (source))
#define APPEND_TOC_COPY_DECODE(source, destination) CopyAndDecodeRawTocPayloadToFullTocDescriptorBody((source), (destination))
#define APPEND_TOC_COPY_BINARY(source, destination) CopyBinaryTocPayloadToFullTocDescriptorBody((source), (destination))
#define APPEND_TOC_BCD(value) PackedBcdByteToBinary(value)
#define APPEND_TOC_CONVERT_ADDRESS(address) ConvertBinaryMsfToAdjustedFramePosition(address)

static s32 ConvertAppendTocTuple(u8 minute, u8 second, u8 frame)
{
    u8 tuple[3];

    tuple[0] = minute;
    tuple[1] = second;
    tuple[2] = frame;
    return ConvertBinaryMsfToAdjustedFramePosition((fw_addr_t)tuple);
}
#define APPEND_TOC_CONVERT_TUPLE(minute, second, frame) ConvertAppendTocTuple((minute), (second), (frame))
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
