#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveQRecordQueueFill(fw_addr_t address, u32 count, u8 value);
extern u8 ObserveQRecordQueueBuild(void);
extern void ObserveQRecordQueueWait(void);
extern u8 ObserveQRecordQueueRestore(u16 count);
extern u8 ObserveQRecordQueueDiscard(u16 count);
extern void ObserveQRecordQueueReportFailure(u8 code);
#define Q_RECORD_QUEUE_FILL(address, count, value) ObserveQRecordQueueFill((address), (count), (value))
#define Q_RECORD_QUEUE_BUILD() ObserveQRecordQueueBuild()
#define Q_RECORD_QUEUE_WAIT() ObserveQRecordQueueWait()
#define Q_RECORD_QUEUE_RESTORE(count) ObserveQRecordQueueRestore(count)
#define Q_RECORD_QUEUE_DISCARD(count) ObserveQRecordQueueDiscard(count)
#define Q_RECORD_QUEUE_REPORT(code) ObserveQRecordQueueReportFailure(code)
#else
#define Q_RECORD_QUEUE_FILL(address, count, value) FillControllerBufferRange((address), (count), (value))
#define Q_RECORD_QUEUE_BUILD() BuildRawReadQRecordTable()
#define Q_RECORD_QUEUE_WAIT() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define Q_RECORD_QUEUE_RESTORE(count) RestoreRawReadDescriptorsToQueueHead(count)
#define Q_RECORD_QUEUE_DISCARD(count) DiscardRawReadDescriptorsFromQueueTail(count)
#define Q_RECORD_QUEUE_REPORT(code) ReportReadPipelineFailure(code)
#endif
#endif
