#include "../media/active_write_pipeline.h"
#include "../media/decoder.h"
#include "../media/lead_in_out_descriptor_sequences.h"
#include "../media/lead_in_raw_sector_descriptor_sequence.h"
#include "../media/operating_code_selection.h"
#include "../media/raw_read_acquisition_plan.h"
#include "../media/raw_read_frame_prefix_wait.h"
#include "../media/raw_read_frame_sequence.h"
#include "../media/raw_sector_sequence.h"
#include "../media/read_acquisition_request.h"
#include "../media/session_boundary.h"
#include "../scsi/mode_sense_response.h"
#include "../scsi/play_audio.h"
#include "../scsi/raw_sector_host_transfer_poll.h"
#include "../scsi/read_cd_track_coverage.h"
#include "../scsi/vendor_f3_state.h"
#include "arithmetic.h"
#include "configuration_range_update.h"
#include "controller_calibration.h"
#include "controller_calibration_outcome.h"
#include "controller_data_out.h"
#include "controller_operating_code_configuration.h"
#include "controller_operating_state_initialization.h"
#include "controller_parameters.h"
#include "controller_record_page_update.h"
#include "controller_transition_wait.h"
#include "coroutine_entries.h"
#include "drive_control_reset.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "integer_division.h"
#include "integer_float32_conversions.h"
#include "leaf_workers.h"
#include "radial_model_profile_coordinate.h"
#include "raw_sector_descriptor_scratch.h"
#include "status_mapping.h"
#include "timing.h"
#include "track_state.h"

/* Addressless typed-call boundaries. */
void MovePickupBy3DirectionTrueForPendingNudge(void)
{
    MovePickupByByteDistance(3u, 1u);
}

void MovePickupBy6DirectionTrueForPendingNudge(void)
{
    MovePickupByByteDistance(6u, 1u);
}

u8 PrepareRawReadFramePrefixOmittingFinalFrameNoWait(void)
{
    return PrepareRawReadFramePrefixAndOptionallyWait(1u, 0u);
}

u8 PrepareRawReadFramePrefixIncludeFinalFrameNoWait(void)
{
    return PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u);
}

u8 PrepareRawReadFramePrefixIncludeFinalFrameAndWait(void)
{
    return PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u);
}

void UpdateControllerBufferRecordPagePreserveZero(s32 frame_position)
{
    UpdateControllerBufferRecordPageForFramePosition(frame_position, 0u);
}

u8 AcquireFixedRawReadFramePrefixWindowMode2Source4(void)
{
    return AcquireFixedRawReadFramePrefixWindow(2u, 4u);
}

u8 PrepareRawReadAcquisitionPlanPrimary(void)
{
    return PrepareRawReadAcquisitionPlan(0u);
}

u8 AcquireLeadInRawSectorDescriptorSequenceInitialAtip(void)
{
    return AcquireLeadInRawSectorDescriptorSequence(1u, 0u);
}

u8 AcquireLeadInRawSectorDescriptorSequenceInitialNonAtip(void)
{
    return AcquireLeadInRawSectorDescriptorSequence(0u, 0u);
}

u8 AcquireLeadInRawSectorDescriptorSequenceRetry(void)
{
    return AcquireLeadInRawSectorDescriptorSequence(0u, 1u);
}

u8 EnsureOpcForSelectedWriteSpeedForCloseTrack(void)
{
    return EnsureOpcForSelectedWriteSpeed();
}

u8 AcquireLeadInAndLeadOutRawSectorDescriptorSequencesForCloseTrack(void)
{
    return AcquireLeadInAndLeadOutRawSectorDescriptorSequences();
}

void ServiceEightChannelMeasurementCaptureForDispatch(void)
{
    ServiceEightChannelMeasurementCapture();
}

void PrepareAndDispatchReadOrSeekAcquisitionAtLbaForSeek10(u32 lba)
{
    PrepareAndDispatchReadOrSeekAcquisitionAtLba((s32)lba);
}

void PrepareAndDispatchReadOrSeekAcquisitionAtLbaForSeek6(u32 lba)
{
    PrepareAndDispatchReadOrSeekAcquisitionAtLba((s32)lba);
}

void ExecuteModeSense6ResponseFromC(u16 allocation_length)
{
    ExecuteModeSense6Or10Response(6u, allocation_length);
}

void ExecuteModeSense10ResponseFromC(u16 allocation_length)
{
    ExecuteModeSense6Or10Response(10u, allocation_length);
}

void BuildRawSectorDescriptorScratchZeroInputsForAtomicEnqueue(void)
{
    BuildRawSectorDescriptorScratch(0u, 0u);
}

void DisableControllerOperatingStateForReadPositionStatus(void)
{
    SetControllerOperatingEnableState(0u);
}

void ServiceMediaQPacketCaptureForReadPositionStatus(void)
{
    ServiceMediaQPacketCaptureWithOperatingCodeBudget();
}

void RestartCoroutine1AtF853cfAndYield(void)
{
    RestartCoroutine1AndYield(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
}

void RestartCoroutine1AtF84B0BAndYield(void)
{
    RestartCoroutine1AndYield(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
}

void RestartCoroutine1AtF8F07CAndYield(void)
{
    RestartCoroutine1AndYield(FW_ENTRY_CLEAR_STARTUP_STATE_AND_SIGNAL_FAILURE_COROUTINE1);
}

void RestartCoroutine1AtF85D74AndYield(void)
{
    RestartCoroutine1AndYield(FW_ENTRY_RUN_MEDIA_STATE_COROUTINE1);
}

void ResetDriveControlStateToEightForState1Calibration(void)
{
    ResetDriveControlStateToEight();
}

u8 InitializeControllerOperatingStateAndRunCalibrationForState1(void)
{
    return InitializeControllerOperatingStateAndRunCalibration();
}

void PersistControllerCalibrationStateBlockForState1(void)
{
    PersistControllerCalibrationStateBlock();
}

void UpdateCalibrationOutcomeStateForState1(u8 calibration_stage)
{
    UpdateCalibrationOutcomeStateForStage(calibration_stage);
}

void CalibrateControllerPort8097WithCommand08ForState1(void)
{
    CalibrateControllerPort8097WithCommand08();
}

void SetControllerOperatingEnableForReadTargetAdjustment(void)
{
    SetControllerOperatingEnableState(1u);
}

void SelectAndApplyOperatingCodeForMediaStateEnabled(void)
{
    SelectAndApplyOperatingCodeForMediaState(1u);
}

void WaitForControllerMeasurementWithinToleranceOrExpiryFalseFalse(void)
{
    WaitForControllerMeasurementWithinToleranceOrExpiry(0u, 0u);
}

void PLEXTOR_NEAR_FUNCTION FinalizeMediaCoroutineState2NearAdapter(void)
{
    FinalizeMediaCoroutineState2();
}

void PLEXTOR_NEAR_FUNCTION FinalizeMediaCoroutineState5NearAdapter(void)
{
    FinalizeMediaCoroutineState5();
}

void PLEXTOR_NEAR_FUNCTION ServiceMediaState0IndexedWaitNearAdapter(void)
{
    FUN_RAM_f8873d();
}

void PLEXTOR_NEAR_FUNCTION TryAdvanceMediaState0NearAdapter(void)
{
    TryAdvanceMediaCoroutineState0ToState3();
}

void PLEXTOR_NEAR_FUNCTION LoadMediaCatalogNumberNearAdapter(void)
{
    LoadMediaCatalogNumberCharacters();
}

void ExecuteControllerDataOutTransferFromBufferOffset(u32 transfer_length, u32 buffer_offset)
{
    ExecuteControllerDataOutTransfer(buffer_offset, transfer_length);
}

void ExecuteControllerDataInTransferFromBufferOffset(u32 transfer_length, u32 buffer_offset)
{
    ExecuteControllerDataInTransfer(buffer_offset, transfer_length);
}

void CaptureControllerPacketFlag1(void)
{
    AdvanceControllerParameterEaEbCalibrationSequence();
}

void CaptureControllerPacketFlag2(void)
{
    ServiceControllerParameterEaEbCalibrationState();
}

void FinalizeCloseTrackState(void)
{
    (void)FinalizeActiveWritePipelineAndTrackState();
}

void FinalizeControllerOperatingState(void)
{
    ConfigureControllerForCurrentOperatingCode(0u);
}

void WaitControllerTimingUnits(u16 units)
{
    WaitTimer1ScaledDuration(units, 0u);
}

u16 MeasureControllerTimingValue(u8 selector)
{
    return MeasureAverageSfrDbSamples(selector);
}

void StartControllerMeasurementEvaluation(void)
{
    ArmControllerThreeBytePacketCapture();
}

u8 PollControllerMeasurementEvaluation(void)
{
    return ParseControllerThreeBytePacket();
}

u8 CompareReadWindowBounds(u32 upper_bound, u32 lower_bound)
{
    return ClassifyReadCdFrameRangeTrackCoverage((s32)lower_bound, (s32)upper_bound);
}

void LoadTrackInformationByNumber(u8 track_number)
{
    (void)LoadTrackInformationByNumberFromInheritedB(track_number, 0u);
}

void FUN_RAM_fbb2c3(void) {}

void FUN_RAM_fbb2c4(void) {}

void ValidateAndStartPlayAudioMsfRangeFromCdbTuplesForC(void)
{
    ValidateAndStartPlayAudioMsfRangeFromCdbTuples(FW_NEAR_UNKNOWN_0B10, FW_NEAR_UNKNOWN_0B0D, 0u);
}

u8 AcquireRawSectorSequenceForRangeAndWait(u32 frame_count)
{
    return AcquireRawSectorSequence(frame_count, 0u, 1u, 0u);
}

u8 AcquireRawSectorSequenceMode2PreserveMain(u32 frame_count)
{
    return AcquireRawSectorSequence(frame_count, 2u, 1u, 1u);
}

u8 PLEXTOR_NEAR_FUNCTION AcquireRawSectorSequenceForResetRange(u32 frame_count, u8 q_metadata_mode, u8 preserve_main_buffer)
{
    return AcquireRawSectorSequence(frame_count, q_metadata_mode, preserve_main_buffer, 1u);
}

u16 PLEXTOR_NEAR_FUNCTION RepeatedRawReadWindowIndexRemainder20(u16 value)
{
    return (u16)DivideS16WithRemainder((s16)value, 20).remainder;
}

u16 ComputeDeviceBusyEventTimeValueNative(void)
{
    u16 countdown = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_FBB4);
    u32 dividend = MultiplyU32Low((u32)countdown, 0x00005be0ul);
    u32 divisor = MultiplyU32Low((u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47), 0x0002c000ul);

    return (u16)DivideU32QuotientNative(dividend, divisor);
}

void ExecuteVendorF3IndexedDriveCycleSequenceFromCdb(void)
{
    ExecuteVendorF3IndexedDriveCycleSequence(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D));
}

u8 ComputeControllerProfileByteFromCoordinateForC(u8 radial_coordinate, u16 radial_endpoint)
{
    return (u8)ConvertFloat32ToInteger32(ComputeRadialModelProfileCoordinate(radial_coordinate, radial_endpoint));
}

u8 DivideMediaState5PositionBy4500ForC(s32 position)
{
    return (u8)DivideS32QuotientNative(position, 4500l);
}

void PositionRawReadPipelineAtFramePositionFromC(s32 target_position)
{
    PositionRawReadPipelineAtFramePosition(target_position);
}

void EnqueueRawSectorDescriptorAtScratchAddress(void)
{
    EnqueueRawSectorDescriptor();
}

u8 ReadControllerPort8083ForEightChannelCapture(void)
{
    return WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_018083);
}

u8 ReadControllerPort8084ForEightChannelCapture(void)
{
    return WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_018084);
}

s32 ConvertFullTocMsfForA2Matcher(fw_addr_t tuple_address)
{
    return ConvertBinaryMsfToAdjustedFramePosition(tuple_address);
}

u8 UpdateAndProgramDriveConfigurationRangeF087Offset34Count2(void)
{
    return UpdateAndProgramDriveConfigurationRange(0x000f087ul, 0x34u, 2u);
}
