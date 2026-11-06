#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveRecordWindowResumeRing(void);
extern void ObserveRecordWindowConvert(s32 frame_position, fw_addr_t destination);
extern void ObserveRecordWindowPackBcd(u16 tuple_address);
extern void ObserveRecordWindowCopyTuple(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObserveRecordWindowWriteControllerBlock(fw_addr_t source);
extern void ObserveRecordWindowSubmit(fw_addr_t cursor, u8 parameter);
extern void ObserveRecordWindowBuildMip(void);
extern void ObserveRecordWindowWriteMarker(void);
extern void ObserveRecordWindowCopyPayload(fw_addr_t destination, fw_addr_t source, u16 dword_count);
extern void ObserveRecordWindowUpdateQ(void);
extern void ObserveRecordWindowEnqueue(void);
extern void ObserveRecordWindowIncrement(u16 tuple_address);
extern fw_addr_t ObserveRecordWindowAdvanceRing(fw_addr_t cursor);
extern void ObserveRecordWindowAdvanceQ(u8 reverse_relative);
extern u8 ObserveRecordWindowWaitForRead(void);
extern void ObserveRecordWindowLatchFailure(void);
extern void ObserveRecordWindowSetNoSense(void);
extern void ObserveRecordWindowReportFailure(u8 failure_code);
#define WINDOW_RESUME_RING() ObserveRecordWindowResumeRing()
#define WINDOW_CONVERT(frame, destination) ObserveRecordWindowConvert((frame), (destination))
#define WINDOW_PACK(address) ObserveRecordWindowPackBcd(address)
#define WINDOW_COPY_TUPLE(source, destination, count) ObserveRecordWindowCopyTuple((source), (destination), (count))
#define WINDOW_WRITE_BLOCK(source) ObserveRecordWindowWriteControllerBlock(source)
#define WINDOW_SUBMIT(cursor, parameter) ObserveRecordWindowSubmit((cursor), (parameter))
#define WINDOW_BUILD_MIP() ObserveRecordWindowBuildMip()
#define WINDOW_WRITE_MARKER() ObserveRecordWindowWriteMarker()
#define WINDOW_COPY_PAYLOAD(destination, source, count) ObserveRecordWindowCopyPayload((destination), (source), (count))
#define WINDOW_UPDATE_Q() ObserveRecordWindowUpdateQ()
#define WINDOW_ENQUEUE() ObserveRecordWindowEnqueue()
#define WINDOW_INCREMENT(address) ObserveRecordWindowIncrement(address)
#define WINDOW_ADVANCE_RING(cursor) ObserveRecordWindowAdvanceRing(cursor)
#define WINDOW_ADVANCE_Q(reverse) ObserveRecordWindowAdvanceQ(reverse)
#define WINDOW_WAIT_FOR_READ() ObserveRecordWindowWaitForRead()
#define WINDOW_LATCH_FAILURE() ObserveRecordWindowLatchFailure()
#define WINDOW_SET_NO_SENSE() ObserveRecordWindowSetNoSense()
#define WINDOW_REPORT_FAILURE(code) ObserveRecordWindowReportFailure(code)
#else
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address);
extern void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address);
extern void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
extern void BuildMipControllerRecordSnapshotBlock(void);
extern void WriteControllerBufferByte53AtOffsetA5DF(void);
extern void CopyDwordsToAddressPlus1MiB(fw_addr_t destination_base, fw_addr_t source, u16 dword_count);
extern void UpdateRawReadQPacketControlBit(void);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern void IncrementPackedBcdMsf(u16 packed_bcd_msf_address);
extern fw_addr_t AdvanceRawReadRingPosition(fw_addr_t main_data_cursor);
extern void AdvanceRawReadQPacketMsfTuples(u8 reverse_relative);
extern u8 WaitForReadAcquisitionCoroutineAndCheckPreparationStatus(void);
extern void LatchFirstReadFailureContext(void);
extern void SetNoSenseData(void);
extern void ReportReadPipelineFailure(u8 failure_code);
#define WINDOW_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define WINDOW_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define WINDOW_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define WINDOW_COPY_TUPLE(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define WINDOW_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define WINDOW_SUBMIT(cursor, parameter) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (parameter))
#define WINDOW_BUILD_MIP() BuildMipControllerRecordSnapshotBlock()
#define WINDOW_WRITE_MARKER() WriteControllerBufferByte53AtOffsetA5DF()
#define WINDOW_COPY_PAYLOAD(destination, source, count) CopyDwordsToAddressPlus1MiB((destination), (source), (count))
#define WINDOW_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define WINDOW_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define WINDOW_INCREMENT(address) IncrementPackedBcdMsf(address)
#define WINDOW_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define WINDOW_ADVANCE_Q(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define WINDOW_WAIT_FOR_READ() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define WINDOW_LATCH_FAILURE() LatchFirstReadFailureContext()
#define WINDOW_SET_NO_SENSE() SetNoSenseData()
#define WINDOW_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
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
