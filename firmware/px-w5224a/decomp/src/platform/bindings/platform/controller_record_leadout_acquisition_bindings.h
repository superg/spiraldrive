#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveLeadOutResumeRing(void);
extern void ObserveLeadOutConvert(s32 frame_position, fw_addr_t destination);
extern void ObserveLeadOutPackBcd(u16 tuple_address);
extern void ObserveLeadOutCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObserveLeadOutWriteControllerBlock(fw_addr_t source);
extern void ObserveLeadOutSubmit(fw_addr_t cursor, u8 parameter);
extern void ObserveLeadOutUpdateQ(void);
extern void ObserveLeadOutEnqueue(void);
extern void ObserveLeadOutIncrement(u16 tuple_address);
extern fw_addr_t ObserveLeadOutAdvanceRing(fw_addr_t cursor);
extern void ObserveLeadOutAdvanceQ(u8 reverse_relative);
extern u8 ObserveLeadOutWaitForCapacity(void);
extern u8 ObserveLeadOutWaitForRead(void);
extern void ObserveLeadOutLatchFailure(void);
extern void ObserveLeadOutSetNoSense(void);
extern void ObserveLeadOutReportFailure(u8 failure_code);
#define LEADOUT_RESUME_RING() ObserveLeadOutResumeRing()
#define LEADOUT_CONVERT(frame, destination) ObserveLeadOutConvert((frame), (destination))
#define LEADOUT_PACK(address) ObserveLeadOutPackBcd(address)
#define LEADOUT_COPY(source, destination, count) ObserveLeadOutCopy((source), (destination), (count))
#define LEADOUT_WRITE_BLOCK(source) ObserveLeadOutWriteControllerBlock(source)
#define LEADOUT_SUBMIT(cursor, parameter) ObserveLeadOutSubmit((cursor), (parameter))
#define LEADOUT_UPDATE_Q() ObserveLeadOutUpdateQ()
#define LEADOUT_ENQUEUE() ObserveLeadOutEnqueue()
#define LEADOUT_INCREMENT(address) ObserveLeadOutIncrement(address)
#define LEADOUT_ADVANCE_RING(cursor) ObserveLeadOutAdvanceRing(cursor)
#define LEADOUT_ADVANCE_Q(reverse) ObserveLeadOutAdvanceQ(reverse)
#define LEADOUT_WAIT_CAPACITY() ObserveLeadOutWaitForCapacity()
#define LEADOUT_WAIT_FOR_READ() ObserveLeadOutWaitForRead()
#define LEADOUT_LATCH_FAILURE() ObserveLeadOutLatchFailure()
#define LEADOUT_SET_NO_SENSE() ObserveLeadOutSetNoSense()
#define LEADOUT_REPORT_FAILURE(code) ObserveLeadOutReportFailure(code)
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
extern u8 WaitForReadAcquisitionCoroutineAndCheckPreparationStatus(void);
extern void LatchFirstReadFailureContext(void);
extern void SetNoSenseData(void);
extern void ReportReadPipelineFailure(u8 failure_code);
#define LEADOUT_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define LEADOUT_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define LEADOUT_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define LEADOUT_COPY(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define LEADOUT_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define LEADOUT_SUBMIT(cursor, parameter) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (parameter))
#define LEADOUT_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define LEADOUT_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define LEADOUT_INCREMENT(address) IncrementPackedBcdMsf(address)
#define LEADOUT_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define LEADOUT_ADVANCE_Q(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define LEADOUT_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define LEADOUT_WAIT_FOR_READ() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define LEADOUT_LATCH_FAILURE() LatchFirstReadFailureContext()
#define LEADOUT_SET_NO_SENSE() SetNoSenseData()
#define LEADOUT_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
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
