#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveLeadInResumeRing(void);
extern void ObserveLeadInConvert(s32 frame_position, fw_addr_t destination);
extern void ObserveLeadInPackBcd(u16 tuple_address);
extern void ObserveLeadInCopyTuple(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObserveLeadInWriteControllerBlock(fw_addr_t source);
extern void ObserveLeadInSubmit(fw_addr_t cursor, u8 parameter);
extern void ObserveLeadInCopyQ(u16 destination, u16 source, u8 byte_count);
extern void ObserveLeadInUpdateQ(void);
extern void ObserveLeadInEnqueue(void);
extern void ObserveLeadInIncrement(u16 tuple_address);
extern fw_addr_t ObserveLeadInAdvanceRing(fw_addr_t cursor);
extern u8 ObserveLeadInWaitForCapacity(void);
extern u8 ObserveLeadInWaitForRead(void);
extern void ObserveLeadInLatchFailure(void);
extern void ObserveLeadInSetNoSense(void);
extern void ObserveLeadInReportFailure(u8 failure_code);
extern s32 ObserveLeadInDivide(s32 dividend, s32 divisor);
extern u32 ObserveLeadInMultiply(u32 left, u32 right);
#define LEADIN_RESUME_RING() ObserveLeadInResumeRing()
#define LEADIN_CONVERT(frame, destination) ObserveLeadInConvert((frame), (destination))
#define LEADIN_PACK(address) ObserveLeadInPackBcd(address)
#define LEADIN_COPY_TUPLE(source, destination, count) ObserveLeadInCopyTuple((source), (destination), (count))
#define LEADIN_WRITE_BLOCK(source) ObserveLeadInWriteControllerBlock(source)
#define LEADIN_SUBMIT(cursor, parameter) ObserveLeadInSubmit((cursor), (parameter))
#define LEADIN_COPY_Q(destination, source, count) ObserveLeadInCopyQ((destination), (source), (count))
#define LEADIN_UPDATE_Q() ObserveLeadInUpdateQ()
#define LEADIN_ENQUEUE() ObserveLeadInEnqueue()
#define LEADIN_INCREMENT(address) ObserveLeadInIncrement(address)
#define LEADIN_ADVANCE_RING(cursor) ObserveLeadInAdvanceRing(cursor)
#define LEADIN_WAIT_CAPACITY() ObserveLeadInWaitForCapacity()
#define LEADIN_WAIT_FOR_READ() ObserveLeadInWaitForRead()
#define LEADIN_LATCH_FAILURE() ObserveLeadInLatchFailure()
#define LEADIN_SET_NO_SENSE() ObserveLeadInSetNoSense()
#define LEADIN_REPORT_FAILURE(code) ObserveLeadInReportFailure(code)
#define LEADIN_DIVIDE(a, b) ObserveLeadInDivide((a), (b))
#define LEADIN_MULTIPLY(a, b) ObserveLeadInMultiply((a), (b))
#else
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address);
extern void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address);
extern void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
extern void CopyBytes16(u16 destination, u16 source, u8 byte_count);
extern void UpdateRawReadQPacketControlBit(void);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern void IncrementPackedBcdMsf(u16 packed_bcd_msf_address);
extern fw_addr_t AdvanceRawReadRingPosition(fw_addr_t main_data_cursor);
extern u8 WaitForRawReadProducerCapacityAndCheckStatus(void);
extern u8 WaitForReadAcquisitionCoroutineAndCheckPreparationStatus(void);
extern void LatchFirstReadFailureContext(void);
extern void SetNoSenseData(void);
extern void ReportReadPipelineFailure(u8 failure_code);
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
extern u32 MultiplyU32Low(u32 factor_a, u32 factor_b);
#define LEADIN_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define LEADIN_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define LEADIN_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define LEADIN_COPY_TUPLE(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define LEADIN_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define LEADIN_SUBMIT(cursor, parameter) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (parameter))
#define LEADIN_COPY_Q(destination, source, count) CopyBytes16((destination), (source), (count))
#define LEADIN_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define LEADIN_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define LEADIN_INCREMENT(address) IncrementPackedBcdMsf(address)
#define LEADIN_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define LEADIN_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define LEADIN_WAIT_FOR_READ() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define LEADIN_LATCH_FAILURE() LatchFirstReadFailureContext()
#define LEADIN_SET_NO_SENSE() SetNoSenseData()
#define LEADIN_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
#define LEADIN_DIVIDE(a, b) DivideS32QuotientNative((a), (b))
#define LEADIN_MULTIPLY(a, b) MultiplyU32Low((a), (b))
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
