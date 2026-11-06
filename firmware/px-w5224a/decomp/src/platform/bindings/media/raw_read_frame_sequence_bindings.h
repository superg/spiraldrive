#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawReadFrameSequenceStateUpdate(void);
extern fw_addr_t ObserveRawReadFrameSequenceResumeRing(void);
extern u8 ObserveRawReadFrameSequenceWaitForCapacity(void);
extern void ObserveRawReadFrameSequenceReportFailure(u8 failure_code);
extern u8 ObserveRawReadFrameSequenceCapture(fw_addr_t controller_buffer_address, u8 source_kind);
extern void ObserveRawReadFrameSequenceSetSense(u8 sense_key, u16 asc_ascq);
extern void ObserveRawReadFrameSequenceSetSenseInformation(u32 information);
extern void ObserveRawReadFrameSequenceUpdateQ(u8 skip_identification_packets, u8 reverse_relative_msf);
extern void ObserveRawReadFrameSequenceEnqueue(void);
extern void ObserveRawReadFrameSequenceAdvanceMsf(u8 reverse_relative_msf);
extern fw_addr_t ObserveRawReadFrameSequenceAdvanceRing(fw_addr_t controller_buffer_address);
extern void ObserveRawReadFrameSequenceCopyTuple(u16 destination, u16 source);
extern void ObserveRawReadFrameSequenceIncrementTuple(u16 address);
extern void ObserveRawReadFrameSequenceWriteControllerBlock(fw_addr_t source_address);
extern void ObserveRawReadFrameSequencePopulateSubheader(fw_addr_t controller_buffer_address, u8 source_kind);
extern u8 InterceptRawReadFrameSequenceWithCurrentQPacket(s32 frame_count, u8 source_kind, u8 *result);
extern u8 ObserveProducerCapacityShouldWait(void);
extern void ObserveProducerCapacityServicePendingHost(void);
extern void ObserveProducerCapacityYield(void);
extern u8 ObserveProducerCapacityRestartAcquisition(void);
extern void ObserveProducerCapacityLatchFailure(void);
extern void ObserveProducerCapacityReportFailure(u8 failure_code);
extern void ObserveProducerCapacityReportSense(void);
extern u8 ObserveProducerCapacityReadBit0028(void);
extern u8 ObserveProducerCapacityReadBit0031(void);
extern void ObserveProducerCapacityClearBit0028(void);
extern u8 ObservePrefixTargetSelectCurrentSectorClass(void);
extern u8 ObservePrefixTargetFindAdjacentQRecordClass(u16 record_index, u8 scan_forward);
extern u8 ObservePrefixTargetDecodeQRecordTypeCode(u16 record_index);
extern u8 ObservePrefixTargetAcquireWithQUpdates(s32 frame_count, u8 reverse_relative_msf);
extern u8 ObservePrefixTargetAcquireFixedWindow(u8 prefix_mode, u8 capture_source_kind);
extern void ObserveFixedRawReadPrefixStateUpdate(void);
extern fw_addr_t ObserveFixedRawReadPrefixResumeRing(void);
extern u8 ObserveFixedRawReadPrefixWaitForCapacity(void);
extern void ObserveFixedRawReadPrefixReportFailure(u8 failure_code);
extern void ObserveFixedRawReadPrefixCopyTuple(u16 destination, u16 source);
extern void ObserveFixedRawReadPrefixIncrementTuple(u16 address);
extern void ObserveFixedRawReadPrefixWriteControllerBlock(fw_addr_t source_address);
extern u8 ObserveFixedRawReadPrefixCapture(fw_addr_t controller_buffer_address, u8 source_kind);
extern void ObserveFixedRawReadPrefixSetSense(u8 sense_key, u16 asc_ascq);
extern void ObserveFixedRawReadPrefixSetSenseInformation(u32 information);
extern void ObserveFixedRawReadPrefixPopulateSubheader(fw_addr_t controller_buffer_address, u8 source_kind);
extern void ObserveFixedRawReadPrefixUpdateQ(u8 skip_identification_packets, u8 reverse_relative_msf);
extern void ObserveFixedRawReadPrefixEnqueue(void);
extern void ObserveFixedRawReadPrefixAdvanceMsf(u8 reverse_relative_msf);
extern fw_addr_t ObserveFixedRawReadPrefixAdvanceRing(fw_addr_t controller_buffer_address);
extern void ObserveFixedRawReadPrefixClearCompletionBit(void);
#define UPDATE_RAW_READ_STATE() ObserveRawReadFrameSequenceStateUpdate()
#define RESUME_RAW_READ_RING() ObserveRawReadFrameSequenceResumeRing()
#define WAIT_FOR_RAW_READ_CAPACITY() ObserveRawReadFrameSequenceWaitForCapacity()
#define REPORT_RAW_READ_FAILURE(code) ObserveRawReadFrameSequenceReportFailure(code)
#define CAPTURE_RAW_READ_SECTOR(address, kind) ObserveRawReadFrameSequenceCapture((address), (kind))
#define SET_RAW_READ_SENSE(key, code) ObserveRawReadFrameSequenceSetSense((key), (code))
#define SET_RAW_READ_INFORMATION(value) ObserveRawReadFrameSequenceSetSenseInformation(value)
#define UPDATE_RAW_READ_Q(skip, reverse) ObserveRawReadFrameSequenceUpdateQ((skip), (reverse))
#define ENQUEUE_RAW_READ_DESCRIPTOR() ObserveRawReadFrameSequenceEnqueue()
#define ADVANCE_RAW_READ_MSF(reverse) ObserveRawReadFrameSequenceAdvanceMsf(reverse)
#define ADVANCE_RAW_READ_RING(address) ObserveRawReadFrameSequenceAdvanceRing(address)
#define COPY_RAW_READ_TUPLE(destination, source) ObserveRawReadFrameSequenceCopyTuple((destination), (source))
#define INCREMENT_RAW_READ_TUPLE(address) ObserveRawReadFrameSequenceIncrementTuple(address)
#define WRITE_RAW_READ_CONTROLLER_BLOCK(address) ObserveRawReadFrameSequenceWriteControllerBlock(address)
#define POPULATE_RAW_READ_SUBHEADER(address, kind) ObserveRawReadFrameSequencePopulateSubheader((address), (kind))
#define UPDATE_FIXED_PREFIX_STATE() ObserveFixedRawReadPrefixStateUpdate()
#define RESUME_FIXED_PREFIX_RING() ObserveFixedRawReadPrefixResumeRing()
#define WAIT_FOR_FIXED_PREFIX_CAPACITY() ObserveFixedRawReadPrefixWaitForCapacity()
#define REPORT_FIXED_PREFIX_FAILURE(code) ObserveFixedRawReadPrefixReportFailure(code)
#define COPY_FIXED_PREFIX_TUPLE(destination, source) ObserveFixedRawReadPrefixCopyTuple((destination), (source))
#define INCREMENT_FIXED_PREFIX_TUPLE(address) ObserveFixedRawReadPrefixIncrementTuple(address)
#define WRITE_FIXED_PREFIX_CONTROLLER_BLOCK(address) ObserveFixedRawReadPrefixWriteControllerBlock(address)
#define CAPTURE_FIXED_PREFIX_SECTOR(address, kind) ObserveFixedRawReadPrefixCapture((address), (kind))
#define SET_FIXED_PREFIX_SENSE(key, code) ObserveFixedRawReadPrefixSetSense((key), (code))
#define SET_FIXED_PREFIX_INFORMATION(value) ObserveFixedRawReadPrefixSetSenseInformation(value)
#define POPULATE_FIXED_PREFIX_SUBHEADER(address, kind) ObserveFixedRawReadPrefixPopulateSubheader((address), (kind))
#define UPDATE_FIXED_PREFIX_Q(skip, reverse) ObserveFixedRawReadPrefixUpdateQ((skip), (reverse))
#define ENQUEUE_FIXED_PREFIX_DESCRIPTOR() ObserveFixedRawReadPrefixEnqueue()
#define ADVANCE_FIXED_PREFIX_MSF(reverse) ObserveFixedRawReadPrefixAdvanceMsf(reverse)
#define ADVANCE_FIXED_PREFIX_RING(address) ObserveFixedRawReadPrefixAdvanceRing(address)
#define CLEAR_FIXED_PREFIX_COMPLETION_BIT() ObserveFixedRawReadPrefixClearCompletionBit()
#define PRODUCER_CAPACITY_SHOULD_WAIT() ObserveProducerCapacityShouldWait()
#define PRODUCER_CAPACITY_SERVICE_HOST() ObserveProducerCapacityServicePendingHost()
#define PRODUCER_CAPACITY_YIELD() ObserveProducerCapacityYield()
#define PRODUCER_CAPACITY_RESTART() ObserveProducerCapacityRestartAcquisition()
#define PRODUCER_CAPACITY_LATCH_FAILURE() ObserveProducerCapacityLatchFailure()
#define PRODUCER_CAPACITY_REPORT_FAILURE(code) ObserveProducerCapacityReportFailure(code)
#define PRODUCER_CAPACITY_REPORT_SENSE() ObserveProducerCapacityReportSense()
#define PRODUCER_CAPACITY_READ_BIT0028() ObserveProducerCapacityReadBit0028()
#define PRODUCER_CAPACITY_READ_BIT0031() ObserveProducerCapacityReadBit0031()
#define PRODUCER_CAPACITY_CLEAR_BIT0028() ObserveProducerCapacityClearBit0028()
#define PREFIX_TARGET_SELECT_CURRENT_CLASS() ObservePrefixTargetSelectCurrentSectorClass()
#define PREFIX_TARGET_FIND_ADJACENT_CLASS(index, forward) ObservePrefixTargetFindAdjacentQRecordClass((index), (forward))
#define PREFIX_TARGET_DECODE_Q_TYPE(index) ObservePrefixTargetDecodeQRecordTypeCode(index)
#define PREFIX_TARGET_ACQUIRE_Q_UPDATES(count, reverse) ObservePrefixTargetAcquireWithQUpdates((count), (reverse))
#define PREFIX_TARGET_ACQUIRE_FIXED(mode, source) ObservePrefixTargetAcquireFixedWindow((mode), (source))
#else
extern void ClearBit0030Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearReadMeasurementBit0028Atomic(void);
extern void ServicePendingHostCommandWhenOperationFlagsPermit(void);
extern void YieldCoroutine0(void);
#define UPDATE_RAW_READ_STATE() UpdateRawReadStateFlagFromModeAndCurrentQ()
#define RESUME_RAW_READ_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define WAIT_FOR_RAW_READ_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define REPORT_RAW_READ_FAILURE(code) ReportReadPipelineFailure(code)
#define CAPTURE_RAW_READ_SECTOR(address, kind) CaptureRawSectorToControllerBufferAndWait((address), (kind))
#define SET_RAW_READ_SENSE(key, code) SetSenseData((key), (code))
#define SET_RAW_READ_INFORMATION(value) SetSenseInformation(value)
#define UPDATE_RAW_READ_Q(skip, reverse) UpdateRawReadQPacketControlAndIdentification((skip), (reverse))
#define ENQUEUE_RAW_READ_DESCRIPTOR() EnqueueCurrentRawSectorDescriptorAtomic()
#define ADVANCE_RAW_READ_MSF(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define ADVANCE_RAW_READ_RING(address) AdvanceRawReadRingPosition(address)
#define COPY_RAW_READ_TUPLE(destination, source) CopyThreeBytes16((destination), (source))
#define INCREMENT_RAW_READ_TUPLE(address) IncrementPackedBcdMsf(address)
#define WRITE_RAW_READ_CONTROLLER_BLOCK(address) WriteControllerRegisterBlock25FromMemory(address)
#define POPULATE_RAW_READ_SUBHEADER(address, kind) PopulateConfiguredMode2SubheaderWhenRequired((address), (kind))
#define UPDATE_FIXED_PREFIX_STATE() UpdateRawReadStateFlagFromModeAndCurrentQ()
#define RESUME_FIXED_PREFIX_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define WAIT_FOR_FIXED_PREFIX_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define REPORT_FIXED_PREFIX_FAILURE(code) ReportReadPipelineFailure(code)
#define COPY_FIXED_PREFIX_TUPLE(destination, source) CopyThreeBytes16((destination), (source))
#define INCREMENT_FIXED_PREFIX_TUPLE(address) IncrementPackedBcdMsf(address)
#define WRITE_FIXED_PREFIX_CONTROLLER_BLOCK(address) WriteControllerRegisterBlock25FromMemory(address)
#define CAPTURE_FIXED_PREFIX_SECTOR(address, kind) CaptureRawSectorToControllerBufferAndWait((address), (kind))
#define SET_FIXED_PREFIX_SENSE(key, code) SetSenseData((key), (code))
#define SET_FIXED_PREFIX_INFORMATION(value) SetSenseInformation(value)
#define POPULATE_FIXED_PREFIX_SUBHEADER(address, kind) PopulateConfiguredMode2SubheaderWhenRequired((address), (kind))
#define UPDATE_FIXED_PREFIX_Q(skip, reverse) UpdateRawReadQPacketControlAndIdentification((skip), (reverse))
#define ENQUEUE_FIXED_PREFIX_DESCRIPTOR() EnqueueCurrentRawSectorDescriptorAtomic()
#define ADVANCE_FIXED_PREFIX_MSF(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define ADVANCE_FIXED_PREFIX_RING(address) AdvanceRawReadRingPosition(address)
#define CLEAR_FIXED_PREFIX_COMPLETION_BIT() ClearBit0030Atomic()
#define PRODUCER_CAPACITY_SHOULD_WAIT() ShouldWaitForRawReadProducerCapacity()
#define PRODUCER_CAPACITY_SERVICE_HOST() ServicePendingHostCommandWhenOperationFlagsPermit()
#define PRODUCER_CAPACITY_YIELD() YieldCoroutine0()
#define PRODUCER_CAPACITY_RESTART() RestartReadAcquisitionCoroutineForProducerWait()
#define PRODUCER_CAPACITY_LATCH_FAILURE() LatchFirstReadFailureContext()
#define PRODUCER_CAPACITY_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
#define PRODUCER_CAPACITY_REPORT_SENSE() ReportSense030C09FromRawReadQueue()
#define PRODUCER_CAPACITY_READ_BIT0028() FirmwareReadBit0028()
#define PRODUCER_CAPACITY_READ_BIT0031() FirmwareReadBit0031()
#define PRODUCER_CAPACITY_CLEAR_BIT0028() ClearReadMeasurementBit0028Atomic()
#define PREFIX_TARGET_SELECT_CURRENT_CLASS() SelectCurrentRawReadSectorClassFromR3(0u)
#define PREFIX_TARGET_FIND_ADJACENT_CLASS(index, forward) FindAdjacentRawReadQRecordClass((index), (forward))
#define PREFIX_TARGET_DECODE_Q_TYPE(index) DecodeRawReadQRecordTypeCode(index)
#define PREFIX_TARGET_ACQUIRE_Q_UPDATES(count, reverse) AcquireRawReadFrameSequenceWithQUpdates((count), (reverse))
#define PREFIX_TARGET_ACQUIRE_FIXED(mode, source) AcquireFixedRawReadFramePrefixWindow((mode), (source))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t target_tuple_address = 0x7000ul;
#else
u8 target_tuple[3];
fw_addr_t target_tuple_address = (fw_addr_t)target_tuple;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
u8 intercepted_result;

if(InterceptRawReadFrameSequenceWithCurrentQPacket(frame_count, capture_source_kind, &intercepted_result) != 0u)
    return intercepted_result;
#endif
#endif
