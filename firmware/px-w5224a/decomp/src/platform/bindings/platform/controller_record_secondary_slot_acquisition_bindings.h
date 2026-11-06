#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveSecondarySlotResumeRing(void);
extern void ObserveSecondarySlotConvert(s32 frame_position, fw_addr_t destination);
extern void ObserveSecondarySlotPackBcd(u16 tuple_address);
extern void ObserveSecondarySlotCopyTuple(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObserveSecondarySlotWriteControllerBlock(fw_addr_t source);
extern void ObserveSecondarySlotSubmit(fw_addr_t cursor, u8 parameter);
extern void ObserveSecondarySlotBuildWorkspace(u8 bank_index, u8 page_index);
extern void ObserveSecondarySlotCopyWorkspace(u8 bank_index, u8 page_index);
extern void ObserveSecondarySlotWriteMarker(void);
extern void ObserveSecondarySlotCopyPayload(fw_addr_t destination, fw_addr_t source, u16 dword_count);
extern void ObserveSecondarySlotUpdateQ(void);
extern void ObserveSecondarySlotEnqueue(void);
extern void ObserveSecondarySlotIncrement(u16 tuple_address);
extern fw_addr_t ObserveSecondarySlotAdvanceRing(fw_addr_t cursor);
extern void ObserveSecondarySlotAdvanceQ(u8 reverse_relative);
extern u8 ObserveSecondarySlotWaitForRead(void);
extern void ObserveSecondarySlotLatchFailure(void);
extern void ObserveSecondarySlotSetNoSense(void);
extern void ObserveSecondarySlotReportFailure(u8 failure_code);
#define SECONDARY_RESUME_RING() ObserveSecondarySlotResumeRing()
#define SECONDARY_CONVERT(frame, destination) ObserveSecondarySlotConvert((frame), (destination))
#define SECONDARY_PACK(address) ObserveSecondarySlotPackBcd(address)
#define SECONDARY_COPY_TUPLE(source, destination, count) ObserveSecondarySlotCopyTuple((source), (destination), (count))
#define SECONDARY_WRITE_BLOCK(source) ObserveSecondarySlotWriteControllerBlock(source)
#define SECONDARY_SUBMIT(cursor, parameter) ObserveSecondarySlotSubmit((cursor), (parameter))
#define SECONDARY_BUILD_WORKSPACE(bank, page) ObserveSecondarySlotBuildWorkspace((bank), (page))
#define SECONDARY_COPY_WORKSPACE(bank, page) ObserveSecondarySlotCopyWorkspace((bank), (page))
#define SECONDARY_WRITE_MARKER() ObserveSecondarySlotWriteMarker()
#define SECONDARY_COPY_PAYLOAD(destination, source, count) ObserveSecondarySlotCopyPayload((destination), (source), (count))
#define SECONDARY_UPDATE_Q() ObserveSecondarySlotUpdateQ()
#define SECONDARY_ENQUEUE() ObserveSecondarySlotEnqueue()
#define SECONDARY_INCREMENT(address) ObserveSecondarySlotIncrement(address)
#define SECONDARY_ADVANCE_RING(cursor) ObserveSecondarySlotAdvanceRing(cursor)
#define SECONDARY_ADVANCE_Q(reverse) ObserveSecondarySlotAdvanceQ(reverse)
#define SECONDARY_WAIT_FOR_READ() ObserveSecondarySlotWaitForRead()
#define SECONDARY_LATCH_FAILURE() ObserveSecondarySlotLatchFailure()
#define SECONDARY_SET_NO_SENSE() ObserveSecondarySlotSetNoSense()
#define SECONDARY_REPORT_FAILURE(code) ObserveSecondarySlotReportFailure(code)
#else
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address);
extern void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address);
extern void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
extern void BuildControllerRecordWorkspacePage(u8 bank_index, u8 page_index);
extern void CopyControllerRecordWorkspacePageToBankedRegion(u8 bank_index, u8 page_index);
extern void WriteControllerBufferByte53AtOffsetA62F(void);
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
#define SECONDARY_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define SECONDARY_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define SECONDARY_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define SECONDARY_COPY_TUPLE(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define SECONDARY_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define SECONDARY_SUBMIT(cursor, parameter) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (parameter))
#define SECONDARY_BUILD_WORKSPACE(bank, page) BuildControllerRecordWorkspacePage((bank), (page))
#define SECONDARY_COPY_WORKSPACE(bank, page) CopyControllerRecordWorkspacePageToBankedRegion((bank), (page))
#define SECONDARY_WRITE_MARKER() WriteControllerBufferByte53AtOffsetA62F()
#define SECONDARY_COPY_PAYLOAD(destination, source, count) CopyDwordsToAddressPlus1MiB((destination), (source), (count))
#define SECONDARY_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define SECONDARY_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define SECONDARY_INCREMENT(address) IncrementPackedBcdMsf(address)
#define SECONDARY_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define SECONDARY_ADVANCE_Q(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define SECONDARY_WAIT_FOR_READ() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define SECONDARY_LATCH_FAILURE() LatchFirstReadFailureContext()
#define SECONDARY_SET_NO_SENSE() SetNoSenseData()
#define SECONDARY_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
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
