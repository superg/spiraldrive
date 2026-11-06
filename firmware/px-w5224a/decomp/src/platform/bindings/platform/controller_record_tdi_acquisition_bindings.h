#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveTdiResumeRing(void);
extern void ObserveTdiConvert(s32 frame_position, fw_addr_t destination);
extern void ObserveTdiPackBcd(u16 tuple_address);
extern void ObserveTdiCopyTuple(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObserveTdiWriteControllerBlock(fw_addr_t source);
extern void ObserveTdiSubmit(fw_addr_t cursor, u8 parameter);
extern void ObserveTdiCopyPayload(fw_addr_t destination, fw_addr_t source, u8 byte_count);
extern void ObserveTdiUpdateQ(void);
extern void ObserveTdiEnqueue(void);
extern void ObserveTdiIncrement(u16 tuple_address);
extern fw_addr_t ObserveTdiAdvanceRing(fw_addr_t cursor);
extern void ObserveTdiAdvanceQ(u8 reverse_relative);
extern void ObserveTdiServiceHost(void);
extern void ObserveTdiYield(void);
extern u8 ObserveTdiWaitForRead(void);
extern void ObserveTdiLatchFailure(void);
extern void ObserveTdiSetNoSense(void);
extern void ObserveTdiReportFailure(u8 failure_code);
#define TDI_RESUME_RING() ObserveTdiResumeRing()
#define TDI_CONVERT(frame, destination) ObserveTdiConvert((frame), (destination))
#define TDI_PACK(address) ObserveTdiPackBcd(address)
#define TDI_COPY_TUPLE(source, destination, count) ObserveTdiCopyTuple((source), (destination), (count))
#define TDI_WRITE_BLOCK(source) ObserveTdiWriteControllerBlock(source)
#define TDI_SUBMIT(cursor, parameter) ObserveTdiSubmit((cursor), (parameter))
#define TDI_COPY_PAYLOAD(destination, source, count) ObserveTdiCopyPayload((destination), (source), (count))
#define TDI_UPDATE_Q() ObserveTdiUpdateQ()
#define TDI_ENQUEUE() ObserveTdiEnqueue()
#define TDI_INCREMENT(address) ObserveTdiIncrement(address)
#define TDI_ADVANCE_RING(cursor) ObserveTdiAdvanceRing(cursor)
#define TDI_ADVANCE_Q(reverse) ObserveTdiAdvanceQ(reverse)
#define TDI_SERVICE_HOST() ObserveTdiServiceHost()
#define TDI_YIELD() ObserveTdiYield()
#define TDI_WAIT_FOR_READ() ObserveTdiWaitForRead()
#define TDI_LATCH_FAILURE() ObserveTdiLatchFailure()
#define TDI_SET_NO_SENSE() ObserveTdiSetNoSense()
#define TDI_REPORT_FAILURE(code) ObserveTdiReportFailure(code)
#else
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address);
extern void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address);
extern void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
extern void CopyBytesToAddressPlus1MiB(fw_addr_t destination_base, fw_addr_t source, u8 byte_count);
extern void UpdateRawReadQPacketControlBit(void);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern void IncrementPackedBcdMsf(u16 packed_bcd_msf_address);
extern fw_addr_t AdvanceRawReadRingPosition(fw_addr_t main_data_cursor);
extern void AdvanceRawReadQPacketMsfTuples(u8 reverse_relative);
extern void ServicePendingHostCommandWhenOperationFlagsPermit(void);
extern void YieldCoroutine0(void);
extern u8 WaitForReadAcquisitionCoroutineAndCheckPreparationStatus(void);
extern void LatchFirstReadFailureContext(void);
extern void SetNoSenseData(void);
extern void ReportReadPipelineFailure(u8 failure_code);
#define TDI_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define TDI_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define TDI_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define TDI_COPY_TUPLE(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define TDI_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define TDI_SUBMIT(cursor, parameter) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (parameter))
#define TDI_COPY_PAYLOAD(destination, source, count) CopyBytesToAddressPlus1MiB((destination), (source), (count))
#define TDI_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define TDI_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define TDI_INCREMENT(address) IncrementPackedBcdMsf(address)
#define TDI_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define TDI_ADVANCE_Q(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define TDI_SERVICE_HOST() ServicePendingHostCommandWhenOperationFlagsPermit()
#define TDI_YIELD() YieldCoroutine0()
#define TDI_WAIT_FOR_READ() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define TDI_LATCH_FAILURE() LatchFirstReadFailureContext()
#define TDI_SET_NO_SENSE() SetNoSenseData()
#define TDI_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
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
