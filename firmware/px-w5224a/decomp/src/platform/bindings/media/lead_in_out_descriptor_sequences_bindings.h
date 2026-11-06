#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveLeadInOutReset(void);
extern void ObserveLeadInOutClearBit0035(void);
extern u8 ObserveLeadInSequence(u8 initial_atip, u8 retry);
extern u8 ObserveLeadOutSequence(u8 retry);
extern void ObserveLeadInOutSetSense(u8 sense_key, u16 asc_ascq);
extern void ObserveLeadInOutSetNoSense(void);
extern u8 ObserveLeadInOutWait(void);
extern void ObserveLeadInOutFill(fw_addr_t address, u32 count, u8 value);
extern void ObserveLeadInOutFailure(u8 code);
extern void ObserveLeadInOutQueueSense(void);
extern u8 ObserveLeadInOutRange(s32 start, u32 count, u8 mode, u8 retry);
#define LEAD_IO_RESET() ObserveLeadInOutReset()
#define LEAD_IO_CLEAR_BIT0035() ObserveLeadInOutClearBit0035()
#define LEAD_IO_ACQUIRE_IN(initial, retry) ObserveLeadInSequence((initial), (retry))
#define LEAD_IO_ACQUIRE_OUT(retry) ObserveLeadOutSequence(retry)
#define LEAD_IO_SET_SENSE(key, ascq) ObserveLeadInOutSetSense((key), (ascq))
#define LEAD_IO_SET_NO_SENSE() ObserveLeadInOutSetNoSense()
#define LEAD_IO_WAIT() ObserveLeadInOutWait()
#define LEAD_IO_FILL(address, count, value) ObserveLeadInOutFill((address), (count), (value))
#define LEAD_IO_FAILURE(code) ObserveLeadInOutFailure(code)
#define LEAD_IO_QUEUE_SENSE() ObserveLeadInOutQueueSense()
#define LEAD_IO_RANGE(start, count, mode, retry) ObserveLeadInOutRange((start), (count), (mode), (retry))
#else
extern void PLEXTOR_NEAR_FUNCTION ClearBit0035ForLeadInOutSequenceAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ResetAndAcquireRawReadRangeForLeadInOut(u8 retry, s32 start, u32 count);
#define LEAD_IO_RESET() ResetRawReadPipelineBuffersAndState()
#define LEAD_IO_CLEAR_BIT0035() ClearBit0035ForLeadInOutSequenceAtomic()
#define LEAD_IO_ACQUIRE_IN(initial, retry) AcquireLeadInRawSectorDescriptorSequence((initial), (retry))
#define LEAD_IO_ACQUIRE_OUT(retry) AcquireLeadOutRawSectorDescriptorSequence(retry)
#define LEAD_IO_SET_SENSE(key, ascq) SetSenseData((key), (ascq))
#define LEAD_IO_SET_NO_SENSE() SetNoSenseData()
#define LEAD_IO_WAIT() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define LEAD_IO_FILL(address, count, value) FillControllerBufferRange((address), (count), (value))
#define LEAD_IO_FAILURE(code) ReportReadPipelineFailure(code)
#define LEAD_IO_QUEUE_SENSE() ReportSense030C09FromRawReadQueue()
#define LEAD_IO_RANGE(start, count, mode, retry) ResetAndAcquireRawReadRangeForLeadInOut((retry), (start), (count))
#endif
#endif
