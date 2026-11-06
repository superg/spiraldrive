#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveTransferBatchReadBit0028(void);
extern u8 ObserveTransferBatchReadBit0036(void);
extern void ObserveTransferBatchYield(void);
extern void ObserveTransferBatchSetSense(u8 key, u16 code);
extern void ObserveTransferBatchSetInformation(u32 information);
extern u8 ObserveTransferBatchShouldWait(void);
extern u8 ObserveTransferBatchTryRecover(void);
extern u8 ObserveTransferBatchFailPreparation(void);
extern void ObserveTransferBatchRestartProducer(void);
extern u8 ObserveTransferBatchTryRelease(void);
extern u16 ObserveTransferBatchComputeCount(void);
extern u8 ObserveTransferBatchPrepareFixedPacket(void);
extern void ObserveTransferBatchServiceHost(void);
extern u8 ObserveTransferBatchComparePosition(s32 position, u16 record_index);
extern void ObserveTransferBatchSequenceError(void);
extern void ObserveTransferBatchBuildTdi(fw_addr_t address);
extern void ObserveTransferBatchAdvanceTrack(void);
extern void ObserveTransferBatchRefreshIsrc(void);
extern u8 ObserveTransferBatchDecodeQType(u16 record_index);
extern fw_addr_t ObserveTransferBatchResumeRing(void);
extern void ObserveTransferBatchConfigure(fw_addr_t fallback, u16 count);
extern void ObserveTransferBatchEnqueue(u16 record_index);
extern u8 ObserveTransferBatchFinalize(fw_addr_t address, u16 record_index, u8 completed_count);
extern u8 ObserveTransferBatchAdvancePacket(void);
extern void ObserveTransferBatchQueueSense(void);
#define TRANSFER_READ_BIT0028() ObserveTransferBatchReadBit0028()
#define TRANSFER_READ_BIT0036() ObserveTransferBatchReadBit0036()
#define TRANSFER_YIELD() ObserveTransferBatchYield()
#define TRANSFER_SET_SENSE(k, c) ObserveTransferBatchSetSense((k), (c))
#define TRANSFER_SET_INFORMATION(v) ObserveTransferBatchSetInformation(v)
#define TRANSFER_SHOULD_WAIT() ObserveTransferBatchShouldWait()
#define TRANSFER_TRY_RECOVER() ObserveTransferBatchTryRecover()
#define TRANSFER_FAIL_PREPARATION() ObserveTransferBatchFailPreparation()
#define TRANSFER_RESTART_PRODUCER() ObserveTransferBatchRestartProducer()
#define TRANSFER_TRY_RELEASE() ObserveTransferBatchTryRelease()
#define TRANSFER_COMPUTE_COUNT() ObserveTransferBatchComputeCount()
#define TRANSFER_PREPARE_FIXED() ObserveTransferBatchPrepareFixedPacket()
#define TRANSFER_SERVICE_HOST() ObserveTransferBatchServiceHost()
#define TRANSFER_COMPARE(p, i) ObserveTransferBatchComparePosition((p), (i))
#define TRANSFER_SEQUENCE_ERROR() ObserveTransferBatchSequenceError()
#define TRANSFER_BUILD_TDI(a) ObserveTransferBatchBuildTdi(a)
#define TRANSFER_ADVANCE_TRACK() ObserveTransferBatchAdvanceTrack()
#define TRANSFER_REFRESH_ISRC() ObserveTransferBatchRefreshIsrc()
#define TRANSFER_DECODE_Q(i) ObserveTransferBatchDecodeQType(i)
#define TRANSFER_RESUME_RING() ObserveTransferBatchResumeRing()
#define TRANSFER_CONFIGURE(a, c) ObserveTransferBatchConfigure((a), (c))
#define TRANSFER_ENQUEUE(i) ObserveTransferBatchEnqueue(i)
#define TRANSFER_FINALIZE(a, i, c) ObserveTransferBatchFinalize((a), (i), (c))
#define TRANSFER_ADVANCE_PACKET() ObserveTransferBatchAdvancePacket()
#define TRANSFER_QUEUE_SENSE() ObserveTransferBatchQueueSense()
#else
u8 bdata RawReadTransferBatchBitByte25 _at_ 0x25;
u8 bdata RawReadTransferBatchBitByte26 _at_ 0x26;
sbit RawReadTransferBatchBit0028 = RawReadTransferBatchBitByte25 ^ 0;
sbit RawReadTransferBatchBit0036 = RawReadTransferBatchBitByte26 ^ 6;
extern void YieldCoroutine0(void);
#define TRANSFER_READ_BIT0028() RawReadTransferBatchBit0028
#define TRANSFER_READ_BIT0036() RawReadTransferBatchBit0036
#define TRANSFER_YIELD() YieldCoroutine0()
#define TRANSFER_SET_SENSE(k, c) SetSenseData((k), (c))
#define TRANSFER_SET_INFORMATION(v) SetSenseInformation(v)
#define TRANSFER_SHOULD_WAIT() ShouldWaitForRawReadProducerCapacity()
#define TRANSFER_TRY_RECOVER() TryRecoverRawReadPipelineFailure()
#define TRANSFER_FAIL_PREPARATION() FailIfReadMeasurementPreparationActive()
#define TRANSFER_RESTART_PRODUCER() RestartReadAcquisitionCoroutineForProducerWait()
#define TRANSFER_TRY_RELEASE() TryReleaseRawReadProducerCapacityWait()
#define TRANSFER_COMPUTE_COUNT() ComputeRawReadTransferBatchBlockCount()
#define TRANSFER_PREPARE_FIXED() PrepareRawReadPlanAtFixedPacketBoundary()
#define TRANSFER_SERVICE_HOST() ServicePendingHostCommandWhenOperationFlagsPermit()
#define TRANSFER_COMPARE(p, i) CompareFramePositionWithRawReadPreparationRecord((p), (i))
#define TRANSFER_SEQUENCE_ERROR() SetRawReadPositionCommandSequenceError()
#define TRANSFER_BUILD_TDI(a) BuildRawSectorTdiPayloadPrefix(a)
#define TRANSFER_ADVANCE_TRACK() AdvanceRawReadTrackRecordAndUpdateCurrentQPacket()
#define TRANSFER_REFRESH_ISRC() RefreshIsrcForCurrentRawTrack()
#define TRANSFER_DECODE_Q(i) DecodeRawReadQRecordTypeCode(i)
#define TRANSFER_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define TRANSFER_CONFIGURE(a, c) ConfigureRawReadControllerTransferBatch((a), (c))
#define TRANSFER_ENQUEUE(i) EnqueueRawReadDescriptorAndAdvanceQStateForIndex(i)
#define TRANSFER_FINALIZE(a, i, c) FinalizeRawReadTransferBatchDescriptors((a), (i), (c))
#define TRANSFER_ADVANCE_PACKET() AdvanceRawReadPacketCursorAfterTransferBatch()
#define TRANSFER_QUEUE_SENSE() ReportSense030C09FromRawReadQueue()
#endif
#endif
