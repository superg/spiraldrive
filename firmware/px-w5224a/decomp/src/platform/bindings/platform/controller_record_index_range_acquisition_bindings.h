#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveRecordRangeResumeRing(void);
extern void ObserveRecordRangeConvert(s32 frame_position, fw_addr_t destination);
extern void ObserveRecordRangePackBcd(u16 tuple_address);
extern void ObserveRecordRangeCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObserveRecordRangeWriteControllerBlock(fw_addr_t source);
extern void ObserveRecordRangeSubmit(fw_addr_t cursor, u8 parameter);
extern void ObserveRecordRangeUpdateQ(void);
extern void ObserveRecordRangeEnqueue(void);
extern void ObserveRecordRangeIncrement(u16 tuple_address);
extern fw_addr_t ObserveRecordRangeAdvanceRing(fw_addr_t cursor);
extern void ObserveRecordRangeAdvanceQ(u8 reverse_relative);
extern void ObserveRecordRangeWaitForCapacity(void);
extern void ObserveRecordRangeServiceHost(void);
extern void ObserveRecordRangeYield(void);
extern void ObserveRecordRangeClearCompletion(void);
extern void ObserveRecordRangeWaitForRead(void);
extern void ObserveRecordRangeRefresh(u16 first_index, u16 last_index);
#define RANGE_RESUME_RING() ObserveRecordRangeResumeRing()
#define RANGE_CONVERT(frame, destination) ObserveRecordRangeConvert((frame), (destination))
#define RANGE_PACK(address) ObserveRecordRangePackBcd(address)
#define RANGE_COPY(source, destination, count) ObserveRecordRangeCopy((source), (destination), (count))
#define RANGE_WRITE_BLOCK(source) ObserveRecordRangeWriteControllerBlock(source)
#define RANGE_SUBMIT(cursor, parameter) ObserveRecordRangeSubmit((cursor), (parameter))
#define RANGE_UPDATE_Q() ObserveRecordRangeUpdateQ()
#define RANGE_ENQUEUE() ObserveRecordRangeEnqueue()
#define RANGE_INCREMENT(address) ObserveRecordRangeIncrement(address)
#define RANGE_ADVANCE_RING(cursor) ObserveRecordRangeAdvanceRing(cursor)
#define RANGE_ADVANCE_Q(reverse) ObserveRecordRangeAdvanceQ(reverse)
#define RANGE_WAIT_CAPACITY() ObserveRecordRangeWaitForCapacity()
#define RANGE_SERVICE_HOST() ObserveRecordRangeServiceHost()
#define RANGE_YIELD() ObserveRecordRangeYield()
#define RANGE_CLEAR_COMPLETION() ObserveRecordRangeClearCompletion()
#define RANGE_WAIT_FOR_READ() ObserveRecordRangeWaitForRead()
#define RANGE_REFRESH(first, last) ObserveRecordRangeRefresh((first), (last))
#else
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address);
extern void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address);
extern void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
extern void UpdateRawReadQPacketControlBit(void);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern void IncrementPackedBcdMsf(u16 packed_bcd_msf_address);
extern fw_addr_t AdvanceRawReadRingPosition(fw_addr_t main_data_cursor);
extern void AdvanceRawReadQPacketMsfTuples(u8 reverse_relative);
extern u8 WaitForRawReadProducerCapacityAndCheckStatus(void);
extern void ServicePendingHostCommandWhenOperationFlagsPermit(void);
extern void YieldCoroutine0(void);
extern void ClearScsiCompletionBits0And1IfBit4Set(void);
extern u8 WaitForReadAcquisitionCoroutineAndCheckPreparationStatus(void);
extern void RefreshType3ControllerRecordBlocksForIndexRange(u16 first_index, u16 last_index);
#define RANGE_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define RANGE_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define RANGE_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define RANGE_COPY(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define RANGE_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define RANGE_SUBMIT(cursor, parameter) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (parameter))
#define RANGE_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define RANGE_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define RANGE_INCREMENT(address) IncrementPackedBcdMsf(address)
#define RANGE_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define RANGE_ADVANCE_Q(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define RANGE_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define RANGE_SERVICE_HOST() ServicePendingHostCommandWhenOperationFlagsPermit()
#define RANGE_YIELD() YieldCoroutine0()
#define RANGE_CLEAR_COMPLETION() ClearScsiCompletionBits0And1IfBit4Set()
#define RANGE_WAIT_FOR_READ() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define RANGE_REFRESH(first, last) RefreshType3ControllerRecordBlocksForIndexRange((first), (last))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t temporary_tuple = FW_NEAR_UNKNOWN_7000;
#else
u8 tuple[3];
fw_addr_t temporary_tuple = (fw_addr_t)tuple;
#endif
#endif
