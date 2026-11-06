#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateReadTocCdb(fw_addr_t mask_descriptor);
extern void ObserveUnsupportedReadTocCommand(void);
extern void ObserveReadFormattedTocResponse(void);
extern void ObserveReadTocFormat1MultiSessionResponse(void);
extern void ObserveReadFullTocResponse(u8 msf_format);
extern void ObserveReadTocFormat3Pma(void);
extern void ObserveReadTocFormat4Atip(void);
extern void ObserveReadTocFormat5CdTextResponse(u16 allocation_length);
#define VALIDATE_READ_TOC_CDB(mask) ObserveValidateReadTocCdb(mask)
#define UNSUPPORTED_READ_TOC() ObserveUnsupportedReadTocCommand()
#define READ_FORMATTED_TOC() ObserveReadFormattedTocResponse()
#define READ_TOC_FORMAT1() ObserveReadTocFormat1MultiSessionResponse()
#define READ_FULL_TOC(msf) ObserveReadFullTocResponse(msf)
#define READ_TOC_FORMAT3() ObserveReadTocFormat3Pma()
#define READ_TOC_FORMAT4() ObserveReadTocFormat4Atip()
#define READ_TOC_FORMAT5(length) ObserveReadTocFormat5CdTextResponse(length)
#else
extern u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_descriptor);
extern void HandleUnsupportedScsiCommand(void);
extern u16 ReadBigEndianU16(fw_addr_t address);
#define VALIDATE_READ_TOC_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define UNSUPPORTED_READ_TOC() HandleUnsupportedScsiCommand()
#define READ_FORMATTED_TOC() ExecuteReadFormattedTocResponse()
#define READ_TOC_FORMAT1() ExecuteReadTocFormat1MultiSessionResponse()
#define READ_FULL_TOC(msf) ExecuteReadFullTocResponse(msf)
#define READ_TOC_FORMAT3() HandleReadTocFormat3Pma()
#define READ_TOC_FORMAT4() HandleReadTocFormat4Atip()
#define READ_TOC_FORMAT5(length) ExecuteReadTocFormat5CdTextResponse(length)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
READ_TOC_FORMAT5((u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_0B11));
#else
READ_TOC_FORMAT5(ReadBigEndianU16(FW_NEAR_UNKNOWN_0B11));
#endif
#endif
