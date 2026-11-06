#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveParseTocCopyDecode(fw_addr_t source, fw_addr_t destination);
extern u8 ObserveParseTocPackedBcdToBinary(u8 value);
#define PARSE_TOC_COPY_DECODE(source, destination) ObserveParseTocCopyDecode((source), (destination))
#define PARSE_TOC_BCD(value) ObserveParseTocPackedBcdToBinary(value)
#else
#include "platform/encoding.h"
#include "platform/raw_q_record_decode.h"
#define PARSE_TOC_COPY_DECODE(source, destination) CopyAndDecodeRawReadQRecordToFarTable((source), (destination))
#define PARSE_TOC_BCD(value) PackedBcdByteToBinary(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return (u16)(((u16)FirmwareReadNear8(address) << 8) | FirmwareReadNear8((u16)(address + 1u)));
#else
return *((volatile u16 near *)address);
#endif
#endif
