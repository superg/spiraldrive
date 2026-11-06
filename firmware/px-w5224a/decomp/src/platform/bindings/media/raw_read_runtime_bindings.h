#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveMediaReadControlResetRawReadQueue(void);
extern void ObserveMediaReadControlYieldCoroutine1(void);
extern s32 ObserveMediaReadControlConvertBinaryMsf(fw_addr_t address);
extern void ObserveMediaReadControlClearBits0017And0020(void);
extern void ObserveMediaReadControlClearBit003F(void);
extern void ObserveMediaReadControlInitializeCoroutine3(u32 entry_address);
#define READ_CONTROL_RESET_QUEUE() ObserveMediaReadControlResetRawReadQueue()
#define READ_CONTROL_YIELD() ObserveMediaReadControlYieldCoroutine1()
#define READ_CONTROL_CONVERT_MSF(address) ObserveMediaReadControlConvertBinaryMsf(address)
#define READ_CONTROL_CLEAR_BITS0017_0020() ObserveMediaReadControlClearBits0017And0020()
#define READ_CONTROL_CLEAR_BIT003F() ObserveMediaReadControlClearBit003F()
#define READ_CONTROL_INITIALIZE_COROUTINE3(entry) ObserveMediaReadControlInitializeCoroutine3(entry)
#else
extern void ResetRawReadQueueAndTransferState(void);
extern void YieldCoroutine1(void);
extern void InitializeCoroutine3(u32 entry_address);
extern void PLEXTOR_NEAR_FUNCTION ClearMediaReadControlBits0017And0020(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit003FAtomic(void);
#define READ_CONTROL_RESET_QUEUE() ResetRawReadQueueAndTransferState()
#define READ_CONTROL_YIELD() YieldCoroutine1()
#define READ_CONTROL_CONVERT_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define READ_CONTROL_CLEAR_BITS0017_0020() ClearMediaReadControlBits0017And0020()
#define READ_CONTROL_CLEAR_BIT003F() ClearBit003FAtomic()
#define READ_CONTROL_INITIALIZE_COROUTINE3(entry) InitializeCoroutine3(entry)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetRawReadPipelineBuffersAndState(void);
extern void ObserveInitializeRawReadSectorControlStateForRuntime(void);
extern void ObserveClearRawReadPipelineStatusStateForRuntime(void);
extern u8 ObserveReadMeasurementTargetPendingBit(void);
extern void ObserveSetReadMeasurementControllerActiveBit(void);
extern void ObserveClearReadMeasurementTargetPendingBit(void);
extern void ObserveResetControllerPortsForEventBit7Transfer(void);
extern u8 ObserveReadControllerBufferByteForEventBit7Transfer(fw_addr_t buffer_offset);
extern void ObserveResetRawReadQueueForEventBit7Transfer(void);
extern void ObserveDispatchReadAcquisitionRequest5ForEventBit7Transfer(void);
extern void ObserveClearEventBit7TransferBitsAtomic(void);
extern void ObserveSetRawReadTerminalBitForEventBit7Transfer(void);
extern void ObserveSetP3RxHighTxLowForEventBit7Transfer(void);
extern u32 ObserveCaptureInheritedDR12ForEventBit7Transfer(void);
extern void ObserveRestoreInheritedDR12ForEventBit7Transfer(u32 value);
extern void ObserveInitializeControllerBufferCursorsForProducerThrottle(void);
extern void ObserveInitializeRawReadSpeedForProducerThrottle(void);
extern void ObserveDispatchReadAcquisitionForProducerThrottle(void);
extern void ObserveYieldCoroutine2ForProducerThrottle(void);
extern void ObserveUpdateRawReadSpeedForProducerThrottle(void);
#define RESET_RAW_READ_PIPELINE_BUFFERS_AND_STATE() ObserveResetRawReadPipelineBuffersAndState()
#define INITIALIZE_RAW_READ_SECTOR_CONTROL_STATE() ObserveInitializeRawReadSectorControlStateForRuntime()
#define CLEAR_RAW_READ_PIPELINE_STATUS_STATE() ObserveClearRawReadPipelineStatusStateForRuntime()
#define READ_MEASUREMENT_TARGET_PENDING() ObserveReadMeasurementTargetPendingBit()
#define SET_READ_MEASUREMENT_CONTROLLER_ACTIVE() ObserveSetReadMeasurementControllerActiveBit()
#define CLEAR_READ_MEASUREMENT_TARGET_PENDING() ObserveClearReadMeasurementTargetPendingBit()
#define RESET_CONTROLLER_PORTS_FOR_EVENT_BIT7_TRANSFER() ObserveResetControllerPortsForEventBit7Transfer()
#define READ_CONTROLLER_BUFFER_BYTE_FOR_EVENT_BIT7_TRANSFER(offset) ObserveReadControllerBufferByteForEventBit7Transfer(offset)
#define RESET_RAW_READ_QUEUE_FOR_EVENT_BIT7_TRANSFER() ObserveResetRawReadQueueForEventBit7Transfer()
#define DISPATCH_REQUEST5_FOR_EVENT_BIT7_TRANSFER() ObserveDispatchReadAcquisitionRequest5ForEventBit7Transfer()
#define CLEAR_EVENT_BIT7_TRANSFER_BITS_ATOMIC() ObserveClearEventBit7TransferBitsAtomic()
#define SET_RAW_READ_TERMINAL_BIT_FOR_EVENT_BIT7_TRANSFER() ObserveSetRawReadTerminalBitForEventBit7Transfer()
#define SET_P3_RX_HIGH_TX_LOW_FOR_EVENT_BIT7_TRANSFER() ObserveSetP3RxHighTxLowForEventBit7Transfer()
#define CAPTURE_INHERITED_DR12_FOR_EVENT_BIT7_TRANSFER() ObserveCaptureInheritedDR12ForEventBit7Transfer()
#define RESTORE_INHERITED_DR12_FOR_EVENT_BIT7_TRANSFER(value) ObserveRestoreInheritedDR12ForEventBit7Transfer(value)
#define INITIALIZE_CONTROLLER_CURSORS_FOR_PRODUCER_THROTTLE() ObserveInitializeControllerBufferCursorsForProducerThrottle()
#define INITIALIZE_RAW_READ_SPEED_FOR_PRODUCER_THROTTLE() ObserveInitializeRawReadSpeedForProducerThrottle()
#define DISPATCH_READ_ACQUISITION_FOR_PRODUCER_THROTTLE() ObserveDispatchReadAcquisitionForProducerThrottle()
#define YIELD_COROUTINE2_FOR_PRODUCER_THROTTLE() ObserveYieldCoroutine2ForProducerThrottle()
#define UPDATE_RAW_READ_SPEED_FOR_PRODUCER_THROTTLE() ObserveUpdateRawReadSpeedForProducerThrottle()
#else
extern void ResetRawReadPipelineBuffersAndState(void);
extern void ResetControllerPorts8087Through808B(void);
extern u8 ReadControllerBufferByteAtomic(fw_addr_t buffer_offset);
extern void ResetRawReadQueueAndTransferState(void);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
extern void PLEXTOR_NEAR_FUNCTION ClearEventBit7TransferBitsAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetRawReadTerminalBitForEventBit7Transfer(void);
extern void PLEXTOR_NEAR_FUNCTION SetP3RxHighTxLowForEventBit7Transfer(void);
extern u32 PLEXTOR_NEAR_FUNCTION CaptureInheritedDR12ForEventBit7Transfer(void);
extern void PLEXTOR_NEAR_FUNCTION RestoreInheritedDR12ForEventBit7Transfer(u32 value);
extern void InitializeControllerBufferCursorRegistersIfNeeded(void);
extern void InitializeRawReadSpeedForCurrentDriveState(void);
extern void YieldCoroutine2(void);
extern void UpdateRawReadSpeedForCurrentPipelineState(void);
#define RESET_RAW_READ_PIPELINE_BUFFERS_AND_STATE() ResetRawReadPipelineBuffersAndState()
#define INITIALIZE_RAW_READ_SECTOR_CONTROL_STATE() InitializeRawReadSectorControlState()
#define CLEAR_RAW_READ_PIPELINE_STATUS_STATE() ClearRawReadPipelineStatusState()
#define READ_MEASUREMENT_TARGET_PENDING() FirmwareReadBit0034()
#define SET_READ_MEASUREMENT_CONTROLLER_ACTIVE() FirmwareWriteBit0043(1u)
#define CLEAR_READ_MEASUREMENT_TARGET_PENDING() FirmwareWriteBit0034(0u)
#define RESET_CONTROLLER_PORTS_FOR_EVENT_BIT7_TRANSFER() ResetControllerPorts8087Through808B()
#define READ_CONTROLLER_BUFFER_BYTE_FOR_EVENT_BIT7_TRANSFER(offset) ReadControllerBufferByteAtomic(offset)
#define RESET_RAW_READ_QUEUE_FOR_EVENT_BIT7_TRANSFER() ResetRawReadQueueAndTransferState()
#define DISPATCH_REQUEST5_FOR_EVENT_BIT7_TRANSFER() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define CLEAR_EVENT_BIT7_TRANSFER_BITS_ATOMIC() ClearEventBit7TransferBitsAtomic()
#define SET_RAW_READ_TERMINAL_BIT_FOR_EVENT_BIT7_TRANSFER() SetRawReadTerminalBitForEventBit7Transfer()
#define SET_P3_RX_HIGH_TX_LOW_FOR_EVENT_BIT7_TRANSFER() SetP3RxHighTxLowForEventBit7Transfer()
#define CAPTURE_INHERITED_DR12_FOR_EVENT_BIT7_TRANSFER() CaptureInheritedDR12ForEventBit7Transfer()
#define RESTORE_INHERITED_DR12_FOR_EVENT_BIT7_TRANSFER(value) RestoreInheritedDR12ForEventBit7Transfer(value)
#define INITIALIZE_CONTROLLER_CURSORS_FOR_PRODUCER_THROTTLE() InitializeControllerBufferCursorRegistersIfNeeded()
#define INITIALIZE_RAW_READ_SPEED_FOR_PRODUCER_THROTTLE() InitializeRawReadSpeedForCurrentDriveState()
#define DISPATCH_READ_ACQUISITION_FOR_PRODUCER_THROTTLE() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define YIELD_COROUTINE2_FOR_PRODUCER_THROTTLE() YieldCoroutine2()
#define UPDATE_RAW_READ_SPEED_FOR_PRODUCER_THROTTLE() UpdateRawReadSpeedForCurrentPipelineState()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveModePage0EBuildControllerShadows(void);
extern void ObserveModePage0EInitializeControllerCursors(void);
extern void ObserveModePage0ESetExternalInterrupt0(u8 enabled);
extern u16 ObserveModePage0ELatestDescriptorAddress(void);
extern void ObserveModePage0EYieldCoroutine0(void);
extern void ObserveModePage0EApplyControllerShadows(void);
#define MODE_PAGE_0E_BUILD_SHADOWS() ObserveModePage0EBuildControllerShadows()
#define MODE_PAGE_0E_INITIALIZE_CURSORS() ObserveModePage0EInitializeControllerCursors()
#define MODE_PAGE_0E_SET_EX0(enabled) ObserveModePage0ESetExternalInterrupt0(enabled)
#define MODE_PAGE_0E_LATEST_DESCRIPTOR() ObserveModePage0ELatestDescriptorAddress()
#define MODE_PAGE_0E_YIELD0() ObserveModePage0EYieldCoroutine0()
#define MODE_PAGE_0E_APPLY_SHADOWS() ObserveModePage0EApplyControllerShadows()
#else
extern void YieldCoroutine0(void);
#define MODE_PAGE_0E_BUILD_SHADOWS() BuildControllerPort8087And808BShadowsFromConfiguration()
#define MODE_PAGE_0E_INITIALIZE_CURSORS() InitializeControllerBufferCursorRegistersIfNeeded()
#define MODE_PAGE_0E_SET_EX0(enabled) DescriptorBatchSetExternalInterrupt0(enabled)
#define MODE_PAGE_0E_LATEST_DESCRIPTOR() GetLatestQueuedRawSectorDescriptorAddress()
#define MODE_PAGE_0E_YIELD0() YieldCoroutine0()
#define MODE_PAGE_0E_APPLY_SHADOWS() ApplyControllerPortShadowsForModeFlag0E()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDescriptorBatchResetRawReadQueue(void);
extern void ObserveDescriptorBatchInitializeCoroutine2(u32 entry_address);
extern void ObserveDescriptorBatchSetExternalInterrupt0(u8 enabled);
extern u16 ObserveDescriptorBatchLatestAddress(void);
extern u8 ObserveDescriptorBatchValidate(u16 descriptor_address);
extern void ObserveDescriptorBatchConsume(u16 descriptor_count);
#define DESCRIPTOR_BATCH_RESET_QUEUE() ObserveDescriptorBatchResetRawReadQueue()
#define DESCRIPTOR_BATCH_INITIALIZE_COROUTINE2(entry_address) ObserveDescriptorBatchInitializeCoroutine2(entry_address)
#define DESCRIPTOR_BATCH_SET_EX0(enabled) ObserveDescriptorBatchSetExternalInterrupt0(enabled)
#define DESCRIPTOR_BATCH_LATEST_ADDRESS() ObserveDescriptorBatchLatestAddress()
#define DESCRIPTOR_BATCH_VALIDATE(address) ObserveDescriptorBatchValidate(address)
#define DESCRIPTOR_BATCH_CONSUME(count) ObserveDescriptorBatchConsume(count)
#else
extern void PLEXTOR_NEAR_FUNCTION SetExternalInterrupt0EnableAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearExternalInterrupt0EnableAtomic(void);
extern u8 ValidateRawSectorDescriptorStatus(u16 descriptor_address);
extern void ConsumeRawReadDescriptors(u16 descriptor_count);
static void DescriptorBatchSetExternalInterrupt0(u8 enabled)
{
    if(enabled != 0u)
        SetExternalInterrupt0EnableAtomic();
    else
        ClearExternalInterrupt0EnableAtomic();
}
#define DESCRIPTOR_BATCH_RESET_QUEUE() ResetRawReadQueueAndTransferState()
#define DESCRIPTOR_BATCH_INITIALIZE_COROUTINE2(entry_address) InitializeCoroutine2(entry_address)
#define DESCRIPTOR_BATCH_SET_EX0(enabled) DescriptorBatchSetExternalInterrupt0(enabled)
#define DESCRIPTOR_BATCH_LATEST_ADDRESS() GetLatestQueuedRawSectorDescriptorAddress()
#define DESCRIPTOR_BATCH_VALIDATE(address) ValidateRawSectorDescriptorStatus(address)
#define DESCRIPTOR_BATCH_CONSUME(count) ConsumeRawReadDescriptors(count)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveHostTransferCountMappingBit(void);
extern u8 ObserveHostTransferCountValidateDescriptor(u16 descriptor_address);
extern s32 ObserveHostTransferCountMapLba(s32 logical_block_address);
#define HOST_TRANSFER_COUNT_MAPPING_BIT() ObserveHostTransferCountMappingBit()
#define HOST_TRANSFER_COUNT_VALIDATE(address) ObserveHostTransferCountValidateDescriptor(address)
#define HOST_TRANSFER_COUNT_MAP_LBA(lba) ObserveHostTransferCountMapLba(lba)
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit000CForHostTransferDescriptorCount(void);
extern u8 ValidateRawSectorDescriptorStatus(u16 descriptor_address);
extern s32 MapLogicalBlockAddressToControllerFramePosition(s32 logical_block_address);
#define HOST_TRANSFER_COUNT_MAPPING_BIT() ReadBit000CForHostTransferDescriptorCount()
#define HOST_TRANSFER_COUNT_VALIDATE(address) ValidateRawSectorDescriptorStatus(address)
#define HOST_TRANSFER_COUNT_MAP_LBA(lba) MapLogicalBlockAddressToControllerFramePosition(lba)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawReadProgressBit003D(void);
extern u8 ObserveRawReadProgressBit003E(void);
extern u8 ObserveRawReadProgressBit003F(void);
extern u8 ObserveRawReadProgressBit0040(void);
extern void ObserveResetRawReadProgressCountersAndPhaseFlags(void);
extern void ObserveResetRawReadQueueForProgressUpdate(void);
#define READ_RAW_READ_PROGRESS_BIT003D() ObserveRawReadProgressBit003D()
#define READ_RAW_READ_PROGRESS_BIT003E() ObserveRawReadProgressBit003E()
#define READ_RAW_READ_PROGRESS_BIT003F() ObserveRawReadProgressBit003F()
#define READ_RAW_READ_PROGRESS_BIT0040() ObserveRawReadProgressBit0040()
#define RESET_RAW_READ_PROGRESS_STATE() ObserveResetRawReadProgressCountersAndPhaseFlags()
#define RESET_RAW_READ_QUEUE_FOR_PROGRESS() ObserveResetRawReadQueueForProgressUpdate()
#else
extern u8 ReadBit003DForRawReadProgress(void);
extern u8 ReadBit003EForRawReadProgress(void);
extern u8 ReadBit003FForRawReadProgress(void);
extern u8 ReadBit0040ForRawReadProgress(void);
extern void ResetRawReadProgressCountersAndPhaseFlags(void);
#define READ_RAW_READ_PROGRESS_BIT003D() ReadBit003DForRawReadProgress()
#define READ_RAW_READ_PROGRESS_BIT003E() ReadBit003EForRawReadProgress()
#define READ_RAW_READ_PROGRESS_BIT003F() ReadBit003FForRawReadProgress()
#define READ_RAW_READ_PROGRESS_BIT0040() ReadBit0040ForRawReadProgress()
#define RESET_RAW_READ_PROGRESS_STATE() ResetRawReadProgressCountersAndPhaseFlags()
#define RESET_RAW_READ_QUEUE_FOR_PROGRESS() ResetRawReadQueueAndTransferState()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveMediaReadInitClearWorkingTrack(void);
extern void ObserveMediaReadInitResetTrackTable(void);
extern void ObserveMediaReadInitUpdateSessionReference(void);
extern void ObserveMediaReadInitPopulateFullTocTracks(void);
extern void ObserveMediaReadInitSessionBounds(void);
extern void ObserveMediaReadInitWorkingTrack(void);
extern void ObserveMediaReadInitSessionRawQTracks(void);
extern void ObserveMediaReadInitResetType2Pipeline(void);
extern void ObserveMediaReadInitPrepareType2Pipeline(void);
extern u16 ObserveMediaReadInitScanControllerBanks(u8 force_bank_scan, u32 frame_count, s32 start_frame_position);
extern u8 ObserveMediaReadInitThirtySevenTrackLayout(void);
extern void ObserveMediaReadInitSetLayoutBit(u8 enabled);
extern void ObserveMediaReadInitResetRawRead(void);
extern void ObserveMediaReadInitCoroutine2(u32 entry_address);
extern void ObserveMediaReadInitYieldCoroutine3(void);
extern void ObserveMediaReadInitSnapshotTrackTail(void);
#define MEDIA_READ_INIT_CLEAR_WORKING_TRACK() ObserveMediaReadInitClearWorkingTrack()
#define MEDIA_READ_INIT_RESET_TRACK_TABLE() ObserveMediaReadInitResetTrackTable()
#define MEDIA_READ_INIT_UPDATE_SESSION_REFERENCE() ObserveMediaReadInitUpdateSessionReference()
#define MEDIA_READ_INIT_POPULATE_FULL_TOC_TRACKS() ObserveMediaReadInitPopulateFullTocTracks()
#define MEDIA_READ_INIT_SESSION_BOUNDS() ObserveMediaReadInitSessionBounds()
#define MEDIA_READ_INIT_WORKING_TRACK() ObserveMediaReadInitWorkingTrack()
#define MEDIA_READ_INIT_SESSION_RAW_Q_TRACKS() ObserveMediaReadInitSessionRawQTracks()
#define MEDIA_READ_INIT_RESET_TYPE2_PIPELINE() ObserveMediaReadInitResetType2Pipeline()
#define MEDIA_READ_INIT_PREPARE_TYPE2_PIPELINE() ObserveMediaReadInitPrepareType2Pipeline()
#define MEDIA_READ_INIT_SCAN_BANKS(force_scan, frame_count, start_frame) ObserveMediaReadInitScanControllerBanks((force_scan), (frame_count), (start_frame))
#define MEDIA_READ_INIT_LAYOUT_RESULT() ObserveMediaReadInitThirtySevenTrackLayout()
#define MEDIA_READ_INIT_SET_LAYOUT_BIT(enabled) ObserveMediaReadInitSetLayoutBit(enabled)
#define MEDIA_READ_INIT_RESET_RAW_READ() ObserveMediaReadInitResetRawRead()
#define MEDIA_READ_INIT_COROUTINE2(entry_address) ObserveMediaReadInitCoroutine2(entry_address)
#define MEDIA_READ_INIT_YIELD3() ObserveMediaReadInitYieldCoroutine3()
#define MEDIA_READ_INIT_SNAPSHOT_TRACK_TAIL() ObserveMediaReadInitSnapshotTrackTail()
#else
extern void ClearWorkingTrackInformation(void);
extern void PopulateTrackTableFromCurrentSessionRawQRecords(void);
extern void ResetType2ControllerRecordPipelineState(void);
extern void PLEXTOR_NEAR_FUNCTION PopulateTrackTableFromFullTocTrackRange(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeWorkingTrackFromReadPipelineState(void);
extern void InitializeCoroutine2(u32 entry_address);
extern void YieldCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit003FAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit003FAtomic(void);
static void MediaReadInitSetLayoutBit(u8 enabled)
{
    if(enabled != 0u)
        SetBit003FAtomic();
    else
        ClearBit003FAtomic();
}
#define MEDIA_READ_INIT_CLEAR_WORKING_TRACK() ClearWorkingTrackInformation()
#define MEDIA_READ_INIT_RESET_TRACK_TABLE() ResetTrackInformationTableState()
#define MEDIA_READ_INIT_UPDATE_SESSION_REFERENCE() UpdateTrackTableReferenceFromCurrentSessionPointB0()
#define MEDIA_READ_INIT_POPULATE_FULL_TOC_TRACKS() PopulateTrackTableFromFullTocTrackRange()
#define MEDIA_READ_INIT_SESSION_BOUNDS() InitializeTrackTableSessionFrameBounds()
#define MEDIA_READ_INIT_WORKING_TRACK() InitializeWorkingTrackFromReadPipelineState()
#define MEDIA_READ_INIT_SESSION_RAW_Q_TRACKS() PopulateTrackTableFromCurrentSessionRawQRecords()
#define MEDIA_READ_INIT_RESET_TYPE2_PIPELINE() ResetType2ControllerRecordPipelineState()
#define MEDIA_READ_INIT_PREPARE_TYPE2_PIPELINE() PrepareType2ControllerRecordPipelineFromDetectedLayout()
#define MEDIA_READ_INIT_SCAN_BANKS(force_scan, frame_count, start_frame) ScanControllerRecordBanksForFrameRange((force_scan), (start_frame), (frame_count))
#define MEDIA_READ_INIT_LAYOUT_RESULT() IsThirtySevenTrackLayoutSpacedBy9000Frames()
#define MEDIA_READ_INIT_SET_LAYOUT_BIT(enabled) MediaReadInitSetLayoutBit(enabled)
#define MEDIA_READ_INIT_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define MEDIA_READ_INIT_COROUTINE2(entry_address) InitializeCoroutine2(entry_address)
#define MEDIA_READ_INIT_YIELD3() YieldCoroutine3()
#define MEDIA_READ_INIT_SNAPSHOT_TRACK_TAIL() SnapshotTrackTableTailState()
#endif
#endif
