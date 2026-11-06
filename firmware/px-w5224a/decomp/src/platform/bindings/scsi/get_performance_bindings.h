#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveGetPerformanceValidateCdb(fw_addr_t mask_descriptor);
extern void ObserveGetPerformanceUnsupported(void);
extern u16 ObserveGetPerformanceAppendNominal(u8 speed, u16 limit, u16 response_length);
extern u16 ObserveGetPerformanceAppendWrite(u8 speed, u16 response_length, u16 limit);
extern u16 ObserveGetPerformanceAppendTable(fw_addr_t record, u16 response_length, u16 inherited_wr14);
extern u8 ObserveGetPerformanceTransfer(fw_addr_t source, u32 byte_count);
extern void ObserveGetPerformanceNoSense(void);
#define GET_PERFORMANCE_VALIDATE(mask) ObserveGetPerformanceValidateCdb(mask)
#define GET_PERFORMANCE_UNSUPPORTED() ObserveGetPerformanceUnsupported()
#define GET_PERFORMANCE_APPEND_NOMINAL(speed, limit, length) ObserveGetPerformanceAppendNominal((speed), (limit), (length))
#define GET_PERFORMANCE_APPEND_WRITE(speed, length, limit) ObserveGetPerformanceAppendWrite((speed), (length), (limit))
#define GET_PERFORMANCE_APPEND_TABLE(record, length, wr14) ObserveGetPerformanceAppendTable((record), (length), (wr14))
#define GET_PERFORMANCE_TRANSFER(source, count) ObserveGetPerformanceTransfer((source), (count))
#define GET_PERFORMANCE_NO_SENSE() ObserveGetPerformanceNoSense()
#else
#define GET_PERFORMANCE_VALIDATE(mask) ValidateScsiCdbFieldMasks(mask)
#define GET_PERFORMANCE_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define GET_PERFORMANCE_APPEND_NOMINAL(speed, limit, length) AppendGetPerformanceNominalSpeedDescriptor((speed), (limit), (length))
#define GET_PERFORMANCE_APPEND_WRITE(speed, length, limit) AppendGetPerformanceWriteSpeedDescriptorsForSpeed((speed), (length), (limit))
#define GET_PERFORMANCE_APPEND_TABLE(record, length, wr14) AppendGetPerformanceDescriptorFromTableRecord((record), (length), (wr14))
#define GET_PERFORMANCE_TRANSFER(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define GET_PERFORMANCE_NO_SENSE() SetNoSenseData()
#endif
#endif
