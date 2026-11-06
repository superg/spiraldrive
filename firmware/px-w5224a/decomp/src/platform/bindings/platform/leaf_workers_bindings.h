#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define READ_POSITION_P1_BIT3() ((FirmwareReadDirectSfr8(90) & 0x08u) != 0u)
#define WRITE_READ_POSITION_TH1(value) FirmwareWriteDirectSfr8(8D, (value))
#define WRITE_READ_POSITION_TL1(value) FirmwareWriteDirectSfr8(8B, (value))
#define READ_POSITION_TIMER_OVERFLOW() ((FirmwareReadDirectSfr8(88) & 0x80u) != 0u)
#define CLEAR_READ_POSITION_TIMER_OVERFLOW() FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) & 0x7fu))
#define START_READ_POSITION_TIMER() FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) | 0x40u))
#define STOP_READ_POSITION_TIMER() FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) & 0xbfu))
#else
sfr ReadPositionTimerHigh = 0x8d;
sfr ReadPositionTimerLow = 0x8b;
sbit ReadPositionP1Bit3 = 0x93;
sbit ReadPositionTimerRun = 0x8e;
sbit ReadPositionTimerOverflow = 0x8f;
#define READ_POSITION_P1_BIT3() (ReadPositionP1Bit3 != 0)
#define WRITE_READ_POSITION_TH1(value) (ReadPositionTimerHigh = (value))
#define WRITE_READ_POSITION_TL1(value) (ReadPositionTimerLow = (value))
#define READ_POSITION_TIMER_OVERFLOW() (ReadPositionTimerOverflow != 0)
#define CLEAR_READ_POSITION_TIMER_OVERFLOW() (ReadPositionTimerOverflow = 0)
#define START_READ_POSITION_TIMER() (ReadPositionTimerRun = 1)
#define STOP_READ_POSITION_TIMER() (ReadPositionTimerRun = 0)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define CAPTURE_RAW_SECTOR_FROM_C(address, source_kind) CaptureRawSectorToControllerBufferAndWait((address), (source_kind))
extern u8 DispatchBuildAndAcquireRawReadQRecordTableForSequence(u8 first_record_index);
extern void LoadControllerCalibrationConfigurationStateForGuard(void);
#define DispatchLoadControllerCalibrationConfigurationStateForGuard() LoadControllerCalibrationConfigurationStateForGuard()
extern u8 DispatchAcquireRawReadFrameBatchWithInterleavedQForFinalize(u32 frame_count);
#else
extern u8 CaptureRawSectorToControllerBufferAndWaitForC(u8 raw_sector_source_kind, fw_addr_t controller_buffer_address);
#define CAPTURE_RAW_SECTOR_FROM_C(address, source_kind) CaptureRawSectorToControllerBufferAndWaitForC((source_kind), (address))
#define DispatchBuildAndAcquireRawReadQRecordTableForSequence(index) BuildAndAcquireRawReadQRecordTable(index)
#define DispatchLoadControllerCalibrationConfigurationStateForGuard() LoadControllerCalibrationConfigurationState()
#define DispatchAcquireRawReadFrameBatchWithInterleavedQForFinalize(count) AcquireRawReadFrameBatchWithInterleavedQRecords(count)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveConfigureControllerPort8006AndChannel3ForState5Calibration(u8 enabled);
#define DispatchConfigureControllerPort8006AndChannel3ForState5Calibration(enabled) ObserveConfigureControllerPort8006AndChannel3ForState5Calibration(enabled)
#else
#define DispatchConfigureControllerPort8006AndChannel3ForState5Calibration(enabled) ConfigureControllerPort8006AndChannel3(enabled)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWriteControllerBufferByteForAddressingTest(fw_addr_t buffer_offset, u8 value);
extern u8 ObserveReadControllerBufferByteForAddressingTest(fw_addr_t buffer_offset);
extern u8 ObserveControllerBufferBankStridePatternForAddressingTest(u8 starting_pattern);
#define DispatchWriteControllerBufferByteForAddressingTest(offset, value) ObserveWriteControllerBufferByteForAddressingTest((offset), (value))
#define DispatchReadControllerBufferByteForAddressingTest(offset) ObserveReadControllerBufferByteForAddressingTest(offset)
#define DispatchControllerBufferBankStridePatternForAddressingTest(pattern) ObserveControllerBufferBankStridePatternForAddressingTest(pattern)
#else
#define DispatchWriteControllerBufferByteForAddressingTest(offset, value) WriteControllerBufferWindowByteAtomic((offset), (value))
#define DispatchReadControllerBufferByteForAddressingTest(offset) ReadControllerBufferByteAtomic(offset)
#define DispatchControllerBufferBankStridePatternForAddressingTest(pattern) TestControllerBufferBankStridePattern(pattern)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 5
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveProbeControllerRecordForType2Tdi(s32 frame_position);
#define DispatchProbeControllerRecordForType2Tdi(frame_position) ObserveProbeControllerRecordForType2Tdi(frame_position)
#else
#define DispatchProbeControllerRecordForType2Tdi(frame_position) ProbeControllerRecordAtFrameAndMapStatus(frame_position)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 6
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFinalizeTrackStateForWriteFailure(u8 scsi_opcode);
extern void ObserveUpdateTrackTableForWriteFailure(void);
#define DispatchFinalizeTrackStateForWriteFailure(opcode) ObserveFinalizeTrackStateForWriteFailure(opcode)
#define DispatchUpdateTrackTableForWriteFailure() ObserveUpdateTrackTableForWriteFailure()
#else
#define DispatchFinalizeTrackStateForWriteFailure(opcode) FinalizeTrackStateForScsiOpcode(opcode)
#define DispatchUpdateTrackTableForWriteFailure() UpdateTrackTableFromWorkingTrack()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 7
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveWritePipelineControllerRecordPrepareStage(u8 stage, u8 argument);
#define DispatchAcquireRequiredRecordRunsForWritePrepare(mode) ((void)ObserveWritePipelineControllerRecordPrepareStage(0u, (mode)))
#define DispatchRecalculateType2GeometryForWritePrepare() ((void)ObserveWritePipelineControllerRecordPrepareStage(1u, 0u))
#define DispatchBuildType2LeadInForWritePrepare(argument, stage) ((void)ObserveWritePipelineControllerRecordPrepareStage((stage), (argument)))
#define DispatchPreparePrimaryType2ForWritePrepare(mode) ObserveWritePipelineControllerRecordPrepareStage(3u, (mode))
#define DispatchAcquireType2BaseWindowForWritePrepare() ObserveWritePipelineControllerRecordPrepareStage(4u, 0u)
#define DispatchPrepareSecondaryType2ForWritePrepare(mode) ObserveWritePipelineControllerRecordPrepareStage(5u, (mode))
#define DispatchServiceFirstType2ForWritePrepare() ObserveWritePipelineControllerRecordPrepareStage(6u, 0u)
#define DispatchAcquireType2WindowForWritePrepare() ObserveWritePipelineControllerRecordPrepareStage(7u, 0u)
#define DispatchInitializeRawQForWritePrepare() ObserveWritePipelineControllerRecordPrepareStage(8u, 0u)
#define DispatchAcquireType2LeadInForWritePrepare() ObserveWritePipelineControllerRecordPrepareStage(10u, 0u)
#define DispatchAcquireType2LeadOutForWritePrepare() ObserveWritePipelineControllerRecordPrepareStage(11u, 0u)
#define DispatchBuildActiveBoundaryForWritePrepare() ((void)ObserveWritePipelineControllerRecordPrepareStage(12u, 0u))
#else
#define DispatchAcquireRequiredRecordRunsForWritePrepare(mode) AcquireRequiredControllerRecordIndexRuns(mode)
#define DispatchRecalculateType2GeometryForWritePrepare() RecalculateType2ControllerRecordGeometryWindow()
#define DispatchBuildType2LeadInForWritePrepare(argument, stage) BuildType2LeadInTocRecordSequence(argument)
#define DispatchPreparePrimaryType2ForWritePrepare(mode) PreparePrimaryType2ControllerRecordSlots(mode)
#define DispatchAcquireType2BaseWindowForWritePrepare() AcquireFortyFrameType2ControllerRecordBaseWindow()
#define DispatchPrepareSecondaryType2ForWritePrepare(mode) PrepareSecondaryType2ControllerRecordSlots(mode)
#define DispatchServiceFirstType2ForWritePrepare() ServiceFirstPendingType2ControllerRecord()
#define DispatchAcquireType2WindowForWritePrepare() AcquireFortyFrameType2ControllerRecordWindow()
#define DispatchInitializeRawQForWritePrepare() InitializeAndAcquireRawQSequenceFromIndex1()
#define DispatchAcquireType2LeadInForWritePrepare() AcquireType2LeadInControllerRecordSequence()
#define DispatchAcquireType2LeadOutForWritePrepare() AcquireType2LeadOutControllerRecordSequence()
#define DispatchBuildActiveBoundaryForWritePrepare() BuildActiveControllerRecordBoundaryTable()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 8
#if defined(PLEXTOR_HOST_TEST)
extern void FirmwareWriteBit0011(u8 value);
extern u8 FirmwareReadBit0016(void);
extern u8 FirmwareReadBit0017(void);
extern u8 FirmwareReadBit0021(void);
extern void FirmwareWriteBit0013(u8 value);
extern void FirmwareWriteBit0016(u8 value);
extern void FirmwareWriteBit0017(u8 value);
extern void FirmwareWriteBit001C(u8 value);
extern void FirmwareWriteBit001D(u8 value);
static void ClearP1Bits0And1Atomic(void)
{
    FirmwareWriteDirectSfr8(90, (u8)(FirmwareReadDirectSfr8(90) & 0xfcu));
}
static void SelectP1Bit0OnlyAtomic(void)
{
    FirmwareWriteDirectSfr8(90, (u8)((FirmwareReadDirectSfr8(90) & 0xfcu) | 1u));
}
extern void DispatchClearControllerDataPathBits31And33(void);
extern void DispatchClearReadMeasurementPhaseBits2BAnd43(void);
extern u8 DispatchCopyFixedResponse52AndExecuteDataIn(fw_addr_t source_address, u32 byte_count);
extern void DispatchSetNoSenseForFixedResponse52(void);
#else
extern void PLEXTOR_NEAR_FUNCTION ClearBit0011Atomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0016ForRangeValidation(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0017ForRangeValidation(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0021ForAlternateRangeValidation(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBits0016Then0017ForAlternateRangeValidationAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBits0016And0017ForRangeValidationAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0013ForRangeValidationAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit001CForRangeValidationAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit001DForRangeValidationAtomic(void);
extern void ClearP1Bits0And1Atomic(void);
extern void SelectP1Bit0OnlyAtomic(void);
extern void ClearControllerDataPathBits31And33Atomic(void);
#define DispatchClearControllerDataPathBits31And33 ClearControllerDataPathBits31And33Atomic
extern void ClearReadMeasurementPhaseBits2BAnd43Atomic(void);
#define DispatchClearReadMeasurementPhaseBits2BAnd43 ClearReadMeasurementPhaseBits2BAnd43Atomic
static u8 DispatchCopyFixedResponse52AndExecuteDataIn(fw_addr_t source_address, u32 byte_count)
{
    return CopyResponseToControllerBufferAndExecuteDataIn(source_address, byte_count);
}
static void DispatchSetNoSenseForFixedResponse52(void)
{
    SetNoSenseData();
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 9
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRunOpcForSelectedWriteSpeed(void);
#define DispatchRunOpcForSelectedWriteSpeed() ObserveRunOpcForSelectedWriteSpeed()
#else
extern u8 RunOpcForCurrentWriteSpeed(void);
#define DispatchRunOpcForSelectedWriteSpeed() RunOpcForCurrentWriteSpeed()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 10
#if defined(PLEXTOR_HOST_TEST)
extern u8 DoesCurrentDecodedFrameMatchFullTocPointA2ForDispatch(void);
#else
extern u8 PLEXTOR_NEAR_FUNCTION DoesCurrentDecodedFrameMatchFullTocPointA2(void);
#define DoesCurrentDecodedFrameMatchFullTocPointA2ForDispatch() DoesCurrentDecodedFrameMatchFullTocPointA2()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 11
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePrepareAndDispatchReadOrSeekAcquisitionAtLba(s32 lba);
#define DispatchPrepareAndDispatchReadOrSeekAcquisitionAtLba(lba) ObservePrepareAndDispatchReadOrSeekAcquisitionAtLba(lba)
#else
extern void PrepareAndDispatchReadOrSeekAcquisitionAtLba(s32 lba);
#define DispatchPrepareAndDispatchReadOrSeekAcquisitionAtLba(lba) PrepareAndDispatchReadOrSeekAcquisitionAtLba(lba)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 12
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetPmaQRecordTablesFromTrackIndex(u8 first_track_index);
#define DispatchResetPmaQRecordTablesFromTrackIndex(first_track_index) ObserveResetPmaQRecordTablesFromTrackIndex(first_track_index)
#else
#define DispatchResetPmaQRecordTablesFromTrackIndex(first_track_index) ResetPmaQRecordTablesFromTrackIndex(first_track_index)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 13
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveClassifyRawSectorPayloadSignatureAtPipelineTarget(void);
#define DispatchClassifyRawSectorPayloadSignatureAtPipelineTarget() ObserveClassifyRawSectorPayloadSignatureAtPipelineTarget()
#else
extern u8 ClassifyRawSectorPayloadSignatureAtPipelineTarget(void);
#define DispatchClassifyRawSectorPayloadSignatureAtPipelineTarget() ClassifyRawSectorPayloadSignatureAtPipelineTarget()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 14
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetDriveControlStateToEight(void);
#define ResetDriveControlStateToEight ObserveResetDriveControlStateToEight
#else
extern void ResetDriveControlStateToEight(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 15
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCaptureControllerD8D9NormalizedDifferenceSample(void);
#define DispatchCaptureControllerD8D9NormalizedDifferenceSample() ObserveCaptureControllerD8D9NormalizedDifferenceSample()
#else
extern void PLEXTOR_NEAR_FUNCTION CaptureControllerD8D9NormalizedDifferenceSample(void);
#define DispatchCaptureControllerD8D9NormalizedDifferenceSample() CaptureControllerD8D9NormalizedDifferenceSample()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 16
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveDecodeAtipTimeFieldsForPacketValidation(void);
#define DispatchDecodeAtipTimeFieldsForPacketValidation() ObserveDecodeAtipTimeFieldsForPacketValidation()
#else
extern void DecodeAtipTimeFieldsAndFramePositions(void);
#define DispatchDecodeAtipTimeFieldsForPacketValidation() DecodeAtipTimeFieldsAndFramePositions()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 17
#if defined(PLEXTOR_HOST_TEST)
extern u8 DispatchCaptureInheritedOffsetForAtipDecode(void);
extern void DispatchApplyMediaStrategyForAtipDecode(void);
extern void DispatchPublishMediaStrategyOffsetForAtipDecode(u8 inherited_offset);
#else
extern u8 CaptureInheritedR15ForAtipDecode(void);
extern void SelectAndApplyMediaStrategyFromAtipLeadInTime(void);
#define DispatchCaptureInheritedOffsetForAtipDecode() CaptureInheritedR15ForAtipDecode()
#define DispatchApplyMediaStrategyForAtipDecode() SelectAndApplyMediaStrategyFromAtipLeadInTime()
#define DispatchPublishMediaStrategyOffsetForAtipDecode(offset) SelectAndPublishMediaStrategyOffsetFromAtipClass(offset)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 18
#if defined(PLEXTOR_HOST_TEST)
extern u8 DispatchFindFirstTrackForBasicDiscInformation(void);
extern u8 DispatchSelectDiscTypeForBasicDiscInformation(void);
#else
extern u8 PLEXTOR_NEAR_FUNCTION FindFirstTrackNumberForLastFullTocSession(void);
extern u8 PLEXTOR_NEAR_FUNCTION SelectDiscInformationDiscTypeFromFullToc(void);
#define DispatchFindFirstTrackForBasicDiscInformation() FindFirstTrackNumberForLastFullTocSession()
#define DispatchSelectDiscTypeForBasicDiscInformation() SelectDiscInformationDiscTypeFromFullToc()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 19
#if defined(PLEXTOR_HOST_TEST)
extern u8 DispatchValidateReadTrackInformationCdb(fw_addr_t mask_descriptor);
extern void DispatchUnsupportedReadTrackInformation(void);
extern u16 DispatchReadTrackInformationAllocationLength(fw_addr_t address);
extern u16 DispatchClampReadTrackInformationLength(u16 value, u16 limit);
extern u8 DispatchBuildReadTrackInformationFromFullToc(void);
extern u8 DispatchBuildReadTrackInformationFromTrackState(void);
extern u8 DispatchInitializeMediaCapacityForReadTrackInformation(void);
extern u8 DispatchTransferReadTrackInformationResponse(fw_addr_t source_address, u32 byte_count);
extern void DispatchSetNoSenseForReadTrackInformation(void);
#else
extern u8 PLEXTOR_NEAR_FUNCTION BuildReadTrackInformationResponseFromFullToc(void);
extern u8 PLEXTOR_NEAR_FUNCTION BuildReadTrackInformationResponse(void);
#define DispatchValidateReadTrackInformationCdb(mask) ValidateScsiCdbFieldMasks(mask)
#define DispatchUnsupportedReadTrackInformation() HandleUnsupportedScsiCommand()
#define DispatchReadTrackInformationAllocationLength(address) ReadBigEndianU16(address)
#define DispatchClampReadTrackInformationLength(value, limit) MinU16((value), (limit))
#define DispatchBuildReadTrackInformationFromFullToc() BuildReadTrackInformationResponseFromFullToc()
#define DispatchBuildReadTrackInformationFromTrackState() BuildReadTrackInformationResponse()
#define DispatchInitializeMediaCapacityForReadTrackInformation() InitializeMediaCapacityGeometryResponse()
#define DispatchTransferReadTrackInformationResponse(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define DispatchSetNoSenseForReadTrackInformation() SetNoSenseData()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 20
#if defined(PLEXTOR_HOST_TEST)
extern s32 DispatchConvertMsfForReadTrackInformationTrackSize(fw_addr_t tuple_address);
extern s32 DispatchComputeFullTocTrackSpanForTrackSize(u8 track_number);
#else
#define DispatchConvertMsfForReadTrackInformationTrackSize(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define DispatchComputeFullTocTrackSpanForTrackSize(track) ComputeFullTocTrackSpanFrames(track)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 21
#if defined(PLEXTOR_HOST_TEST)
extern void DispatchClearBit0030ForWorkingTrackFinalizer(void);
extern void DispatchResetRawReadForWorkingTrackFinalizer(void);
extern void DispatchInitializeCoroutine2ForWorkingTrackFinalizer(u32 entry_point);
extern void DispatchYieldCoroutine0ForWorkingTrackFinalizer(void);
extern u8 DispatchEnsureOpcForWorkingTrackFinalizer(void);
extern u8 DispatchPrepareWorkingTrackRawReadRangeForFinalizer(void);
extern void DispatchUpdateWorkingTrackAfterWriteFailureForFinalizer(void);
extern void DispatchFinalizeTrackStateForWorkingTrackFinalizer(u8 scsi_opcode);
#else
extern void ClearBit0030Atomic(void);
extern void UpdateWorkingTrackAfterWriteFailure(void);
#define DispatchClearBit0030ForWorkingTrackFinalizer() ClearBit0030Atomic()
#define DispatchResetRawReadForWorkingTrackFinalizer() ResetRawReadQueueAndTransferState()
#define DispatchInitializeCoroutine2ForWorkingTrackFinalizer(entry) InitializeCoroutine2(entry)
#define DispatchYieldCoroutine0ForWorkingTrackFinalizer() YieldCoroutine0()
#define DispatchEnsureOpcForWorkingTrackFinalizer() EnsureOpcForSelectedWriteSpeed()
#define DispatchPrepareWorkingTrackRawReadRangeForFinalizer() PrepareAndAcquireWorkingTrackRawReadRange()
#define DispatchUpdateWorkingTrackAfterWriteFailureForFinalizer() UpdateWorkingTrackAfterWriteFailure()
#define DispatchFinalizeTrackStateForWorkingTrackFinalizer(opcode) FinalizeTrackStateForScsiOpcode(opcode)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 22
#if defined(PLEXTOR_HOST_TEST)
extern void DispatchNormalizeWriteSpeedForPipelinePreparation(void);
extern void DispatchResetRawReadForPipelinePreparation(void);
extern void DispatchInitializeCoroutine2ForPipelinePreparation(u32 entry_point);
extern void DispatchYieldCoroutine0ForPipelinePreparation(void);
extern void DispatchPrepareWritePipelineControllerRecordState(void);
#else
extern u8 PrepareWritePipelineControllerRecordState(void);
#define DispatchNormalizeWriteSpeedForPipelinePreparation() NormalizeWriteSpeedForCurrentMedia()
#define DispatchResetRawReadForPipelinePreparation() ResetRawReadQueueAndTransferState()
#define DispatchInitializeCoroutine2ForPipelinePreparation(entry) InitializeCoroutine2(entry)
#define DispatchYieldCoroutine0ForPipelinePreparation() YieldCoroutine0()
#define DispatchPrepareWritePipelineControllerRecordState() ((void)PrepareWritePipelineControllerRecordState())
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 23
#if defined(PLEXTOR_HOST_TEST)
extern void DispatchInitializeControllerAsicForDriveSubsystems(void);
extern void DispatchZeroMemoryForDriveSubsystems(fw_addr_t destination, u16 byte_count);
extern void DispatchFillControllerBufferForDriveSubsystems(fw_addr_t buffer_address, u32 byte_count, u8 value);
extern void DispatchSelectMainInterruptHandlersForDriveSubsystems(void);
extern void DispatchInitializeRuntimeDriveStateForDriveSubsystems(void);
extern void DispatchInitializeDefaultProfileForDriveSubsystems(void);
extern void DispatchLoadFixedControllerTablesForDriveSubsystems(void);
extern void DispatchInitializePickupParametersForDriveSubsystems(void);
extern void DispatchResetControllerStateForDriveSubsystems(void);
extern void DispatchInitializeDriveConfigurationForDriveSubsystems(void);
extern void DispatchInitializeDiscControlForDriveSubsystems(void);
#else
extern void InitializeControllerAsic(void);
extern void SelectMainFirmwareInterruptHandlers(void);
extern void InitializeRuntimeDriveState(void);
extern void InitializeDriveConfigurationState(void);
#define DispatchInitializeControllerAsicForDriveSubsystems() InitializeControllerAsic()
#define DispatchZeroMemoryForDriveSubsystems(destination, count) ZeroMemoryWords((destination), (count))
#define DispatchFillControllerBufferForDriveSubsystems(address, count, value) FillControllerBufferRange((address), (count), (value))
#define DispatchSelectMainInterruptHandlersForDriveSubsystems() SelectMainFirmwareInterruptHandlers()
#define DispatchInitializeRuntimeDriveStateForDriveSubsystems() InitializeRuntimeDriveState()
#define DispatchInitializeDefaultProfileForDriveSubsystems() InitializeDefaultControllerOperatingProfile()
#define DispatchLoadFixedControllerTablesForDriveSubsystems() LoadFixedControllerParameterTables()
#define DispatchInitializePickupParametersForDriveSubsystems() InitializePickupSeekControlParameters()
#define DispatchResetControllerStateForDriveSubsystems() ResetControllerOperatingState()
#define DispatchInitializeDriveConfigurationForDriveSubsystems() InitializeDriveConfigurationState()
#define DispatchInitializeDiscControlForDriveSubsystems() InitializeDiscControlStateFromConfiguration()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 24
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveStartFormat2RawSectorReadAtFramePosition(s32 frame_position);
extern void ObserveClearFormat2RawReadModeBits(void);
extern void ObserveSetFormat2RawReadModeBit(void);
extern void ObserveClearFormat2RawReadModeBit(void);
extern void ObserveResetControllerPortsForFormat2RawRead(void);
extern void ObserveExecuteRawSectorReadRangeForFormat2(s32 start_frame_position, s32 end_frame_position_exclusive, u8 format_code);
extern void ObserveBuildControllerShadowsForFormat2RawRead(void);
extern void ObserveApplyControllerShadowsForFormat2RawRead(void);
#define DispatchStartFormat2RawSectorReadAtFramePosition(position) ObserveStartFormat2RawSectorReadAtFramePosition(position)
#define DispatchClearFormat2RawReadModeBits() ObserveClearFormat2RawReadModeBits()
#define DispatchSetFormat2RawReadModeBit() ObserveSetFormat2RawReadModeBit()
#define DispatchClearFormat2RawReadModeBit() ObserveClearFormat2RawReadModeBit()
#define DispatchResetControllerPortsForFormat2RawRead() ObserveResetControllerPortsForFormat2RawRead()
#define DispatchExecuteRawSectorReadRangeForFormat2(start, end, format) ObserveExecuteRawSectorReadRangeForFormat2(start, end, format)
#define DispatchBuildControllerShadowsForFormat2RawRead() ObserveBuildControllerShadowsForFormat2RawRead()
#define DispatchApplyControllerShadowsForFormat2RawRead() ObserveApplyControllerShadowsForFormat2RawRead()
#else
#define DispatchStartFormat2RawSectorReadAtFramePosition(position) StartFormat2RawSectorReadAtFramePosition(position)
extern void ClearFormat2RawReadModeBitsAtomic(void);
extern void SetFormat2RawReadModeBitAtomic(void);
extern void ClearFormat2RawReadModeBitAtomic(void);
#define DispatchClearFormat2RawReadModeBits() ClearFormat2RawReadModeBitsAtomic()
#define DispatchSetFormat2RawReadModeBit() SetFormat2RawReadModeBitAtomic()
#define DispatchClearFormat2RawReadModeBit() ClearFormat2RawReadModeBitAtomic()
#define DispatchResetControllerPortsForFormat2RawRead() ResetControllerPorts8087Through808B()
#define DispatchExecuteRawSectorReadRangeForFormat2(start, end, format) ExecuteRawSectorReadRange(start, end, format)
#define DispatchBuildControllerShadowsForFormat2RawRead() BuildControllerPort8087And808BShadowsFromConfiguration()
#define DispatchApplyControllerShadowsForFormat2RawRead() ApplyControllerPortShadowsForModeFlag0E()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 25
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCompleteControllerMeasurementSequence(void);
extern void ObserveResumeMainDataRingCursorFromLatestDescriptor(void);
extern void ObservePopulateTrackTableFromDecodedTocTrackRange(void);
extern u8 DispatchSelectReadAcquisitionTargetForDirectionBitClear(u8 wide);
extern u8 DispatchSelectReadAcquisitionTargetForDirectionBitSet(u8 wide);
extern void DispatchConfigureControllerPorts8085And8086Value80AndYield(void);
extern void DispatchResetControllerPort8087And808A808BUnlessFlag0E(void);
extern void DispatchClearControllerPorts8088And8089UnlessFlag0E(void);
extern void DispatchMovePickupByDistanceInChunks(u8 direction, u16 distance);
extern u8 DispatchIsIndexedDriveStateActiveForOperatingUpdate(u8 state_code);
extern void DispatchConvergeControllerOperatingStageIndexToTarget(void);
extern void DispatchSubmitControllerByteAndWordForStageConvergence(u8 stage_code, u16 controller_word);
extern void DispatchWaitTimer1ScaledDurationForStageConvergence(u16 duration_units, u8 yield_while_waiting);
extern void DispatchFillControllerBufferChunkAndWait(fw_addr_t buffer_address, u16 byte_count, u8 value);
extern u8 DispatchClassifyThreeControllerSamplesByOrdering(void);
extern u8 DispatchValidateMediaCalibrationConfigurationForState3(void);
extern void DispatchResetDriveControlStateToEightForState3(void);
extern void DispatchPersistControllerOperatingStatusByteForState3(void);
extern void DispatchSignalDriveStatusPatternForeverForState3(void);
extern void DispatchRestartCoroutine1AtF84B0BAndYieldForState3(void);
extern void DispatchAccumulateControllerCommand0EMeasurementCapture(void);
extern u8 DispatchPrepareRawReadAcquisitionPlanForSyntheticRange(u8 alternate_plan);
extern void DispatchBuildRawReadQPacketForSyntheticRange(s32 frame_position, fw_addr_t destination);
extern u8 DispatchGenerateSyntheticRawReadPatternRange(s32 start_frame_position, s32 end_frame_position_exclusive);
extern void DispatchAccumulateMediaState5ControllerCalibrationSample(void);
extern u8 DispatchWaitForReadAcquisitionForPipelineProgress(void);
extern void DispatchReportReadPipelineFailureForRecovery(u8 failure_code);
extern void DispatchReportSense030C09ForRecovery(void);
extern void DispatchInitializeType2ControllerRecordGeometryForReset(void);
extern void DispatchWaitForReadAcquisitionForVendorF3Measurement(void);
extern void DispatchWriteControllerParametersE8E9ForVendorF3Reset(u8 value);
extern void DispatchWriteControllerParameterEFForVendorF3Reset(u8 value);
extern void DispatchSetControllerFlagForVendorF3Reset(u8 enabled);
extern u8 DispatchSelectCurrentRawReadSectorClass(void);
extern u8 DispatchFindAdjacentRawReadQRecordClass(u16 record_index, u8 scan_forward);
extern u16 DispatchBuildVendorEcRecordListResponse(u8 include_all, u8 alternate_format);
extern void DispatchCopyVendorEcResponseToControllerBuffer(fw_addr_t source, u32 controller_offset, u32 byte_count);
extern u8 DispatchExecuteVendorEcDataIn(u32 transfer_length);
extern void DispatchInitializeWriteDefaultsForPrepareWriteCommand(void);
extern void DispatchPopulateRawSectorHeaderAndTdiPayloadRangeForFinalize(fw_addr_t controller_buffer_address, u32 sector_count);
extern void DispatchEnqueueRawReadDescriptorAndAdvanceQStateForFinalize(u16 track_record_index);
extern void DispatchWriteControllerReadinessParameters(void);
extern void DispatchWaitControllerPortReadyForReadiness(fw_addr_t port_address, u8 value);
extern u8 DispatchProbeControllerReadPathAtCurrentFrame(void);
extern u8 DispatchWaitForControllerState1302Bit5ForReadiness(void);
extern void DispatchRestoreControllerReadPathForReadiness(u8 full_restore);
extern void DispatchSubmitControllerWord0204ForReadiness(u8 subtract_high_code_offset);
extern u8 DispatchAcquireRawReadFrameSequenceWithCurrentQForPendingRange(s32 frame_count, u8 source_kind);
extern u8 DispatchAcquireFixedRawReadFramePrefixForPendingRange(u8 start_index, u8 frame_count);
extern u8 DispatchAcquireRawReadFrameSequenceWithQUpdatesForPendingRange(s32 frame_count, u8 update_q_metadata);
extern u8 DispatchBinaryByteToPackedBcdForWorkingTrackAcquisition(u8 value);
extern u8 DispatchAcquireFixedRawReadFramePrefixForWorkingTrack(u8 start_index, u8 frame_count);
extern void DispatchSetSenseDataForWorkingTrackAcquisition(u8 sense_key, u16 asc_ascq);
extern void DispatchSetSenseInformationForWorkingTrackAcquisition(u32 information);
extern u8 DispatchAcquireRawReadFrameSequenceWithCurrentQForWorkingTrack(s32 frame_count, u8 source_kind);
extern u8 DispatchAcquireRawReadFrameSequenceWithQUpdatesForWorkingTrack(s32 frame_count, u8 update_q_metadata);
extern u8 DispatchWaitForProducerCapacityForQMsfSectorCapture(void);
extern void DispatchLatchFailureContextForQMsfSectorCapture(void);
extern void DispatchWriteControllerBlock25ForQMsfSectorCapture(fw_addr_t q_msf_address);
extern u8 DispatchCaptureRawSectorForQMsfSectorCapture(fw_addr_t controller_buffer_address, u8 source_kind);
extern void DispatchSetSenseDataForQMsfSectorCapture(u8 sense_key, u16 asc_ascq);
extern void DispatchSetSenseInformationForQMsfSectorCapture(u32 information);
extern void DispatchPopulateMode2SubheaderForQMsfSectorCapture(fw_addr_t controller_buffer_address, u8 source_kind);
extern u8 DispatchProcessIndexedStateGroupForState0C(u8 terminal_handoff);
extern u8 DispatchValidatePauseResumeCdb(fw_addr_t mask_descriptor);
extern void DispatchHandleUnsupportedPauseResumeCommand(void);
extern void DispatchExecuteReadMasterCueResponse(void);
extern void DispatchSetPauseResumeCommandSequenceError(void);
extern void DispatchWaitForPauseReadAcquisition(void);
extern void DispatchClearPauseResumeBit0018(void);
extern void DispatchStartPauseResumeFormat2Read(s32 frame_position);
extern void DispatchSetPauseResumeNoSense(void);
extern void DispatchStrobeControllerPortForReadPathRestore(fw_addr_t port_address, u8 value);
extern void DispatchWriteControllerParameterD6ForReadPathRestore(void);
extern void DispatchRefreshControllerParametersForReadPathRestore(void);
extern void DispatchConfigureControllerPortsForReadPathRestore(void);
extern void DispatchSubmitControllerWord0204ForReadPathRestore(u8 subtract_high_code_offset);
extern void DispatchRestoreControllerReadPathForState1302Wait(u8 refresh_derived_parameters);
extern void DispatchWaitTimer1ScaledDurationForState1302Wait(u16 duration_units, u8 yield_while_waiting);
extern void DispatchStrobeControllerPortForState1302Wait(fw_addr_t port_address, u8 value);
extern void DispatchWriteControllerParameterD6ForState1302Wait(void);
extern void DispatchIncrementPackedBcdMsfForSectorCapture(u16 address);
extern void DispatchWriteControllerRegisterBlock25ForSectorCapture(fw_addr_t source_address);
extern u8 DispatchCaptureRawSectorForSectorCapture(fw_addr_t main_buffer_address, u8 source_kind);
extern void DispatchFillControllerBufferForSectorCapture(fw_addr_t buffer_address, u32 byte_count, u8 value);
extern void DispatchSetSenseDataForSectorCapture(u8 sense_key, u16 asc_ascq);
extern void DispatchSetSenseInformationForSectorCapture(u32 information);
extern u8 DispatchAcquireRawReadRangeAndWaitForQSequence(s32 start_frame_address, u32 frame_count);
extern u8 DispatchPollRawReadPipelineProgressForQSequence(u8 poll_count);
extern void DispatchReportReadPipelineFailureForQSequence(u8 failure_code);
extern void DispatchReportSense030C09ForQSequence(void);
extern void DispatchSetP1Bits0And1ForOperatingCode4(void);
extern void DispatchStrobeControllerPortForOperatingCode4(fw_addr_t port_address, u8 value);
extern void DispatchConfigureControllerThresholdForOperatingCode4(u32 threshold_value);
extern void DispatchPulseControllerWord0253ForOperatingCode4(u16 duration_units);
extern void DispatchConfigureControllerPort8006AndChannel3ForOperatingCode4(u8 enabled);
extern void DispatchUpdateQPacketForIndexedDescriptor(u8 decrement_relative_msf, u8 skip_identification_packets);
extern void DispatchEnqueueCurrentDescriptorForIndexedDescriptor(void);
extern void DispatchIncrementMsfForIndexedDescriptor(fw_addr_t tuple_address);
extern void DispatchDecrementMsfForIndexedDescriptor(fw_addr_t tuple_address);
extern void DispatchAdvanceRingForIndexedDescriptor(u32 main_data_cursor);
#else
static u8 DispatchSelectReadAcquisitionTargetForDirectionBitClear(u8 wide)
{
    return SelectReadAcquisitionTargetForDirectionBitClear(wide);
}
static u8 DispatchSelectReadAcquisitionTargetForDirectionBitSet(u8 wide)
{
    return SelectReadAcquisitionTargetForDirectionBitSet(wide);
}
static void DispatchConfigureControllerPorts8085And8086Value80AndYield(void)
{
    ConfigureControllerPorts8085And8086Value80AndYield();
}
static void DispatchResetControllerPort8087And808A808BUnlessFlag0E(void)
{
    ResetControllerPort8087And808A808BUnlessFlag0E();
}
static void DispatchClearControllerPorts8088And8089UnlessFlag0E(void)
{
    ClearControllerPorts8088And8089UnlessFlag0E();
}
static void DispatchMovePickupByDistanceInChunks(u8 direction, u16 distance)
{
    MovePickupByDistanceInChunks(direction, distance);
}
static u8 DispatchIsIndexedDriveStateActiveForOperatingUpdate(u8 state_code)
{
    return IsIndexedDriveStateActive(state_code);
}
static void DispatchConvergeControllerOperatingStageIndexToTarget(void)
{
    ConvergeControllerOperatingStageIndexToTarget();
}
static void DispatchSubmitControllerByteAndWordForStageConvergence(u8 stage_code, u16 controller_word)
{
    SubmitControllerByteAndWord(stage_code, controller_word);
}
static void DispatchWaitTimer1ScaledDurationForStageConvergence(u16 duration_units, u8 yield_while_waiting)
{
    WaitTimer1ScaledDuration(duration_units, yield_while_waiting);
}
static void DispatchFillControllerBufferChunkAndWait(fw_addr_t buffer_address, u16 byte_count, u8 value)
{
    FillControllerBufferChunkAndWait(buffer_address, byte_count, value);
}
static void DispatchInitializeType2ControllerRecordGeometryForReset(void)
{
    InitializeType2ControllerRecordGeometry();
}
static void DispatchWaitForReadAcquisitionForVendorF3Measurement(void)
{
    (void)DispatchAndWaitForReadAcquisitionResult();
}
static void DispatchWriteControllerParametersE8E9ForVendorF3Reset(u8 value)
{
    WriteControllerParametersE8E9ForValue(value);
}
static void DispatchWriteControllerParameterEFForVendorF3Reset(u8 value)
{
    WriteControllerParameterEF00AndConditionalD0Value(value);
}
static void DispatchSetControllerFlagForVendorF3Reset(u8 enabled)
{
    SetControllerFlagFC07Bit2AndPort8098Bit6(enabled);
}
static u8 DispatchSelectCurrentRawReadSectorClass(void)
{
    return SelectCurrentRawReadSectorClass();
}
static u8 DispatchFindAdjacentRawReadQRecordClass(u16 record_index, u8 scan_forward)
{
    return FindAdjacentRawReadQRecordClass(record_index, scan_forward);
}
static u16 DispatchBuildVendorEcRecordListResponse(u8 include_all, u8 alternate_format)
{
    return BuildVendorEcRecordListResponse(include_all, alternate_format);
}
static void DispatchCopyVendorEcResponseToControllerBuffer(fw_addr_t source, u32 controller_offset, u32 byte_count)
{
    (void)CopyToControllerBufferWindow(source, controller_offset, byte_count);
}
static u8 DispatchExecuteVendorEcDataIn(u32 transfer_length)
{
    return ExecuteControllerDataInAndMapStatus(transfer_length);
}
static void DispatchInitializeWriteDefaultsForPrepareWriteCommand(void)
{
    InitializeWriteModePage05AndTrackStateDefaults();
}
extern void PLEXTOR_NEAR_FUNCTION EnqueueRawReadDescriptorAndAdvanceQStateForIndex(u16 track_record_index);
static void DispatchPopulateRawSectorHeaderAndTdiPayloadRangeForFinalize(fw_addr_t controller_buffer_address, u32 sector_count)
{
    PopulateRawSectorHeaderAndTdiPayloadRange(controller_buffer_address, sector_count);
}
static void DispatchEnqueueRawReadDescriptorAndAdvanceQStateForFinalize(u16 track_record_index)
{
    EnqueueRawReadDescriptorAndAdvanceQStateForIndex(track_record_index);
}
extern u32 AdvanceRawReadRingPosition(u32 main_data_cursor);
#define DispatchUpdateQPacketForIndexedDescriptor(decrement, skip) UpdateRawReadQPacketControlAndIdentification((skip), (decrement))
#define DispatchEnqueueCurrentDescriptorForIndexedDescriptor() EnqueueCurrentRawSectorDescriptorAtomic()
#define DispatchIncrementMsfForIndexedDescriptor(address) IncrementPackedBcdMsf(address)
#define DispatchDecrementMsfForIndexedDescriptor(address) DecrementPackedBcdMsf(address)
#define DispatchAdvanceRingForIndexedDescriptor(cursor) AdvanceRawReadRingPosition(cursor)
extern u8 PLEXTOR_NEAR_FUNCTION ProbeControllerReadPathAtCurrentFrame(void);
extern u8 PLEXTOR_NEAR_FUNCTION WaitForControllerState1302Bit5WithTimeout(void);
extern void RestoreControllerReadPathConfiguration(u8 full_restore);
static void DispatchWriteControllerReadinessParameters(void)
{
    WriteControllerParametersFCFDF6F7FromCurrentState();
}
static void DispatchWaitControllerPortReadyForReadiness(fw_addr_t port_address, u8 value)
{
    WaitControllerPortReadyAndWriteByte(port_address, value);
}
static u8 DispatchProbeControllerReadPathAtCurrentFrame(void)
{
    return ProbeControllerReadPathAtCurrentFrame();
}
static u8 DispatchWaitForControllerState1302Bit5ForReadiness(void)
{
    return WaitForControllerState1302Bit5WithTimeout();
}
static void DispatchRestoreControllerReadPathForReadiness(u8 full_restore)
{
    RestoreControllerReadPathConfiguration(full_restore);
}
static void DispatchSubmitControllerWord0204ForReadiness(u8 subtract_high_code_offset)
{
    SubmitControllerWord0204WithOptionalHighCodeOffset(subtract_high_code_offset);
}
static u8 DispatchAcquireRawReadFrameSequenceWithCurrentQForPendingRange(s32 frame_count, u8 source_kind)
{
    return AcquireRawReadFrameSequenceWithCurrentQPacket(frame_count, source_kind);
}
static u8 DispatchAcquireFixedRawReadFramePrefixForPendingRange(u8 prefix_mode, u8 capture_source_kind)
{
    return AcquireFixedRawReadFramePrefixWindow(prefix_mode, capture_source_kind);
}
static u8 DispatchAcquireRawReadFrameSequenceWithQUpdatesForPendingRange(s32 frame_count, u8 update_q_metadata)
{
    return AcquireRawReadFrameSequenceWithQUpdates(frame_count, update_q_metadata);
}
static u8 DispatchBinaryByteToPackedBcdForWorkingTrackAcquisition(u8 value)
{
    return BinaryByteToPackedBcd(value);
}
static u8 DispatchAcquireFixedRawReadFramePrefixForWorkingTrack(u8 prefix_mode, u8 capture_source_kind)
{
    return AcquireFixedRawReadFramePrefixWindow(prefix_mode, capture_source_kind);
}
static void DispatchSetSenseDataForWorkingTrackAcquisition(u8 sense_key, u16 asc_ascq)
{
    SetSenseData(sense_key, asc_ascq);
}
static void DispatchSetSenseInformationForWorkingTrackAcquisition(u32 information)
{
    SetSenseInformation(information);
}
static u8 DispatchAcquireRawReadFrameSequenceWithCurrentQForWorkingTrack(s32 frame_count, u8 source_kind)
{
    return AcquireRawReadFrameSequenceWithCurrentQPacket(frame_count, source_kind);
}
static u8 DispatchAcquireRawReadFrameSequenceWithQUpdatesForWorkingTrack(s32 frame_count, u8 update_q_metadata)
{
    return AcquireRawReadFrameSequenceWithQUpdates(frame_count, update_q_metadata);
}
static u8 DispatchWaitForProducerCapacityForQMsfSectorCapture(void)
{
    return WaitForRawReadProducerCapacityAndCheckStatus();
}
static void DispatchLatchFailureContextForQMsfSectorCapture(void)
{
    LatchFirstReadFailureContext();
}
static void DispatchWriteControllerBlock25ForQMsfSectorCapture(fw_addr_t q_msf_address)
{
    WriteControllerRegisterBlock25FromMemory(q_msf_address);
}
static u8 DispatchCaptureRawSectorForQMsfSectorCapture(fw_addr_t controller_buffer_address, u8 source_kind)
{
    return CAPTURE_RAW_SECTOR_FROM_C(controller_buffer_address, source_kind);
}
static void DispatchSetSenseDataForQMsfSectorCapture(u8 sense_key, u16 asc_ascq)
{
    SetSenseData(sense_key, asc_ascq);
}
static void DispatchSetSenseInformationForQMsfSectorCapture(u32 information)
{
    SetSenseInformation(information);
}
static void DispatchPopulateMode2SubheaderForQMsfSectorCapture(fw_addr_t controller_buffer_address, u8 source_kind)
{
    PopulateConfiguredMode2SubheaderWhenRequired(controller_buffer_address, source_kind);
}
extern u8 PLEXTOR_NEAR_FUNCTION ProcessIndexedStateGroup(u8 terminal_handoff);
static u8 DispatchProcessIndexedStateGroupForState0C(u8 terminal_handoff)
{
    return ProcessIndexedStateGroup(terminal_handoff);
}
extern void ClearBit0018Atomic(void);
static u8 DispatchValidatePauseResumeCdb(fw_addr_t mask_descriptor)
{
    return ValidateScsiCdbFieldMasks(mask_descriptor);
}
static void DispatchHandleUnsupportedPauseResumeCommand(void)
{
    HandleUnsupportedScsiCommand();
}
extern void ExecuteReadMasterCueResponse(void);
#define DispatchExecuteReadMasterCueResponse() ExecuteReadMasterCueResponse()
static void DispatchSetPauseResumeCommandSequenceError(void)
{
    SetCommandSequenceErrorSense();
}
static void DispatchWaitForPauseReadAcquisition(void)
{
    (void)DispatchAndWaitForReadAcquisitionResult();
}
static void DispatchClearPauseResumeBit0018(void)
{
    ClearBit0018Atomic();
}
static void DispatchStartPauseResumeFormat2Read(s32 frame_position)
{
    StartFormat2RawSectorReadAtFramePosition(frame_position);
}
static void DispatchSetPauseResumeNoSense(void)
{
    SetNoSenseData();
}
static void DispatchStrobeControllerPortForReadPathRestore(fw_addr_t port_address, u8 value)
{
    StrobeControllerPortWriteByteFromC(port_address, value);
}
static void DispatchWriteControllerParameterD6ForReadPathRestore(void)
{
    WriteControllerParameterD6Value00();
}
static void DispatchRefreshControllerParametersForReadPathRestore(void)
{
    RefreshControllerParametersFCAndF6ForCurrentOperatingCode();
}
static void DispatchConfigureControllerPortsForReadPathRestore(void)
{
    ConfigureControllerPorts80D0D1D3ForCurrentState();
}
static void DispatchSubmitControllerWord0204ForReadPathRestore(u8 subtract_high_code_offset)
{
    SubmitControllerWord0204WithOptionalHighCodeOffset(subtract_high_code_offset);
}
static void DispatchRestoreControllerReadPathForState1302Wait(u8 refresh_derived_parameters)
{
    RestoreControllerReadPathConfiguration(refresh_derived_parameters);
}
static void DispatchWaitTimer1ScaledDurationForState1302Wait(u16 duration_units, u8 yield_while_waiting)
{
    WaitTimer1ScaledDuration(duration_units, yield_while_waiting);
}
static void DispatchStrobeControllerPortForState1302Wait(fw_addr_t port_address, u8 value)
{
    StrobeControllerPortWriteByteFromC(port_address, value);
}
static void DispatchWriteControllerParameterD6ForState1302Wait(void)
{
    WriteControllerParameterD6Value20();
}
static void DispatchIncrementPackedBcdMsfForSectorCapture(u16 address)
{
    IncrementPackedBcdMsf(address);
}
static void DispatchWriteControllerRegisterBlock25ForSectorCapture(fw_addr_t source_address)
{
    WriteControllerRegisterBlock25FromMemory(source_address);
}
static u8 DispatchCaptureRawSectorForSectorCapture(fw_addr_t main_buffer_address, u8 source_kind)
{
    return CAPTURE_RAW_SECTOR_FROM_C(main_buffer_address, source_kind);
}
static void DispatchFillControllerBufferForSectorCapture(fw_addr_t buffer_address, u32 byte_count, u8 value)
{
    FillControllerBufferRange(buffer_address, byte_count, value);
}
static void DispatchSetSenseDataForSectorCapture(u8 sense_key, u16 asc_ascq)
{
    SetSenseData(sense_key, asc_ascq);
}
static void DispatchSetSenseInformationForSectorCapture(u32 information)
{
    SetSenseInformation(information);
}
static u8 DispatchAcquireRawReadRangeAndWaitForQSequence(s32 start_frame_address, u32 frame_count)
{
    return AcquireRawReadRangeAndWait(start_frame_address, frame_count);
}
static u8 DispatchPollRawReadPipelineProgressForQSequence(u8 poll_count)
{
    return PollRawReadPipelineProgress(poll_count);
}
static void DispatchReportReadPipelineFailureForQSequence(u8 failure_code)
{
    ReportReadPipelineFailure(failure_code);
}
static void DispatchReportSense030C09ForQSequence(void)
{
    ReportSense030C09FromRawReadQueue();
}
static void DispatchSetP1Bits0And1ForOperatingCode4(void)
{
    SetP1Bits0And1();
}
static void DispatchStrobeControllerPortForOperatingCode4(fw_addr_t port_address, u8 value)
{
    StrobeControllerPortWriteByteFromC(port_address, value);
}
static void DispatchConfigureControllerThresholdForOperatingCode4(u32 threshold_value)
{
    ConfigureControllerForCurrentOperatingCodeAndThresholdValue(threshold_value);
}
static void DispatchPulseControllerWord0253ForOperatingCode4(u16 duration_units)
{
    PulseControllerWord0253Value7F00ForDuration(duration_units);
}
static void DispatchConfigureControllerPort8006AndChannel3ForOperatingCode4(u8 enabled)
{
    ConfigureControllerPort8006AndChannel3(enabled);
}
static u8 DispatchClassifyThreeControllerSamplesByOrdering(void)
{
    return ClassifyThreeControllerSamplesByOrdering();
}
static u8 DispatchValidateMediaCalibrationConfigurationForState3(void)
{
    return ValidateMediaCalibrationConfigurationForCurrentState();
}
static void DispatchResetDriveControlStateToEightForState3(void)
{
    ResetDriveControlStateToEight();
}
static void DispatchPersistControllerOperatingStatusByteForState3(void)
{
    PersistControllerOperatingStatusByte();
}
static void DispatchSignalDriveStatusPatternForeverForState3(void)
{
    SignalDriveStatusPatternForever();
}
static void DispatchRestartCoroutine1AtF84B0BAndYieldForState3(void)
{
    RestartCoroutine1AtF84B0BAndYield();
}
static void DispatchAccumulateControllerCommand0EMeasurementCapture(void)
{
    AccumulateControllerCommand0EMeasurementCapture();
}
static u8 DispatchPrepareRawReadAcquisitionPlanForSyntheticRange(u8 alternate_plan)
{
    return PrepareRawReadAcquisitionPlan(alternate_plan);
}
static void DispatchBuildRawReadQPacketForSyntheticRange(s32 frame_position, fw_addr_t destination)
{
    BuildRawReadQPacketForFramePosition(frame_position, destination);
}
static u8 DispatchGenerateSyntheticRawReadPatternRange(s32 start_frame_position, s32 end_frame_position_exclusive)
{
    return GenerateSyntheticRawReadPatternRange(start_frame_position, end_frame_position_exclusive);
}
static void DispatchAccumulateMediaState5ControllerCalibrationSample(void)
{
    AccumulateMediaState5ControllerCalibrationSample();
}
static u8 DispatchWaitForReadAcquisitionForPipelineProgress(void)
{
    return WaitForReadAcquisitionCoroutineAndCheckPreparationStatus();
}
static void DispatchReportReadPipelineFailureForRecovery(u8 failure_code)
{
    ReportReadPipelineFailure(failure_code);
}
static void DispatchReportSense030C09ForRecovery(void)
{
    ReportSense030C09FromRawReadQueue();
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 26
#if defined(PLEXTOR_HOST_TEST)
static void DispatchPopulateTrackTableFromDecodedTocTrackRange(void)
{
    ObservePopulateTrackTableFromDecodedTocTrackRange();
}
#else
static void DispatchPopulateTrackTableFromDecodedTocTrackRange(void)
{
    PopulateTrackTableFromDecodedTocTrackRange();
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 27
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveComputeControllerOperatingProfileAdjustment(void);
#define DISPATCH_COMPUTE_CONTROLLER_OPERATING_PROFILE_ADJUSTMENT() ObserveComputeControllerOperatingProfileAdjustment()
#else
extern void PLEXTOR_NEAR_FUNCTION ComputeControllerOperatingProfileAdjustment(void);
#define DISPATCH_COMPUTE_CONTROLLER_OPERATING_PROFILE_ADJUSTMENT() ComputeControllerOperatingProfileAdjustment()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 28
#if defined(PLEXTOR_HOST_TEST)
extern void DispatchAdvanceControllerMeasurementSeries00And01Sequence(void);
extern void DispatchAdvanceFixedControllerParameterSequence(void);
extern void DispatchServiceControllerOperatingProfileInputMatches(void);
#else
static void DispatchAdvanceControllerMeasurementSeries00And01Sequence(void)
{
    AdvanceControllerMeasurementSeries00And01Sequence();
}
static void DispatchAdvanceFixedControllerParameterSequence(void)
{
    AdvanceFixedControllerParameterSequence();
}
static void DispatchServiceControllerOperatingProfileInputMatches(void)
{
    ServiceControllerOperatingProfileInputMatches();
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 29
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRunDriveControlState9Sequence(void);
#define DISPATCH_RUN_DRIVE_CONTROL_STATE9_SEQUENCE() ObserveRunDriveControlState9Sequence()
#else
extern u8 PLEXTOR_NEAR_FUNCTION RunDriveControlState9Sequence(void);
#define DISPATCH_RUN_DRIVE_CONTROL_STATE9_SEQUENCE() RunDriveControlState9Sequence()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 30
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveTestControllerBufferMemoryAddressingForSelfTest(void);
#define DispatchTestControllerBufferMemoryAddressingForSelfTest() ObserveTestControllerBufferMemoryAddressingForSelfTest()
#else
#define DispatchTestControllerBufferMemoryAddressingForSelfTest() TestControllerBufferMemoryAddressing()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 31
#if defined(PLEXTOR_HOST_TEST)
extern void DispatchBuildRawSectorDescriptorScratch(u16 descriptor_status, u8 descriptor_mode);
extern u8 DispatchCheckP1Bit3StableForParsedQDescriptor(void);
extern void DispatchEnqueueParsedQRawSectorDescriptor(void);
#else
static void DispatchBuildRawSectorDescriptorScratch(u16 descriptor_status, u8 descriptor_mode)
{
    BuildRawSectorDescriptorScratch(descriptor_status, descriptor_mode);
}
static u8 DispatchCheckP1Bit3StableForParsedQDescriptor(void)
{
    return CheckP1Bit3StableHighForCurrentMediaState();
}
static void DispatchEnqueueParsedQRawSectorDescriptor(void)
{
    EnqueueRawSectorDescriptor();
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 32
#if defined(PLEXTOR_HOST_TEST)
extern void DispatchAccumulateEightChannelMeasurementCapture(void);
extern u8 DispatchIsIndexedDriveStateActiveForWord0255Target(u8 state_code);
extern u8 DispatchProbeWorkingTrackRawQAtPipelineTarget(void);
extern u8 ObserveInspectLatestRawSectorDescriptorForWorkingTrackProbe(void);
#define DispatchInspectLatestRawSectorDescriptorForWorkingTrackProbe() ObserveInspectLatestRawSectorDescriptorForWorkingTrackProbe()
extern u8 DispatchRawQTargetReadAcquisition(void);
extern u8 DispatchRefineRawQFrameProbe(s32 frame_position);
extern u8 DispatchProbeRawReadQPositionAtTargetForOffsets(void);
extern u8 DispatchApplyRawQProbeStatusToWorkingTrackStateForTdi(void);
extern u8 DispatchApplyRawQProbeStatusToWorkingTrackTailForTdi(void);
extern void DispatchPopulateTrackTableRecordForCurrentSession(fw_addr_t raw_q_record_address);
extern void DispatchLoadTrackInformationForCurrentSession(u8 track_number);
extern u8 DispatchInitializeWorkingTrackTypeForCurrentSession(void);
extern u8 DispatchClassifyWorkingTrackForCurrentSession(void);
extern void DispatchUpdateTrackTableForCurrentSession(void);
#else
extern void PLEXTOR_NEAR_FUNCTION AccumulateEightChannelMeasurementCapture(void);
#define DispatchAccumulateEightChannelMeasurementCapture() AccumulateEightChannelMeasurementCapture()
#define DispatchInspectLatestRawSectorDescriptorForWorkingTrackProbe() InspectLatestRawSectorDescriptorAndCaptureHeader()
extern u8 ProbeRawReadQPositionAtTarget(void);
extern u8 PLEXTOR_NEAR_FUNCTION ApplyRawQProbeStatusToWorkingTrackState(void);
extern u8 PLEXTOR_NEAR_FUNCTION ApplyRawQProbeStatusToWorkingTrackTail(void);
static u8 DispatchIsIndexedDriveStateActiveForWord0255Target(u8 state_code)
{
    return IsIndexedDriveStateActive(state_code);
}
static u8 DispatchProbeWorkingTrackRawQAtPipelineTarget(void)
{
    return ProbeWorkingTrackRawQAtPipelineTarget();
}
static u8 DispatchRawQTargetReadAcquisition(void)
{
    return DispatchReadAcquisitionAndWaitForResult();
}
static u8 DispatchRefineRawQFrameProbe(s32 frame_position)
{
    u8 packet[10];

    return ProbeRawQAtTargetAndCopyPacket(frame_position, (fw_addr_t)(void *)packet);
}
static u8 DispatchProbeRawReadQPositionAtTargetForOffsets(void)
{
    return ProbeRawReadQPositionAtTarget();
}
static u8 DispatchApplyRawQProbeStatusToWorkingTrackStateForTdi(void)
{
    return ApplyRawQProbeStatusToWorkingTrackState();
}
static u8 DispatchApplyRawQProbeStatusToWorkingTrackTailForTdi(void)
{
    return ApplyRawQProbeStatusToWorkingTrackTail();
}
static void DispatchPopulateTrackTableRecordForCurrentSession(fw_addr_t raw_q_record_address)
{
    PopulateTrackTableRecordFromRawQRecord(raw_q_record_address);
}
static void DispatchLoadTrackInformationForCurrentSession(u8 track_number)
{
    /* The freshly populated record is type two or three, so fc3731 cannot
     * consume its otherwise inherited B value on this call path. */
    (void)LoadTrackInformationByNumberFromInheritedB(track_number, 0u);
}
static u8 DispatchInitializeWorkingTrackTypeForCurrentSession(void)
{
    return InitializeWorkingTrackTypeFromRawQProbe();
}
static u8 DispatchClassifyWorkingTrackForCurrentSession(void)
{
    return ClassifyAndInitializeWorkingTrackFromRawQProbe();
}
static void DispatchUpdateTrackTableForCurrentSession(void)
{
    UpdateTrackTableFromWorkingTrack();
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 33
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveBuildControllerOperatingInterpolationGrid(void);
extern void DispatchBuildControllerOperatingInterpolationGridColumn(void);
extern void DispatchBuildControllerOperatingInterpolationTableSlice(void);
#else
static void DispatchBuildControllerOperatingInterpolationGridColumn(void)
{
    BuildControllerOperatingInterpolationGridColumn();
}
static void DispatchBuildControllerOperatingInterpolationTableSlice(void)
{
    BuildControllerOperatingInterpolationTableSlice();
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 34
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveUpdateReadAcquisitionMeasurementFrom8Samples(void);
#define DispatchUpdateReadAcquisitionMeasurementFrom8Samples() ObserveUpdateReadAcquisitionMeasurementFrom8Samples()
#else
extern void PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionMeasurementFrom8Samples(void);
#define DispatchUpdateReadAcquisitionMeasurementFrom8Samples() UpdateReadAcquisitionMeasurementFrom8Samples()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 35
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFinalizeControllerCommand0EMeasurementCapture(void);
#define DispatchFinalizeControllerCommand0EMeasurementCapture() ObserveFinalizeControllerCommand0EMeasurementCapture()
#else
extern void FinalizeControllerCommand0EMeasurementCapture(void);
#define DispatchFinalizeControllerCommand0EMeasurementCapture() FinalizeControllerCommand0EMeasurementCapture()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 36
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveApplyControllerOperatingParametersForState1Path(void);
#define DispatchApplyControllerOperatingParametersForState1Path() ObserveApplyControllerOperatingParametersForState1Path()
extern void ObserveApplyControllerOperatingParametersForState2Path(void);
#define DispatchApplyControllerOperatingParametersForState2Path() ObserveApplyControllerOperatingParametersForState2Path()
#else
#define DispatchApplyControllerOperatingParametersForState1Path() ApplyControllerOperatingParametersForState1Path()
#define DispatchApplyControllerOperatingParametersForState2Path() ApplyControllerOperatingParametersForState2Path()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 37
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveExecuteControllerDataInTransfer(u32 buffer_address, u32 transfer_length);
#define DispatchExecuteControllerDataInTransfer(address, length) ObserveExecuteControllerDataInTransfer((address), (length))
extern void ObserveExecuteControllerDataOutTransfer(u32 buffer_address, u32 transfer_length);
#define DispatchExecuteControllerDataOutTransfer(address, length) ObserveExecuteControllerDataOutTransfer((address), (length))
#else
extern void ExecuteControllerDataInTransfer(u32 buffer_address, u32 transfer_length);
#define DispatchExecuteControllerDataInTransfer(address, length) ExecuteControllerDataInTransfer((address), (length))
extern void ExecuteControllerDataOutTransfer(u32 buffer_address, u32 transfer_length);
#define DispatchExecuteControllerDataOutTransfer(address, length) ExecuteControllerDataOutTransfer((address), (length))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 38
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCaptureControllerSampleSeriesForPostTocCalibration(void);
#define DispatchCaptureControllerSampleSeriesForPostTocCalibration() ObserveCaptureControllerSampleSeriesForPostTocCalibration()
#else
extern void CaptureControllerSampleSeries(void);
#define DispatchCaptureControllerSampleSeriesForPostTocCalibration() CaptureControllerSampleSeries()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 39
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveServiceIndexedState0FTimedOutputSequence(void);
#define DispatchServiceIndexedState0FTimedOutputSequence() ObserveServiceIndexedState0FTimedOutputSequence()
#else
extern void ServiceIndexedState0FTimedOutputSequence(void);
#define DispatchServiceIndexedState0FTimedOutputSequence() ServiceIndexedState0FTimedOutputSequence()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 40
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveEnsureOpcForFormatUnit(void);
extern void ObserveFormatUnitSuccessfulPreparation(void);
extern void ObserveFormatUnitFailedPreparation(void);
#define DispatchEnsureOpcForFormatUnit() ObserveEnsureOpcForFormatUnit()
#define DispatchFormatUnitSuccessfulPreparation() ObserveFormatUnitSuccessfulPreparation()
#define DispatchFormatUnitFailedPreparation() ObserveFormatUnitFailedPreparation()
#else
extern void PLEXTOR_NEAR_FUNCTION UpdateFormatUnitTrackStateAfterSuccessfulPreparation(void);
extern void PLEXTOR_NEAR_FUNCTION UpdateFormatUnitTrackStateAfterFailedPreparation(void);
#define DispatchEnsureOpcForFormatUnit() EnsureOpcForSelectedWriteSpeed()
#define DispatchFormatUnitSuccessfulPreparation() UpdateFormatUnitTrackStateAfterSuccessfulPreparation()
#define DispatchFormatUnitFailedPreparation() UpdateFormatUnitTrackStateAfterFailedPreparation()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 41
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveConfigureControllerPortsForCalibrationRetry(void);
extern u8 ObserveRunControllerCalibrationRetryAttempt(u8 calibration_mode);
extern void ObserveSubmitControllerWordForCalibrationRetry(u16 controller_value, u16 controller_selector);
extern void ObservePulseControllerWordForCalibrationRetry(u16 duration_units);
extern void ObserveSetControllerFlagForCalibrationRetry(u8 enabled);
extern void ObserveLoadControllerProfileForCalibrationRetry(u8 operating_code);
#define DispatchConfigureControllerPortsForCalibrationRetry() ObserveConfigureControllerPortsForCalibrationRetry()
#define DispatchRunControllerCalibrationRetryAttempt(calibration_mode) ObserveRunControllerCalibrationRetryAttempt(calibration_mode)
#define DispatchSubmitControllerWordForCalibrationRetry(value, selector) ObserveSubmitControllerWordForCalibrationRetry(value, selector)
#define DispatchPulseControllerWordForCalibrationRetry(duration) ObservePulseControllerWordForCalibrationRetry(duration)
#define DispatchSetControllerFlagForCalibrationRetry(enabled) ObserveSetControllerFlagForCalibrationRetry(enabled)
#define DispatchLoadControllerProfileForCalibrationRetry(operating_code) ObserveLoadControllerProfileForCalibrationRetry(operating_code)
#else
#define DispatchConfigureControllerPortsForCalibrationRetry() ConfigureControllerPorts80B2Through80C5ForCalibration()
#define DispatchRunControllerCalibrationRetryAttempt(calibration_mode) RunControllerPort80F8CalibrationAttempt(calibration_mode)
#define DispatchSubmitControllerWordForCalibrationRetry(value, selector) SubmitControllerWordPair(value, selector)
#define DispatchPulseControllerWordForCalibrationRetry(duration) PulseControllerWord0253Value8000ForDuration(duration)
#define DispatchSetControllerFlagForCalibrationRetry(enabled) SetControllerFlagFC07Bit2AndPort8098Bit6(enabled)
#define DispatchLoadControllerProfileForCalibrationRetry(operating_code) LoadControllerChannel0OperatingProfile(operating_code)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 42
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveAttemptMediaState3QCapture(void);
extern void ObserveRearmControllerQPacketCaptureForState3(void);
extern u8 ObserveProcessCapturedControllerQPacketForState3(void);
extern void ObserveYieldCoroutine1ForState3QCapture(void);
extern u8 ObserveCheckP1Cex0PulseForState3QCapture(void);
extern void ObserveResetDriveControlForState3QCapture(void);
extern u8 ObserveInitializeControllerForState3QCapture(void);
#define DispatchRearmControllerQPacketCaptureForState3() ObserveRearmControllerQPacketCaptureForState3()
#define DispatchProcessCapturedControllerQPacketForState3() ObserveProcessCapturedControllerQPacketForState3()
#define DispatchYieldCoroutine1ForState3QCapture() ObserveYieldCoroutine1ForState3QCapture()
#define DispatchCheckP1Cex0PulseForState3QCapture() ObserveCheckP1Cex0PulseForState3QCapture()
#define DispatchResetDriveControlForState3QCapture() ObserveResetDriveControlForState3QCapture()
#define DispatchInitializeControllerForState3QCapture() ObserveInitializeControllerForState3QCapture()
#else
#define DispatchRearmControllerQPacketCaptureForState3() RearmControllerQPacketCaptureWithTimeout()
#define DispatchProcessCapturedControllerQPacketForState3() ProcessCapturedControllerQPacket()
#define DispatchYieldCoroutine1ForState3QCapture() YieldCoroutine1()
#define DispatchCheckP1Cex0PulseForState3QCapture() CheckP1Cex0PulseWithinTimer1Window()
#define DispatchResetDriveControlForState3QCapture() ResetDriveControlStateToEight()
#define DispatchInitializeControllerForState3QCapture() InitializeControllerOperatingStateAndRunCalibration()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 43
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveAcquireDiscReadPositionForReinitializationRetry(void);
extern void ObserveResetDriveControlForReinitializationRetry(void);
extern u8 ObserveInitializeControllerForReinitializationRetry(void);
extern void ObserveSetControllerEnableForReinitializationRetry(u8 enabled);
extern void ObserveYieldCoroutine1ForReinitializationRetry(void);
#define DispatchAcquireDiscReadPositionForReinitializationRetry() ObserveAcquireDiscReadPositionForReinitializationRetry()
#define DispatchResetDriveControlForReinitializationRetry() ObserveResetDriveControlForReinitializationRetry()
#define DispatchInitializeControllerForReinitializationRetry() ObserveInitializeControllerForReinitializationRetry()
#define DispatchSetControllerEnableForReinitializationRetry(enabled) ObserveSetControllerEnableForReinitializationRetry(enabled)
#define DispatchYieldCoroutine1ForReinitializationRetry() ObserveYieldCoroutine1ForReinitializationRetry()
#else
extern u8 AcquireDiscReadPosition(void);
extern void YieldCoroutine1(void);
#define DispatchAcquireDiscReadPositionForReinitializationRetry() AcquireDiscReadPosition()
#define DispatchResetDriveControlForReinitializationRetry() ResetDriveControlStateToEight()
#define DispatchInitializeControllerForReinitializationRetry() InitializeControllerOperatingStateAndRunCalibration()
#define DispatchSetControllerEnableForReinitializationRetry(enabled) SetControllerOperatingEnableState(enabled)
#define DispatchYieldCoroutine1ForReinitializationRetry() YieldCoroutine1()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 44
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveInitializeMediaState5PositionController(void);
extern void ObserveSnapshotMediaState5MeasurementTarget(void);
extern void ObserveApplyMediaState5CalibrationOffset(void);
extern u8 ObserveTryReduceMediaState5WriteSpeed(u8 enabled);
extern void ObserveSetMediaState5ControllerEnable(u8 enabled);
extern u8 ObserveRunMediaState5ControllerCommand06(u8 selector);
extern void ObserveRecomputeMediaState5ControllerThreshold(void);
#define DispatchInitializeMediaState5PositionController() ObserveInitializeMediaState5PositionController()
#define DispatchSnapshotMediaState5MeasurementTarget() ObserveSnapshotMediaState5MeasurementTarget()
#define DispatchApplyMediaState5CalibrationOffset() ObserveApplyMediaState5CalibrationOffset()
#define DispatchTryReduceMediaState5WriteSpeed(enabled) ObserveTryReduceMediaState5WriteSpeed(enabled)
#define DispatchSetMediaState5ControllerEnable(enabled) ObserveSetMediaState5ControllerEnable(enabled)
#define DispatchRunMediaState5ControllerCommand06(selector) ObserveRunMediaState5ControllerCommand06(selector)
#define DispatchRecomputeMediaState5ControllerThreshold() ObserveRecomputeMediaState5ControllerThreshold()
#else
#define DispatchInitializeMediaState5PositionController() InitializeMediaState5PositionAndControllerState()
#define DispatchSnapshotMediaState5MeasurementTarget() SnapshotMeasurementDescriptorTargetPositionIfEnabled()
#define DispatchApplyMediaState5CalibrationOffset() ApplyFourSpeedWriteCalibrationOffset()
#define DispatchTryReduceMediaState5WriteSpeed(enabled) TryReduceWriteSpeedForCurrentMediaState(enabled)
#define DispatchSetMediaState5ControllerEnable(enabled) SetControllerOperatingEnableState(enabled)
#define DispatchRunMediaState5ControllerCommand06(selector) RunControllerCommand06AndDiscardPort80C7(selector)
#define DispatchRecomputeMediaState5ControllerThreshold() RecomputeMediaState5ControllerThreshold()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 45
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveLoadControllerProfileForCalibrationSetup(u8 operating_code);
extern void ObserveSetIndexedChannelForCalibrationSetup(u8 channel_index, u8 value);
extern void ObserveWriteControllerPortForCalibrationSetup(fw_addr_t port_address, u8 value);
#define DispatchLoadControllerProfileForCalibrationSetup(operating_code) ObserveLoadControllerProfileForCalibrationSetup(operating_code)
#define DispatchSetIndexedChannelForCalibrationSetup(channel_index, value) ObserveSetIndexedChannelForCalibrationSetup(channel_index, value)
#define DispatchWriteControllerPortForCalibrationSetup(address, value) ObserveWriteControllerPortForCalibrationSetup(address, value)
#else
#define DispatchLoadControllerProfileForCalibrationSetup(operating_code) LoadControllerChannel0OperatingProfile(operating_code)
#define DispatchSetIndexedChannelForCalibrationSetup(channel_index, value) SetIndexedControllerChannelValue(channel_index, value)
#define DispatchWriteControllerPortForCalibrationSetup(address, value) WaitControllerPortReadyAndWriteByte(address, value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 46
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSubmitControllerWordForPulse0253(u16 controller_value, u16 controller_selector);
extern void ObserveYieldCoroutine1ForPulse0253(void);
#define DispatchSubmitControllerWordForPulse0253(value, selector) ObserveSubmitControllerWordForPulse0253(value, selector)
#define DispatchYieldCoroutine1ForPulse0253() ObserveYieldCoroutine1ForPulse0253()
#else
#define DispatchSubmitControllerWordForPulse0253(value, selector) SubmitControllerWordPair(value, selector)
#define DispatchYieldCoroutine1ForPulse0253() YieldCoroutine1()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 47
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSetP1BitsForControllerRestart(void);
extern void ObservePulseWord0253ForControllerRestart(u16 duration_units);
extern void ObserveLoadProfileForControllerRestart(u8 profile_index);
extern void ObserveWritePortForControllerRestart(fw_addr_t port_address, u8 value);
extern void ObserveSubmitWordForControllerRestart(u16 controller_value, u16 controller_selector);
extern void ObserveSetChannelForControllerRestart(u8 channel_index, u8 value);
extern void ObserveYieldForControllerRestart(void);
extern void ObserveResolveDriveControlForControllerRestart(void);
extern void ObserveRestartCoroutine1ForControllerRestart(void);
#define DispatchSetP1BitsForControllerRestart() ObserveSetP1BitsForControllerRestart()
#define DispatchPulseWord0253ForControllerRestart(duration) ObservePulseWord0253ForControllerRestart(duration)
#define DispatchLoadProfileForControllerRestart(profile) ObserveLoadProfileForControllerRestart(profile)
#define DispatchWritePortForControllerRestart(address, value) ObserveWritePortForControllerRestart(address, value)
#define DispatchSubmitWordForControllerRestart(value, selector) ObserveSubmitWordForControllerRestart(value, selector)
#define DispatchSetChannelForControllerRestart(index, value) ObserveSetChannelForControllerRestart(index, value)
#define DispatchYieldForControllerRestart() ObserveYieldForControllerRestart()
#define DispatchResolveDriveControlForControllerRestart() ObserveResolveDriveControlForControllerRestart()
#define DispatchRestartCoroutine1ForControllerRestart() ObserveRestartCoroutine1ForControllerRestart()
#else
#define DispatchSetP1BitsForControllerRestart() SetP1Bits0And1()
#define DispatchPulseWord0253ForControllerRestart(duration) PulseControllerWord0253Value7F00ForDuration(duration)
#define DispatchLoadProfileForControllerRestart(profile) LoadControllerOperatingProfile(profile)
#define DispatchWritePortForControllerRestart(address, value) WaitControllerPortReadyAndWriteByte(address, value)
#define DispatchSubmitWordForControllerRestart(value, selector) SubmitControllerWordPair(value, selector)
#define DispatchSetChannelForControllerRestart(index, value) SetIndexedControllerChannelValue(index, value)
#define DispatchYieldForControllerRestart() YieldCoroutine1()
#define DispatchResolveDriveControlForControllerRestart() ResolveDriveControlStateToZero()
#define DispatchRestartCoroutine1ForControllerRestart() RestartCoroutine1AtF84B0BAndYield()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 48
#if defined(PLEXTOR_HOST_TEST)
static void SelectP1Bit1OnlyAtomic(void)
{
    FirmwareWriteDirectSfr8(90, (u8)((FirmwareReadDirectSfr8(90) & 0xfcu) | 2u));
}
extern void ObserveWritePortForState5Minimum(fw_addr_t port_address, u8 value);
extern u8 ObserveReadPortForState5Minimum(fw_addr_t port_address);
extern void ObserveSetP1BothForState5Minimum(void);
extern void ObserveSetP1Bit1OnlyForState5Minimum(void);
extern void ObserveClearP1BothForState5Minimum(void);
extern void ObservePulse7F00ForState5Minimum(u16 duration_units);
extern void ObservePulse8000ForState5Minimum(u16 duration_units);
extern u8 ObserveIndexedStateForState5Minimum(u8 state_code);
extern void ObserveSetChannelForState5Minimum(u8 channel_index, u8 value);
#define DispatchWritePortForState5Minimum(address, value) ObserveWritePortForState5Minimum(address, value)
#define DispatchReadPortForState5Minimum(address) ObserveReadPortForState5Minimum(address)
#define DispatchSetP1BothForState5Minimum() ObserveSetP1BothForState5Minimum()
#define DispatchSetP1Bit1OnlyForState5Minimum() ObserveSetP1Bit1OnlyForState5Minimum()
#define DispatchClearP1BothForState5Minimum() ObserveClearP1BothForState5Minimum()
#define DispatchPulse7F00ForState5Minimum(duration) ObservePulse7F00ForState5Minimum(duration)
#define DispatchPulse8000ForState5Minimum(duration) ObservePulse8000ForState5Minimum(duration)
#define DispatchIndexedStateForState5Minimum(state) ObserveIndexedStateForState5Minimum(state)
#define DispatchSetChannelForState5Minimum(index, value) ObserveSetChannelForState5Minimum(index, value)
#else
extern void SelectP1Bit1OnlyAtomic(void);
#define DispatchWritePortForState5Minimum(address, value) WaitControllerPortReadyAndWriteByte(address, value)
#define DispatchReadPortForState5Minimum(address) WaitControllerPortReadyAndReadByte(address)
#define DispatchSetP1BothForState5Minimum() SetP1Bits0And1()
#define DispatchSetP1Bit1OnlyForState5Minimum() SelectP1Bit1OnlyAtomic()
#define DispatchClearP1BothForState5Minimum() ClearP1Bits0And1Atomic()
#define DispatchPulse7F00ForState5Minimum(duration) PulseControllerWord0253Value7F00ForDuration(duration)
#define DispatchPulse8000ForState5Minimum(duration) PulseControllerWord0253Value8000ForDuration(duration)
#define DispatchIndexedStateForState5Minimum(state) IsIndexedDriveStateActive(state)
#define DispatchSetChannelForState5Minimum(index, value) SetIndexedControllerChannelValue(index, value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 49
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObservePrepareWriteCommandState(u32 start_address);
extern void ObserveInitializeVendorF3AlternateReadAcquisitionState(void);
extern void ObserveInitializeVendorF3ReadAcquisitionMeasurementState(void);
#define DispatchPrepareWriteCommandState(start_address) ObservePrepareWriteCommandState(start_address)
#define DispatchInitializeVendorF3AlternateReadAcquisitionState() ObserveInitializeVendorF3AlternateReadAcquisitionState()
#define DispatchInitializeVendorF3ReadAcquisitionMeasurementState() ObserveInitializeVendorF3ReadAcquisitionMeasurementState()
#else
#define DispatchPrepareWriteCommandState(start_address) PrepareWriteCommandState(start_address)
#define DispatchInitializeVendorF3AlternateReadAcquisitionState() InitializeVendorF3AlternateReadAcquisitionState()
#define DispatchInitializeVendorF3ReadAcquisitionMeasurementState() InitializeVendorF3ReadAcquisitionMeasurementState()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 50
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetControllerHardwareRegisters(void);
#define DispatchResetControllerHardwareRegisters() ObserveResetControllerHardwareRegisters()
#else
extern void ResetControllerHardwareRegisters(void);
#define DispatchResetControllerHardwareRegisters() ResetControllerHardwareRegisters()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 51
#if defined(PLEXTOR_HOST_TEST)
extern void FirmwareWriteBit0046(u8 value);
extern void FirmwareWriteBit0021(u8 value);
#define SetBit0046ForReadCommandAtomic() FirmwareWriteBit0046(1u)
#define ClearBit0046ForReadCommandAtomic() FirmwareWriteBit0046(0u)
#define SetBit0021ForReadCdAtomic() FirmwareWriteBit0021(1u)
#else
extern void SetBit0046ForReadCommandAtomic(void);
extern void ClearBit0046ForReadCommandAtomic(void);
extern void SetBit0021ForReadCdAtomic(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 52
#if defined(PLEXTOR_HOST_TEST)
#define ClearBit000EForReadRequestAtomic() FirmwareWriteBit000E(0u)
#define InitializeCoroutine2AtF84A00ForC() InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2)
#define InitializeCoroutine2AtF90000ForC() InitializeCoroutine2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2)
#define InitializeCoroutine1AtF85D74ForC() InitializeCoroutine1(FW_ENTRY_RUN_MEDIA_STATE_COROUTINE1)
#define InitializeCoroutine1AtF84B0BForC() InitializeCoroutine1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1)
#define InitializeCoroutine3AtF84B00ForC() InitializeCoroutine3(FW_ENTRY_IDLE_COROUTINE3)
#define InitializeCoroutine1AtF852EBForC() InitializeCoroutine1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COMPLETION_COROUTINE1)
#else
extern void PLEXTOR_NEAR_FUNCTION ClearBit000EForReadRequestAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF84A00ForC(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF90000ForC(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine1AtF85D74ForC(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine1AtF84B0BForC(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine3AtF84B00ForC(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine1AtF852EBForC(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 53
#if defined(PLEXTOR_HOST_TEST)
#define ReadBit0014Atomic() FirmwareReadBit0014()
#define ReadBit0015Atomic() FirmwareReadBit0015()
#define SetBit0014Atomic() FirmwareWriteBit0014(1u)
#define SetBit0015Atomic() FirmwareWriteBit0015(1u)
#define ClearBit0015ForCursorAtomic() FirmwareWriteBit0015(0u)
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0014Atomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0015Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0014Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0015Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0015ForCursorAtomic(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 54
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePrepareReadMeasurementForHighDescriptorMode(void);
#define PREPARE_READ_MEASUREMENT_FOR_HIGH_DESCRIPTOR_MODE() ObservePrepareReadMeasurementForHighDescriptorMode()
#else
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForHighDescriptorMode(void);
#define PREPARE_READ_MEASUREMENT_FOR_HIGH_DESCRIPTOR_MODE() PrepareReadMeasurementForHighDescriptorMode()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 55
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRefreshTocStateFromRawReadQRecordTable(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 56
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveLoadReadAcquisitionTargetForTrackNumber(void);
#define DispatchLoadReadAcquisitionTargetForTrackNumber() ObserveLoadReadAcquisitionTargetForTrackNumber()
#else
#define DispatchLoadReadAcquisitionTargetForTrackNumber() LoadReadAcquisitionTargetForTrackNumber()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 57
#if defined(PLEXTOR_HOST_TEST)
#define ClearBit0036Atomic() FirmwareWriteBit0036(0u)
#else
extern void PLEXTOR_NEAR_FUNCTION ClearBit0036Atomic(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 58
#if defined(PLEXTOR_HOST_TEST)
extern u8 CalibrateControllerParameterForCodeFromC(u8 parameter_code);
#define DispatchCalibrateControllerParameterForCode(code) CalibrateControllerParameterForCodeFromC(code)
#else
#define DispatchCalibrateControllerParameterForCode(code) CalibrateControllerParameterForCode(code)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 59
#if defined(PLEXTOR_HOST_TEST)
/* The packet count is driven asynchronously by the controller interrupt.
 * Focused host vectors supply the completed parser result/frame snapshot. */
extern u8 ParseMediaState6PacketForTargetWindow(void);
extern u8 ObserveMediaState6AtipWindowPulseResult(void);
#define EvaluateMediaState6AtipWindowPulseResult() ObserveMediaState6AtipWindowPulseResult()
extern u8 ParseReadMeasurementRangeStartPacket(void);
extern u8 EvaluatePulseThresholdForReadAcquisitionMeasurement(void);
extern u8 InterceptServicePendingHostCommandWhenOperationFlagsPermit(void);
extern u8 InterceptResetDriveControlAndInitializeControllerCalibration(void);
#else
static u8 ParseMediaState6PacketForTargetWindow(void)
{
    return ParseControllerThreeBytePacket();
}
#define EvaluateMediaState6AtipWindowPulseResult() FUN_RAM_f896ee()
static u8 ParseReadMeasurementRangeStartPacket(void)
{
    return ParseControllerThreeBytePacket();
}
static u8 EvaluatePulseThresholdForReadAcquisitionMeasurement(void)
{
    return TestP1Cex0PulseSuccessThreshold(10u);
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 60
#if defined(PLEXTOR_HOST_TEST)
/* The host simulator supplies this recovery-monitor interface.  It models the
 * code call at RAM:ff4500 without importing monitor implementation bytes. */
extern u8 InvokeRecoveryConfigurationProgram(u8 extended_only);
extern void YieldCoroutine1(void);
extern void ObserveServiceConditionalControllerOperatingDisable(void);
extern void ObservePrepareReadVerifyCommandState(void);
extern u8 EvaluateThirtySevenTrackLayoutForType2Finalization(void);
extern void ObserveCommitTrackTailState(void);
static void SetMediaState5LeadOutTransitionBitsAtomic(void)
{
    FirmwareWriteBit003B(1u);
    FirmwareWriteBit0039(1u);
    FirmwareWriteBit003A(1u);
    FirmwareWriteBit0036(1u);
}
static void SetExternalInterrupt1EnabledAtomic(u8 enabled)
{
    FirmwareWriteBit0542(enabled);
}
static void SetType2LayoutBitAtomic(u8 enabled)
{
    FirmwareWriteBit003F(enabled);
}
#else
/* Reconstruction helper; no direct Ghidra function address.
 * A251 preserves the immediate ECALL RAM:ff4500 contract. */
extern u8 InvokeRecoveryConfigurationProgram(u8 extended_only);
extern void SetP3Bit0Atomic(void);
extern void ClearP3Bit0Atomic(void);
extern void SetP3Bit1Atomic(void);
extern void ClearP3Bit1Atomic(void);
extern void YieldCoroutine1(void);
extern void PLEXTOR_NEAR_FUNCTION SetMediaState5LeadOutTransitionBitsAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0542Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0542Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit003FAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit003FAtomic(void);
static void SetExternalInterrupt1EnabledAtomic(u8 enabled)
{
    if(enabled != 0u)
        SetBit0542Atomic();
    else
        ClearBit0542Atomic();
}
static void SetType2LayoutBitAtomic(u8 enabled)
{
    if(enabled != 0u)
        SetBit003FAtomic();
    else
        ClearBit003FAtomic();
}
static u8 EvaluateThirtySevenTrackLayoutForType2Finalization(void)
{
    return IsThirtySevenTrackLayoutSpacedBy9000Frames();
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 61
#if defined(PLEXTOR_HOST_TEST)
static u32 ReadBigEndianU32Near(u16 address)
{
    return FirmwareReadNearU32(address);
}
#else
#define ReadBigEndianU32Near(address) (*((volatile u32 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 62
#if defined(PLEXTOR_HOST_TEST)
static void WriteBigEndianU32Near(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}
#else
#define WriteBigEndianU32Near(address, value) (*((volatile u32 huge *)(fw_addr_t)(u16)(address)) = (u32)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 63
#if defined(PLEXTOR_HOST_TEST)
static u16 ReadBigEndianU16Near(u16 address)
{
    return FirmwareReadNearU16(address);
}
#else
#define ReadBigEndianU16Near(address) (*((volatile u16 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 64
#if defined(PLEXTOR_HOST_TEST)
static void WriteBigEndianU16Near(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}
#else
#define WriteBigEndianU16Near(address, value) (*((volatile u16 huge *)(fw_addr_t)(u16)(address)) = (u16)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 65
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0011(0u);
#else
ClearBit0011Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 66
#if defined(PLEXTOR_HOST_TEST)
if(FirmwareReadBit0017() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) != 0u && FirmwareReadBit0016() != 0u)
{
    sense_code = 0x6400u;
    goto reject;
}
FirmwareWriteBit0017(0u);
FirmwareWriteBit0016(0u);
#else
if(ReadBit0017ForRangeValidation() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) != 0u && ReadBit0016ForRangeValidation() != 0u)
{
    sense_code = 0x6400u;
    goto reject;
}
ClearBits0016And0017ForRangeValidationAtomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 67
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0013(1u);
#else
SetBit0013ForRangeValidationAtomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 68
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit001C(1u);
#else
SetBit001CForRangeValidationAtomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 69
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit001D(1u);
#else
SetBit001DForRangeValidationAtomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 70
#if defined(PLEXTOR_HOST_TEST)
if(FirmwareReadBit0017() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) == 1u)
{
    u16 sense_code = FirmwareReadBit0016() != 0u ? 0x6400u : 0x2100u;
    u32 adjustment;
    u32 current_start_lba;

    SetSenseData(5u, sense_code);
    adjustment = ReadBigEndianU32Near(0x0f0cu);
    current_start_lba = ReadBigEndianU32Near(0x0b02u);
    SetSenseInformation(current_start_lba - adjustment);
    return 0u;
}
FirmwareWriteBit0016(0u);
FirmwareWriteBit0017(0u);
#else
if(ReadBit0017ForRangeValidation() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) == 1u)
{
    u16 sense_code = ReadBit0016ForRangeValidation() != 0u ? 0x6400u : 0x2100u;
    u32 adjustment;
    u32 current_start_lba;

    SetSenseData(5u, sense_code);
    adjustment = ReadBigEndianU32Near(0x0f0cu);
    current_start_lba = ReadBigEndianU32Near(0x0b02u);
    SetSenseInformation(current_start_lba - adjustment);
    return 0u;
}
ClearBits0016Then0017ForAlternateRangeValidationAtomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 71
#if defined(PLEXTOR_HOST_TEST)
if(FirmwareReadBit0021() != 0u)
#else
if(ReadBit0021ForAlternateRangeValidation() != 0u)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 72
#if defined(PLEXTOR_HOST_TEST)
    FirmwareWriteBit0547((u8)(saved_ie >> 7));
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 73
#if defined(PLEXTOR_HOST_TEST)
ObserveResumeMainDataRingCursorFromLatestDescriptor();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 74
#if defined(PLEXTOR_HOST_TEST)
static u32 ReadBigEndianU32Far(fw_addr_t address)
{
    return FirmwareReadU32(address);
}
#else
#define ReadBigEndianU32Far(address) (*((volatile u32 huge *)(fw_addr_t)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 75
#if defined(PLEXTOR_HOST_TEST)
static void WriteBigEndianU32Far(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}
#else
#define WriteBigEndianU32Far(address, value) (*((volatile u32 huge *)(fw_addr_t)(address)) = (u32)(value))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 76
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) | 1u));
FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) & 0xfdu));
#else
    SetP3Bit0Atomic();
ClearP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 77
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteDirectSfr8(B0, (u8)(FirmwareReadDirectSfr8(B0) | 3u));
#else
SetP3Bit0Atomic();
SetP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 78
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0547(0u);
#else
ClearBit0547Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 79
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0547((u8)(saved_ie >> 7));
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 80
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0547(0u);
#else
ClearBit0547Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 81
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0580(0u);
FirmwareWriteBit0581(1u);
#else
ClearP3Bit0Atomic();
SetP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 82
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0580(1u);
FirmwareWriteBit0581(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u ? 0u : 1u);
#else
SetP3Bit0Atomic();
if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u)
    ClearP3Bit1Atomic();
else
    SetP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 83
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveSignalDriveStatusPersist(void);
extern void ObserveSignalDriveStatusOutput(u8 rxd_high, u8 txd_high);
#define SIGNAL_DRIVE_STATUS_PERSIST() ObserveSignalDriveStatusPersist()
#define SIGNAL_DRIVE_STATUS_OUTPUT(rxd, txd) ObserveSignalDriveStatusOutput((rxd), (txd))
#define SIGNAL_DRIVE_STATUS_FUNCTION SignalDriveStatusPatternForeverForTest
#else
#define SIGNAL_DRIVE_STATUS_PERSIST() PersistControllerOperatingStatusByte()
static void SetSignalDriveStatusOutput(u8 rxd_high, u8 txd_high)
{
    if(rxd_high != 0u)
        SetP3Bit0Atomic();
    else
        ClearP3Bit0Atomic();
    if(txd_high != 0u)
        SetP3Bit1Atomic();
    else
        ClearP3Bit1Atomic();
}
#define SIGNAL_DRIVE_STATUS_OUTPUT(rxd, txd) SetSignalDriveStatusOutput((rxd), (txd))
#define SIGNAL_DRIVE_STATUS_FUNCTION SignalDriveStatusPatternForever
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 84
#if defined(PLEXTOR_HOST_TEST)
ObserveServiceConditionalControllerOperatingDisable();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 85
#if defined(PLEXTOR_HOST_TEST)
ObservePrepareReadVerifyCommandState();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 86
#if defined(PLEXTOR_HOST_TEST)
ObserveCommitTrackTailState();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 87
#if defined(PLEXTOR_HOST_TEST)
ObserveCompleteControllerMeasurementSequence();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 88
#if defined(PLEXTOR_HOST_TEST)
ObserveBuildControllerOperatingInterpolationGrid();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 89
#if defined(PLEXTOR_HOST_TEST)
bit0017_set = FirmwareReadBit0017();
#else
bit0017_set = ReadBit0017ForRangeValidation();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 90
#if defined(PLEXTOR_HOST_TEST)
packet_address = FW_NEAR_UNKNOWN_FE00;
#else
packet_address = (fw_addr_t)(void *)packet;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 91
#if defined(PLEXTOR_HOST_TEST)
packet_address = FW_NEAR_UNKNOWN_FE20;
#else
packet_address = (fw_addr_t)(void *)packet;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 92
#if defined(PLEXTOR_HOST_TEST)
lead_out_copy_address = FW_NEAR_UNKNOWN_FE30;
#else
lead_out_copy_address = (fw_addr_t)(void *)lead_out_copy;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 93
#if defined(PLEXTOR_HOST_TEST)
ObserveAttemptMediaState3QCapture();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 94
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x30u);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802A, 3u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x80u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803F, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803E, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803D, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803C, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803B, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01803A, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018039, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018038, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018037, 0u);
FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018036, 0u);
#else
{
    volatile u8 huge *controller = (volatile u8 huge *)FW_CONTROLLER_COMMAND_WINDOW;

    controller[0x2d] = 0x30u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u);
    controller[0x04] = 2u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
    controller[0x05] = 0u;
    controller[0x2a] = 3u;
    controller[0x32] = 0x80u;
    controller[0x35] = 0u;
    controller[0x34] = 0u;
    controller[0x33] = 0u;
    controller[0x3f] = 0u;
    controller[0x3e] = 0u;
    controller[0x3d] = 0u;
    controller[0x3c] = 0u;
    controller[0x3b] = 0u;
    controller[0x3a] = 0u;
    controller[0x39] = 0u;
    controller[0x38] = 0u;
    controller[0x37] = 0u;
    controller[0x36] = 0u;
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 95
#if defined(PLEXTOR_HOST_TEST)
if(InterceptServicePendingHostCommandWhenOperationFlagsPermit() != 0u)
    return;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 96
#if defined(PLEXTOR_HOST_TEST)
result = InterceptResetDriveControlAndInitializeControllerCalibration();
if(result != 0xffu)
    return result;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 97
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0580(0u);
FirmwareWriteBit0581(0u);
#else
ClearP3Bit0Atomic();
ClearP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 98
#if defined(PLEXTOR_HOST_TEST)
ObserveRefreshTocStateFromRawReadQRecordTable();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 99
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteBit0580(1u);
FirmwareWriteBit0581(0u);
#else
SetP3Bit0Atomic();
ClearP3Bit1Atomic();
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 100
#if !defined(PLEXTOR_HOST_TEST)
u8 packet[10];
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 101
#if !defined(PLEXTOR_HOST_TEST)
u8 packet[10];
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 102
#if !defined(PLEXTOR_HOST_TEST)
u8 lead_out_copy[4];
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 103
#if !defined(PLEXTOR_HOST_TEST)
/* RAM:fe29d2 */
u8 RestoreRawReadDescriptorsToQueueHead(u16 descriptor_count)
{
    u16 queued_count;
    u16 consumer_index;
    u16 index_mask;
    u8 accepted;

    ClearBit0547Atomic();
    queued_count = (u16)(descriptor_count + ReadBigEndianU16Near(0x1292u));
    accepted = (u8)(queued_count <= ReadBigEndianU16Near(0x129cu));
    if(accepted != 0u)
    {
        queued_count = (u16)(descriptor_count + ReadBigEndianU16Near(0x1292u));
        WriteBigEndianU16Near(0x1292u, queued_count);
        consumer_index = ReadBigEndianU16Near(0x1294u);
        index_mask = ReadBigEndianU16Near(0x129au);
        WriteBigEndianU16Near(0x1294u, (u16)(consumer_index - descriptor_count) & index_mask);
    }
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetSfrA9Bit2Atomic();
    return accepted;
}

/* RAM:fe2a0e */
u8 PLEXTOR_NEAR_FUNCTION DiscardRawReadDescriptorsFromQueueTail(u16 descriptor_count)
{
    u16 queued_count;
    u16 producer_index;
    u16 index_mask;
    u8 accepted;

    ClearBit0547Atomic();
    accepted = (u8)(descriptor_count <= ReadBigEndianU16Near(0x1292u));
    if(accepted != 0u)
    {
        queued_count = ReadBigEndianU16Near(0x1292u);
        WriteBigEndianU16Near(0x1292u, (u16)(queued_count - descriptor_count));
        producer_index = ReadBigEndianU16Near(0x1296u);
        index_mask = ReadBigEndianU16Near(0x129au);
        WriteBigEndianU16Near(0x1296u, (u16)(producer_index - descriptor_count) & index_mask);
    }
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetSfrA9Bit2Atomic();
    return accepted;
}
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 104
#if defined(PLEXTOR_HOST_TEST)
static void DispatchPublishWriteRawReadTransferPacket(s32 frame_position, u8 packet[4])
{
    u32 position;
    u32 total_seconds;

    if(frame_position < 0)
        frame_position += 450000l;
    position = (u32)frame_position;
    packet[2] = BinaryByteToPackedBcd((u8)(position % 75ul));
    total_seconds = position / 75ul;
    packet[1] = BinaryByteToPackedBcd((u8)(total_seconds % 60ul));
    packet[0] = BinaryByteToPackedBcd((u8)(total_seconds / 60ul));
    packet[3] = 2u;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, packet[0]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, packet[1]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, packet[2]);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, packet[3]);
}
#else
#define DispatchPublishWriteRawReadTransferPacket(frame_position, packet)                                                                                                                              \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        u16 packet_address = (u16)(packet);                                                                                                                                                            \
        ConvertFramePositionToBinaryMsfTuple((frame_position), (fw_addr_t)packet_address);                                                                                                             \
        ConvertThreeBinaryBytesToPackedBcd(packet_address);                                                                                                                                            \
        (packet)[3] = 2u;                                                                                                                                                                              \
        WriteControllerRegisterBlock25FromMemory((fw_addr_t)packet_address);                                                                                                                           \
    } while(0)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 105
#if defined(PLEXTOR_HOST_TEST)
static void DispatchCopyControllerBufferToHeaderLocal(u8 *destination, u32 controller_offset, u16 byte_count)
{
    u16 index;

    for(index = 0u; index < byte_count; ++index)
        destination[index] = FirmwareRead8(FirmwareAddressIncrement24(controller_offset, FW_ADDRESS_BANK_STRIDE + index));
}
#else
#define DispatchCopyControllerBufferToHeaderLocal(destination, controller_offset, byte_count) ((void)CopyFromControllerBufferWindow((controller_offset), (fw_addr_t)(destination), (byte_count)))
#endif
#endif
