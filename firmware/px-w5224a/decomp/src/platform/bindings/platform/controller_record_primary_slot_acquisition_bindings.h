#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObservePrimarySlotResumeRing(void);
extern void ObservePrimarySlotConvert(s32 frame_position, fw_addr_t destination);
extern void ObservePrimarySlotPackBcd(u16 tuple_address);
extern void ObservePrimarySlotCopyFar(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObservePrimarySlotWriteControllerBlock(fw_addr_t source);
extern void ObservePrimarySlotSubmit(fw_addr_t cursor, u8 parameter);
extern void ObservePrimarySlotCopyQ(u16 destination, u16 source, u8 byte_count);
extern void ObservePrimarySlotBuildWorkspace(u8 bank_index, u8 page_index);
extern void ObservePrimarySlotCopyWorkspace(u8 bank_index, u8 page_index);
extern void ObservePrimarySlotCopyPayload(fw_addr_t destination, fw_addr_t source, u16 dword_count);
extern void ObservePrimarySlotUpdateQ(void);
extern void ObservePrimarySlotEnqueue(void);
extern void ObservePrimarySlotIncrement(u16 tuple_address);
extern fw_addr_t ObservePrimarySlotAdvanceRing(fw_addr_t cursor);
extern void ObservePrimarySlotIncrementRecord(u8 record_slot_index);
extern u8 ObservePrimarySlotWaitForRead(void);
extern void ObservePrimarySlotLatchFailure(void);
extern void ObservePrimarySlotSetNoSense(void);
extern void ObservePrimarySlotReportFailure(u8 failure_code);
#define PRIMARY_RESUME_RING() ObservePrimarySlotResumeRing()
#define PRIMARY_CONVERT(frame, destination) ObservePrimarySlotConvert((frame), (destination))
#define PRIMARY_PACK(address) ObservePrimarySlotPackBcd(address)
#define PRIMARY_COPY_FAR(source, destination, count) ObservePrimarySlotCopyFar((source), (destination), (count))
#define PRIMARY_WRITE_BLOCK(source) ObservePrimarySlotWriteControllerBlock(source)
#define PRIMARY_SUBMIT(cursor, parameter) ObservePrimarySlotSubmit((cursor), (parameter))
#define PRIMARY_COPY_Q(destination, source, count) ObservePrimarySlotCopyQ((destination), (source), (count))
#define PRIMARY_BUILD_WORKSPACE(bank, page) ObservePrimarySlotBuildWorkspace((bank), (page))
#define PRIMARY_COPY_WORKSPACE(bank, page) ObservePrimarySlotCopyWorkspace((bank), (page))
#define PRIMARY_COPY_PAYLOAD(destination, source, count) ObservePrimarySlotCopyPayload((destination), (source), (count))
#define PRIMARY_UPDATE_Q() ObservePrimarySlotUpdateQ()
#define PRIMARY_ENQUEUE() ObservePrimarySlotEnqueue()
#define PRIMARY_INCREMENT(address) ObservePrimarySlotIncrement(address)
#define PRIMARY_ADVANCE_RING(cursor) ObservePrimarySlotAdvanceRing(cursor)
#define PRIMARY_INCREMENT_RECORD(slot) ObservePrimarySlotIncrementRecord(slot)
#define PRIMARY_WAIT_FOR_READ() ObservePrimarySlotWaitForRead()
#define PRIMARY_LATCH_FAILURE() ObservePrimarySlotLatchFailure()
#define PRIMARY_SET_NO_SENSE() ObservePrimarySlotSetNoSense()
#define PRIMARY_REPORT_FAILURE(code) ObservePrimarySlotReportFailure(code)
#else
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address);
extern void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address);
extern void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
extern void CopyBytes16(u16 destination, u16 source, u8 byte_count);
extern void BuildControllerRecordWorkspacePage(u8 bank_index, u8 page_index);
extern void CopyControllerRecordWorkspacePageToBankedRegion(u8 bank_index, u8 page_index);
extern void CopyDwordsToAddressPlus1MiB(fw_addr_t destination_base, fw_addr_t source, u16 dword_count);
extern void UpdateRawReadQPacketControlBit(void);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern void IncrementPackedBcdMsf(u16 packed_bcd_msf_address);
extern fw_addr_t AdvanceRawReadRingPosition(fw_addr_t main_data_cursor);
extern void IncrementType2ControllerRecordSlotValue(u8 record_slot_index);
extern u8 WaitForReadAcquisitionCoroutineAndCheckPreparationStatus(void);
extern void LatchFirstReadFailureContext(void);
extern void SetNoSenseData(void);
extern void ReportReadPipelineFailure(u8 failure_code);
#define PRIMARY_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define PRIMARY_CONVERT(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define PRIMARY_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define PRIMARY_COPY_FAR(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define PRIMARY_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define PRIMARY_SUBMIT(cursor, parameter) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (parameter))
#define PRIMARY_COPY_Q(destination, source, count) CopyBytes16((destination), (source), (count))
#define PRIMARY_BUILD_WORKSPACE(bank, page) BuildControllerRecordWorkspacePage((bank), (page))
#define PRIMARY_COPY_WORKSPACE(bank, page) CopyControllerRecordWorkspacePageToBankedRegion((bank), (page))
#define PRIMARY_COPY_PAYLOAD(destination, source, count) CopyDwordsToAddressPlus1MiB((destination), (source), (count))
#define PRIMARY_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define PRIMARY_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define PRIMARY_INCREMENT(address) IncrementPackedBcdMsf(address)
#define PRIMARY_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define PRIMARY_INCREMENT_RECORD(slot) IncrementType2ControllerRecordSlotValue(slot)
#define PRIMARY_WAIT_FOR_READ() WaitForReadAcquisitionCoroutineAndCheckPreparationStatus()
#define PRIMARY_LATCH_FAILURE() LatchFirstReadFailureContext()
#define PRIMARY_SET_NO_SENSE() SetNoSenseData()
#define PRIMARY_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
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
