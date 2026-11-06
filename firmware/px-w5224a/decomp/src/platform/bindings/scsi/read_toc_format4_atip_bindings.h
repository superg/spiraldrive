#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRejectReadTocFormat4Command(void);
extern u16 ObserveReadTocFormat4MinU16(u16 value, u16 limit);
extern u8 ObserveTransferReadTocFormat4Response(fw_addr_t source_address, u32 byte_count);
extern void ObserveSetReadTocFormat4NoSense(void);
#define REJECT_READ_TOC_FORMAT4() ObserveRejectReadTocFormat4Command()
#define READ_TOC_FORMAT4_MIN(value, limit) ObserveReadTocFormat4MinU16((value), (limit))
#define TRANSFER_READ_TOC_FORMAT4(source, count) ObserveTransferReadTocFormat4Response((source), (count))
#define SET_READ_TOC_FORMAT4_NO_SENSE() ObserveSetReadTocFormat4NoSense()
#else
#define REJECT_READ_TOC_FORMAT4() HandleUnsupportedScsiCommand()
#define READ_TOC_FORMAT4_MIN(value, limit) MinU16((value), (limit))
#define TRANSFER_READ_TOC_FORMAT4(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define SET_READ_TOC_FORMAT4_NO_SENSE() SetNoSenseData()
#endif
#endif
