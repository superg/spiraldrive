#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveBaseWindowResumeRing(void);
extern void ObserveBaseWindowConvert(s32 frame_position, fw_addr_t destination);
extern void ObserveBaseWindowPackBcd(u16 tuple_address);
extern void ObserveBaseWindowCopyFar(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObserveBaseWindowWriteControllerBlock(fw_addr_t source);
extern void ObserveBaseWindowSubmit(fw_addr_t cursor, u8 parameter);
extern void ObserveBaseWindowCopyQ(u16 destination, u16 source, u8 byte_count);
extern void ObserveBaseWindowBuildMip(void);
extern void ObserveBaseWindowCopyPayload(fw_addr_t destination, fw_addr_t source, u16 dword_count);
extern void ObserveBaseWindowUpdateQ(void);
extern void ObserveBaseWindowEnqueue(void);
extern void ObserveBaseWindowIncrement(u16 tuple_address);
extern fw_addr_t ObserveBaseWindowAdvanceRing(fw_addr_t cursor);
extern void ObserveBaseWindowAdvanceCaptureCursor(void);
extern u8 ObserveBaseWindowWaitForRead(void);
extern void ObserveBaseWindowLatchFailure(void);
extern void ObserveBaseWindowSetNoSense(void);
extern void ObserveBaseWindowReportFailure(u8 failure_code);
#define BASE_RESUME_RING() ObserveBaseWindowResumeRing()
#define BASE_CONVERT(frame, destination) ObserveBaseWindowConvert((frame), (destination))
#define BASE_PACK(address) ObserveBaseWindowPackBcd(address)
#define BASE_COPY_FAR(source, destination, count) ObserveBaseWindowCopyFar((source), (destination), (count))
#define BASE_WRITE_BLOCK(source) ObserveBaseWindowWriteControllerBlock(source)
#define BASE_SUBMIT(cursor, parameter) ObserveBaseWindowSubmit((cursor), (parameter))
#define BASE_COPY_Q(destination, source, count) ObserveBaseWindowCopyQ((destination), (source), (count))
#define BASE_BUILD_MIP() ObserveBaseWindowBuildMip()
#define BASE_COPY_PAYLOAD(destination, source, count) ObserveBaseWindowCopyPayload((destination), (source), (count))
#define BASE_UPDATE_Q() ObserveBaseWindowUpdateQ()
#define BASE_ENQUEUE() ObserveBaseWindowEnqueue()
#define BASE_INCREMENT(address) ObserveBaseWindowIncrement(address)
#define BASE_ADVANCE_RING(cursor) ObserveBaseWindowAdvanceRing(cursor)
#define BASE_ADVANCE_CAPTURE() ObserveBaseWindowAdvanceCaptureCursor()
#define BASE_WAIT_FOR_READ() ObserveBaseWindowWaitForRead()
#define BASE_LATCH_FAILURE() ObserveBaseWindowLatchFailure()
#define BASE_SET_NO_SENSE() ObserveBaseWindowSetNoSense()
#define BASE_REPORT_FAILURE(code) ObserveBaseWindowReportFailure(code)
#else
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address);
extern void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address);
extern void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
extern void CopyBytes16(u16 destination, u16 source, u8 byte_count);
extern void BuildMipControllerRecordSnapshotBlock(void);
extern void CopyDwordsToAddressPlus1MiB(fw_addr_t destination_base, fw_addr_t source, u16 dword_count);
extern void UpdateRawReadQPacketControlBit(void);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern void IncrementPackedBcdMsf(u16 packed_bcd_msf_address);
extern fw_addr_t AdvanceRawReadRingPosition(fw_addr_t main_data_cursor);
extern void AdvanceType2ControllerRecordCaptureCursor(void);
extern u8 WaitForReadAcquisitionCoroutineAndCheckPreparationStatus(void);
extern void LatchFirstReadFailureContext(void);
extern void SetNoSenseData(void);
extern void ReportReadPipelineFailure(u8 failure_code);
#define BASE_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define BASE_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define BASE_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define BASE_COPY_FAR(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define BASE_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define BASE_SUBMIT(cursor, parameter) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (parameter))
#define BASE_COPY_Q(destination, source, count) CopyBytes16((destination), (source), (count))
#define BASE_BUILD_MIP() BuildMipControllerRecordSnapshotBlock()
#define BASE_COPY_PAYLOAD(destination, source, count) CopyDwordsToAddressPlus1MiB((destination), (source), (count))
#define BASE_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define BASE_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define BASE_INCREMENT(address) IncrementPackedBcdMsf(address)
#define BASE_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define BASE_ADVANCE_CAPTURE() AdvanceType2ControllerRecordCaptureCursor()
#define BASE_WAIT_FOR_READ() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define BASE_LATCH_FAILURE() LatchFirstReadFailureContext()
#define BASE_SET_NO_SENSE() SetNoSenseData()
#define BASE_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
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
