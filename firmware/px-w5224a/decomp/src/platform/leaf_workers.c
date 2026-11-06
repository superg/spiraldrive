#include "leaf_workers.h"
#include "../media/decoder.h"
#include "../media/pma_q_record_reset.h"
#include "../media/raw_read_acquisition_plan.h"
#include "../media/raw_read_frame_sequence.h"
#include "../media/raw_read_interleaved_q_batch.h"
#include "../media/raw_sector_header_inspect.h"
#include "../media/synthetic_raw_read_pattern.h"
#include "../media/toc_session_append.h"
#include "../media/toc_state_parse.h"
#include "../media/toc_state_rebuild.h"
#include "../media/working_track_raw_read_range.h"
#include "../scsi/cd_text_pack_validation.h"
#include "../scsi/full_toc_track_span.h"
#include "../scsi/raw_sector_range.h"
#include "../scsi/vendor_ec_record_list.h"
#include "../scsi/vendor_measurement_response.h"
#include "../startup/controller_calibration_configuration.h"
#include "../startup/disc_control_state_initialization.h"
#include "arithmetic.h"
#include "configuration_range_update.h"
#include "controller_buffer_atomic.h"
#include "controller_buffer_chunk_fill.h"
#include "controller_mapping.h"
#include "controller_operating_adjustment_response.h"
#include "controller_operating_interpolation_grid_column.h"
#include "controller_operating_interpolation_table_slice.h"
#include "controller_parameter_calibration_search.h"
#include "controller_parameters.h"
#include "controller_q_packet_process.h"
#include "controller_record_slot_preparation.h"
#include "controller_threshold_configuration.h"
#include "controller_word0251_sweep.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "media_interrupt_leaves.h"
#include "memory.h"
#include "pickup_move_controller_sequence.h"
#include "radial_position_table.h"
#include "raw_read_q_control.h"
#include "raw_sector_capture.h"
#include "raw_sector_descriptor_scratch.h"
#include "read_acquisition_calibration_sequence.h"
#include "read_acquisition_controller_config.h"
#include "scaled_radial_pickup_move_sequence.h"
#include "sfr_da_db_measurement.h"
#include "status_mapping.h"
#include "timer_debounce.h"
#include "track_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 6
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern u8 AcquireRequiredControllerRecordIndexRuns(u8 operation_mode);
extern u8 AcquireType2LeadInControllerRecordSequence(void);
extern u8 AcquireType2LeadOutControllerRecordSequence(void);
extern void BuildActiveControllerRecordBoundaryTable(void);
#define PLEXTOR_BINDING_SECTION 7
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern void ResetRawReadQueueAndTransferState(void);
extern s32 MapLogicalBlockAddressToControllerFramePosition(s32 logical_block_address);
extern void DisableControllerOperatingStateForReadPositionStatus(void);
extern void ServiceMediaQPacketCaptureForReadPositionStatus(void);
extern void YieldCoroutine0(void);
extern void RestartCoroutine1AtF853cfAndYield(void);
extern void RestartCoroutine1AtF85D74AndYield(void);
extern void RestartCoroutine1AtF84B0BAndYield(void);
extern void ResetDriveControlStateToEightForState1Calibration(void);
extern u8 InitializeControllerOperatingStateAndRunCalibrationForState1(void);
extern void PersistControllerCalibrationStateBlockForState1(void);
extern void UpdateCalibrationOutcomeStateForState1(u8 calibration_stage);
extern void CalibrateControllerPort8097WithCommand08ForState1(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0013ForReadTargetAdjustment(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit001bForReadTargetAdjustment(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit001cForReadTargetAdjustment(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit001dForReadTargetAdjustment(void);
#define PLEXTOR_BINDING_SECTION 8
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void SetControllerOperatingEnableForReadTargetAdjustment(void);
extern void PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7ClearWithoutTimeout(void);
extern u8 PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7Clear500AndYield(void);
extern void WaitForControllerMeasurementWithinToleranceOrExpiryFalseFalse(void);
extern void CalibrateControllerPort8097WithCommand08(void);
extern u8 UpdateAndProgramDriveConfigurationRangeF087Offset34Count2(void);
#define PLEXTOR_BINDING_SECTION 9
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void PLEXTOR_NEAR_FUNCTION ServicePendingHostCommandDuringMediaOperation(void);
#define PLEXTOR_BINDING_SECTION 10
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void ServiceEightChannelMeasurementCaptureForDispatch(void);
#define PLEXTOR_BINDING_SECTION 11
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void PrepareAndDispatchReadOrSeekAcquisitionAtLbaForSeek10(u32 lba);
extern void PrepareAndDispatchReadOrSeekAcquisitionAtLbaForSeek6(u32 lba);
extern void ExecuteModeSense6ResponseFromC(u16 allocation_length);
extern void ExecuteModeSense10ResponseFromC(u16 allocation_length);
#define PLEXTOR_BINDING_SECTION 12
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 13
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 14
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 InitializeControllerOperatingStateAndRunCalibration(void);
extern void InitializeMediaState5PositionAndControllerState(void);
extern u8 TryReduceWriteSpeedForCurrentMediaState(u8 enabled);
extern void RecomputeMediaState5ControllerThreshold(void);
extern void BuildType2LeadInTocRecordSequence(u8 write_full_toc_descriptors);
extern u8 AcquireFortyFrameType2ControllerRecordBaseWindow(void);
extern u8 AcquireFortyFrameType2ControllerRecordWindow(void);
#define PLEXTOR_BINDING_SECTION 15
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void ConvergeControllerOperatingStageIndexToTarget(void);
extern void SignalDriveStatusPatternForever(void);
extern void PLEXTOR_NEAR_FUNCTION AccumulateControllerCommand0EMeasurementCapture(void);
extern void AccumulateMediaState5ControllerCalibrationSample(void);
extern u8 AcquireRawSectorSequenceMode2PreserveMain(u32 frame_count);
extern u8 PrepareRawReadFramePrefixOmittingFinalFrameNoWait(void);
extern u8 PrepareRawReadFramePrefixIncludeFinalFrameNoWait(void);
extern u8 PrepareRawReadFramePrefixIncludeFinalFrameAndWait(void);
extern void UpdateControllerBufferRecordPagePreserveZero(s32 frame_position);
extern u8 PLEXTOR_NEAR_FUNCTION ProgramVendorF3CalibrationHeaderPair(u16 header_word);
#define PLEXTOR_BINDING_SECTION 16
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 17
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 18
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 19
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 20
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 21
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 22
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 23
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 24
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 ClassifyThreeControllerSamplesByOrdering(void);
extern void InitializeType2ControllerRecordGeometry(void);
#define PLEXTOR_BINDING_SECTION 25
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 26
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void AdvanceControllerMeasurementSeries00And01Sequence(void);
extern void AdvanceControllerD8D9MeasurementSeries(void);
extern void AdvanceFixedControllerParameterSequence(void);
extern void ServiceControllerOperatingProfileInputMatches(void);
#define PLEXTOR_BINDING_SECTION 27
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 28
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 29
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 30
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void PLEXTOR_NEAR_FUNCTION ClearBit070FAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit070FAtomic(void);
extern void BuildRawSectorDescriptorScratchZeroInputsForAtomicEnqueue(void);
#define PLEXTOR_BINDING_SECTION 31
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 32
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 33
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 34
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void ExecuteReadHeaderSectorAcquisition(void);
extern void ExecuteReadVerifyRange(void);
extern void FinalizeReadVerifyCommandState(void);
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetExternalInterrupt0EnableAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearExternalInterrupt0EnableAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadP1Bit3Atomic(void);
#define PLEXTOR_BINDING_SECTION 35
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0038Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0038Atomic(void);
#define PLEXTOR_BINDING_SECTION 36
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0033Atomic(void);
extern void ValidateAndStartPlayAudioFrameRange(s32 start_frame_position, s32 frame_count, u8 playback_mode);
extern void ExecuteReadCommandRawSectorTransfer(void);
extern void ExecuteReadCdCommand(void);
#define PLEXTOR_BINDING_SECTION 37
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void SetReadCdMsfNegativeRangeBitsAtomic(void);
extern void ClearReadCdMsfTemporaryBitsIfSetAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit001EAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearCdTextRawReadSetupBitsAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0013ForCdTextRawReadAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION FinalizeCdTextRawReadModeBitsAtomic(void);
extern u8 RunControllerParameterCalibrationAndReferenceCapture(void);
#define PLEXTOR_BINDING_SECTION 38
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void PLEXTOR_NEAR_FUNCTION BuildFormatPlanPrimaryStagedRawQRecordSequence(void);
extern void PLEXTOR_NEAR_FUNCTION BuildFormatPlanSecondaryStagedRawQRecordSequence(void) PLEXTOR_REENTRANT;
#define PLEXTOR_BINDING_SECTION 39
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 PrepareFormatOperationRawReadState(void);
#define PLEXTOR_BINDING_SECTION 40
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 PLEXTOR_NEAR_FUNCTION RunControllerPort80F8CalibrationAttempt(u8 calibration_mode);
#define PLEXTOR_BINDING_SECTION 41
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 42
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 43
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 44
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 45
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 46
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void ResolveDriveControlStateToZero(void);
#define PLEXTOR_BINDING_SECTION 47
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 48
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 PrepareWriteCommandState(u32 start_address);
extern u8 StartOrContinueWriteCommandTransfer(u32 start_address, u32 block_count);
extern void PLEXTOR_NEAR_FUNCTION ExecuteWriteCommandRange(u32 start_address, u32 block_count);
extern void InitializeVendorF3AlternateReadAcquisitionState(void);
extern void InitializeVendorF3ReadAcquisitionMeasurementState(void);
#define PLEXTOR_BINDING_SECTION 49
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 50
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void YieldCoroutine3(void);
extern u8 PollWritePipelineTransferBatchStatus(void) PLEXTOR_REENTRANT;
#define PLEXTOR_BINDING_SECTION 51
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 52
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 53
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void PLEXTOR_NEAR_FUNCTION SetBit054AAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForDescriptorMode(void);
#define PLEXTOR_BINDING_SECTION 54
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementAtAdjustedTarget(void);
#define PLEXTOR_BINDING_SECTION 55
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void FUN_RAM_fbb2c3(void);
extern void FUN_RAM_fbb2c4(void);
extern void ExecuteControllerDataInTransferFromBufferOffset(u32 transfer_length, u32 buffer_offset);
extern void ExecuteControllerDataOutTransferFromBufferOffset(u32 transfer_length, u32 buffer_offset);
extern void ProcessModeSelectParameterList(u8 opcode, u16 parameter_list_length);
extern void ValidateAndStartPlayAudioMsfRangeFromCdbTuplesForC(void);
#define PLEXTOR_BINDING_SECTION 56
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern void MovePickupBy3DirectionTrueForPendingNudge(void);
extern void MovePickupBy6DirectionTrueForPendingNudge(void);
extern void DisableGlobalInterrupts(void);
extern void EnableFirmwareInterruptSources(void);
extern void PLEXTOR_NEAR_FUNCTION ClearRxdatBit3Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetRxdatBit3Atomic(void);
#define PLEXTOR_BINDING_SECTION 57
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
#define PLEXTOR_BINDING_SECTION 58
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
extern u8 FirmwareReadBit0014(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0014Atomic(void);
extern void SetP3Bit1Atomic(void);
extern void SetP3Bit0Atomic(void);
extern void ClearP3Bit0Atomic(void);
extern void ClearP3Bit1Atomic(void);

#define PLEXTOR_BINDING_SECTION 59
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 60
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 61
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 62
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32Low16(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 63
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 64
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fee8b0 */
u32 ResetRawReadTransferState(void)
{
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1459, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F06, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1466, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1467, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 0u);
#define PLEXTOR_BINDING_SECTION 65
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_145D, ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B00));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1455, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B06));
    return ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02) + 150ul;
}

/* RAM:feea32 */
u8 ValidateCurrentCommandLogicalBlockRange(void)
{
    s32 start_lba;
    s32 end_limit;
    u16 sense_code = 0x2100u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
        return ValidateCurrentCommandLogicalBlockRangeAlternate();

    start_lba = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
    {
        if(start_lba <= 300l && start_lba >= -75l)
        {
            u8 flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_1132);

            if((flags & 0xc0u) == 0xc0u)
                goto reject;
#define PLEXTOR_BINDING_SECTION 66
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        }
        end_limit = (s32)ReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION);
        {
            s32 media_end = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1);

            if(end_limit < media_end)
                end_limit = media_end;
        }
    }
    else
    {
        end_limit = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1);
    }

    if(start_lba < 0)
    {
        if(start_lba >= -140l)
            return 1u;
        if(start_lba > -1000l)
            goto reject;
        if(start_lba > -20000l)
        {
#define PLEXTOR_BINDING_SECTION 67
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
            return 1u;
        }
        if(start_lba > -30000l)
        {
#define PLEXTOR_BINDING_SECTION 68
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
            return 1u;
        }
#define PLEXTOR_BINDING_SECTION 69
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        return 1u;
    }

    if(start_lba <= end_limit && (u32)start_lba + ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B06) <= (u32)end_limit)
        return 1u;

reject:
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B18, 0xf0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1A, 5u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0B24, sense_code);
    {
        u32 adjustment = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0F0C);
        u32 current_start_lba = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02);

        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B1B, current_start_lba - adjustment);
    }
    return 0u;
}

/* RAM:fd3ba5 */
u8 ValidateCurrentCommandLogicalBlockRangeAlternate(void)
{
    s32 start_lba = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02);
    s32 media_end_minus_150;

    if(start_lba >= -150l && start_lba <= 300l)
    {
        u8 mode_bits = FirmwareReadNear8(FW_NEAR_UNKNOWN_1132);

        if((mode_bits & 0x10u) != 0u)
            goto reject_start_2100;
#define PLEXTOR_BINDING_SECTION 70
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    }

    media_end_minus_150 = (s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1) - 150ul);
    start_lba = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02);
    if(start_lba < 0)
    {
#define PLEXTOR_BINDING_SECTION 71
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        return 1u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) == 0u)
            goto reject_start_2100;
        if(start_lba < (s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F) - 150ul))
            goto reject_start_2100;
        return 1u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u || FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 3u || FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0xb9u)
        return 1u;

    {
        s32 end_limit = (s32)((u32)media_end_minus_150 - 0x507ul);

        if(start_lba > end_limit)
            goto reject_start_2100;
        if((u32)start_lba + ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B06) > (u32)end_limit)
            goto reject_end_2100;
    }
    return 1u;

reject_start_2100:
    SetSenseData(5u, 0x2100u);
    {
        u32 adjustment = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0F0C);
        u32 current_start_lba = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02);

        SetSenseInformation(current_start_lba - adjustment);
    }
    return 0u;

reject_end_2100:
    SetSenseData(5u, 0x2100u);
    {
        u32 adjustment = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0F0C);
        u32 current_start_lba = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02);
        u32 block_count = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B06);

        SetSenseInformation(block_count + current_start_lba - adjustment);
    }
    return 0u;
}

/* RAM:fa6cd1 */
void AdvanceOperatingUpdateStageAtFrameBoundaries(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 2u) == 0u)
        return;
    if(DispatchIsIndexedDriveStateActiveForOperatingUpdate(0x1fu) == 1u)
        return;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103C) & 3u) == 1u && (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) > 135000l)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103C, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103C) & 0xfcu) | 2u));
        DispatchConvergeControllerOperatingStageIndexToTarget();
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103C) & 3u) == 2u && (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) > 270000l)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103C) | 3u));
        DispatchConvergeControllerOperatingStageIndexToTarget();
    }
}

/* RAM:fa6d35 */
void ConvergeControllerOperatingStageIndexToTarget(void)
{
    u8 current_stage;
    u8 target_stage;
    u8 factor;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103C) & 3u) == 2u)
    {
        factor = FirmwareReadNear8(FW_NEAR_UNKNOWN_1042);
        current_stage = (u8)(factor * 3u);
        current_stage = (u8)(current_stage + FirmwareReadNear8(FW_NEAR_UNKNOWN_1041));
        factor = FirmwareReadNear8(FW_NEAR_UNKNOWN_1043);
        target_stage = (u8)(factor * 3u);
        target_stage = (u8)(target_stage + FirmwareReadNear8(FW_NEAR_UNKNOWN_1041));
    }
    else
    {
        factor = FirmwareReadNear8(FW_NEAR_UNKNOWN_1043);
        current_stage = (u8)(factor * 3u);
        current_stage = (u8)(current_stage + FirmwareReadNear8(FW_NEAR_UNKNOWN_1041));
        factor = FirmwareReadNear8(FW_NEAR_UNKNOWN_1044);
        target_stage = (u8)(factor * 3u);
        target_stage = (u8)(target_stage + FirmwareReadNear8(FW_NEAR_UNKNOWN_1041));
    }

    for(;;)
    {
        DispatchSubmitControllerByteAndWordForStageConvergence((u8)(current_stage + 0x80u), 0x0255u);
        if(current_stage == target_stage)
            return;
        DispatchWaitTimer1ScaledDurationForStageConvergence(5u, 1u);
        if(current_stage < target_stage)
            current_stage = (u8)(current_stage + 1u);
        else
            current_stage = (u8)(current_stage - 1u);
    }
}

/* RAM:facbd6 */
void FillControllerBufferRange(fw_addr_t buffer_address, u32 byte_count, u8 value)
{
    while(byte_count >= 0x8000ul)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DA, 0u);
        DispatchFillControllerBufferChunkAndWait(buffer_address, 0x8000u, value);
        buffer_address += 0x8000ul;
        byte_count -= 0x8000ul;
    }
    if(byte_count != 0ul)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DA, 0u);
        DispatchFillControllerBufferChunkAndWait(buffer_address, (u16)byte_count, value);
    }
}

/* RAM:fe858a */
/* The target binding passes the byte first so C251 can preserve the following
 * 24-bit address in DR4 without overlapping ACC's R11 register. */
u8 PLEXTOR_NEAR_FUNCTION CompleteRawSectorCaptureAndClearBuffers(fw_addr_t main_buffer_address, u8 preserve_main_buffer, u8 clear_subchannel_slot)
{
    fw_addr_t subchannel_slot_address;

    DispatchIncrementPackedBcdMsfForSectorCapture(FW_NEAR_UNKNOWN_1428);
    DispatchWriteControllerRegisterBlock25ForSectorCapture(FW_NEAR_UNKNOWN_1428);
    if(DispatchCaptureRawSectorForSectorCapture(main_buffer_address, 6u) == 0u)
    {
        DispatchSetSenseDataForSectorCapture(4u, 0x4400u);
        DispatchSetSenseInformationForSectorCapture(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598));
        return 0u;
    }

    if(preserve_main_buffer != 1u)
        DispatchFillControllerBufferForSectorCapture(main_buffer_address, 0x18ul, 0u);
    if(clear_subchannel_slot == 1u)
    {
        subchannel_slot_address = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1566) + MultiplyU32Low(0x60ul, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1262));
        DispatchFillControllerBufferForSectorCapture(subchannel_slot_address, 0x60ul, 0u);
    }
    return 1u;
}

/* RAM:fe8a7c */
u8 ExecuteRawReadQRecordSequence(u8 first_record_index)
{
    u16 expected_count;
    s32 range_start;
    u32 range_count;

    if(DispatchBuildAndAcquireRawReadQRecordTableForSequence(first_record_index) == 0u)
        return 0u;

    expected_count = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_8010);
    range_start = (s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F) - 1000ul + (u32)expected_count);
    range_count = (u32)(u16)(1000u - expected_count);
    if(DispatchAcquireRawReadRangeAndWaitForQSequence(range_start, range_count) == 0u)
        return 0u;
    if(DispatchPollRawReadPipelineProgressForQSequence(1u) != 0u)
        return 1u;

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) == 1u)
        DispatchReportReadPipelineFailureForQSequence(8u);
    else if(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        DispatchReportSense030C09ForQSequence();
    else
        DispatchReportReadPipelineFailureForQSequence(2u);
    return 0u;
}

/* RAM:fa7eab */
void PLEXTOR_NEAR_FUNCTION ClassifyMediaCalibrationSampleOrderingAndPublishCode(void)
{
    u8 classification = DispatchClassifyThreeControllerSamplesByOrdering();
    u8 published_code;
    u16 lower_sample;
    u16 upper_sample;

    switch(classification)
    {
    case 1u:
        published_code = 0xffu;
        break;
    case 2u:
        published_code = 0xfdu;
        break;
    case 3u:
        published_code = 3u;
        break;
    case 4u:
        lower_sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_1047);
        upper_sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_1048);
        published_code = (u16)(upper_sample - lower_sample) <= 5u ? 3u : 1u;
        break;
    case 5u:
        lower_sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_1046);
        upper_sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_1047);
        published_code = (u16)(upper_sample - lower_sample) <= 5u ? 0xfdu : 0xffu;
        break;
    default:
        /* The classifier exhaustively returns only codes one through five. */
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1050, published_code);
}

/* RAM:fa7f11 */
u8 ClassifyThreeControllerSamplesByOrdering(void)
{
    u8 sample0 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1046);
    u8 sample1 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1047);
    u8 sample2 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1048);
    u8 ordering = 0u;

    if(sample0 == sample1 && sample1 == sample2)
        return 1u;
    if(sample0 > sample1)
        ordering = 1u;
    if(sample2 < (ordering == 0u ? sample0 : sample1))
        ordering = 2u;
    if(ordering == 0u)
        return 2u;
    if(ordering == 2u)
        return 3u;
    return sample0 > sample2 ? 4u : 5u;
}

/* RAM:fa7d36 */
u8 ValidateMediaCalibrationConfigurationForCurrentState(void)
{
    u8 configuration_value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
        return (u8)(FirmwareRead8(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFC02E)) != 0xffu);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        configuration_value = FirmwareRead8(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFC030));
        return (u8)(configuration_value == 1u || configuration_value == 2u);
    }
    return 1u;
}

/* RAM:fa7cc9 */
void ServiceMediaState3ControllerCalibrationTransition(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1045) == 0u && DispatchValidateMediaCalibrationConfigurationForState3() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        DispatchResetDriveControlStateToEightForState3();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) | 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B8, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) | 0x0cu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x2au);
        DispatchPersistControllerOperatingStatusByteForState3();
        DispatchSignalDriveStatusPatternForeverForState3();
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB0) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        DispatchRestartCoroutine1AtF84B0BAndYieldForState3();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 4u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    }
}

/* RAM:fa89d7 */
void ServiceControllerCommand0EMeasurementCapture(void)
{
    u8 flags;
    u32 minimum_frame;
    u32 current_frame;
    u8 saved_ie;
    u8 saved_a9;

    flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_1098);
    if((flags & 0x01u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0x02u) == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        return;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0x08u) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0x04u) != 0u)
        {
            minimum_frame = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1099);
            current_frame = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
            if((s32)current_frame < (s32)minimum_frame)
                return;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) | 0x08u));
    }

    saved_ie = FirmwareReadDirectSfr8(A8);
    saved_a9 = FirmwareReadDirectSfr8(A9);
    ClearBit0547Atomic();
    DispatchAccumulateControllerCommand0EMeasurementCapture();
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
#define PLEXTOR_BINDING_SECTION 72
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:febe9f */
u8 PLEXTOR_NEAR_FUNCTION PrepareSyntheticRawReadPatternRange(u8 use_prepared_acquisition_plan, s32 start_frame_position, s32 end_frame_position_exclusive)
{
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, (u32)start_frame_position);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 1u);

    if(use_prepared_acquisition_plan == 1u)
    {
        if(DispatchPrepareRawReadAcquisitionPlanForSyntheticRange(0u) == 0u)
            return 0u;
    }
    else
    {
        DispatchBuildRawReadQPacketForSyntheticRange(0l, FW_NEAR_UNKNOWN_1272);
    }

    return (u8)(DispatchGenerateSyntheticRawReadPatternRange(start_frame_position, end_frame_position_exclusive) != 0u);
}

/* RAM:f8793f */
void PLEXTOR_NEAR_FUNCTION ServiceMediaState5PostTargetThreshold(s32 target_frame_position)
{
    u16 configuration_word;
    s32 threshold;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) != 0x1fu)
        return;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F978) & 0x02u) != 0u)
    {
        configuration_word = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC);
        WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC, (u16)(configuration_word | 0x0004u));
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F977) != 1u)
        return;

    threshold = (s32)((u32)target_frame_position + 225ul);
    if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) <= threshold && (ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC) & 0x0004u) == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        return;

    DispatchAccumulateMediaState5ControllerCalibrationSample();
}

/* RAM:fe7a10 */
u8 PLEXTOR_NEAR_FUNCTION PollRawReadPipelineProgress(u8 poll_count)
{
    u8 poll_index;

    for(poll_index = 0u; poll_index < poll_count; ++poll_index)
    {
        if(DispatchWaitForReadAcquisitionForPipelineProgress() == 0u)
        {
            if(FirmwareReadBit0028() != 0u)
                return 0u;
            if(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) == 0u)
                return 0u;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
            break;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, (u8)(FirmwareReadBit0031() != 0u));
    }

    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u);
}

/* RAM:fe8a2b */
u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadRangeMode2WithTwoFrameAdjustment(s32 start_frame_address, u32 frame_count)
{
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, (u32)start_frame_address);
    if(AcquireRawSectorSequenceMode2PreserveMain(frame_count - 2ul) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    if(PrepareRawReadFramePrefixOmittingFinalFrameNoWait() == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    if(PrepareRawReadFramePrefixIncludeFinalFrameNoWait() == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);
    return (u8)(PrepareRawReadFramePrefixIncludeFinalFrameAndWait() != 0u);
}

/* RAM:fed0bc */
void UpdateThirtyTwoControllerBufferRecordPositionsIfBlockChanged(s16 position_block_index)
{
    u32 base_position = (u32)(s32)position_block_index << 5;
    u8 index;

    if(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FC44) == base_position)
        return;

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_FC44, base_position);
    for(index = 0u; index != 32u; ++index)
        UpdateControllerBufferRecordPagePreserveZero((s32)(base_position + (u32)index));
}

/* RAM:fd6761 */
u8 TryInitializeErasedDriveConfiguration(void)
{
    u8 index;

    ReadDriveConfigurationBytes(0u, FW_NEAR_UNKNOWN_1620, 0x00a0u);
    for(index = 0u; index != 0x5eu; ++index)
        if(FirmwareReadNear8((u16)(0x1620u + index)) != 0xffu)
            return 0u;

    if(UpdateAndProgramDriveConfigurationRange(FW_NEAR_UNKNOWN_1620, 0u, 0x00a0u) != 0u)
        return 1u;

    InitializeCoroutine1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
    return 0u;
}

/* RAM:fe2b01 */
u8 PLEXTOR_NEAR_FUNCTION TryRecoverRawReadPipelineFailure(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u && FirmwareReadBit0028() != 0u)
    {
        DispatchReportReadPipelineFailureForRecovery(1u);
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 0u)
    {
        DispatchReportReadPipelineFailureForRecovery(1u);
        return 0u;
    }

    if(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) != 0u && FirmwareReadBit0032() != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        return 1u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 0x02u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && ReadBigEndianU32Near(FW_NEAR_UNKNOWN_14B8) == 0u)
        return 1u;

    DispatchReportSense030C09ForRecovery();
    return 0u;
}

/* RAM:fd7b90 */
u8 FindTrackNumberForFramePosition(s32 frame_position)
{
    u8 track_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    u8 last_track_index;

    while(track_index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
    {
        fw_addr_t tuple_address = FW_NEAR_UNKNOWN_F2BF + (fw_addr_t)((u16)track_index * 6u);

        if(frame_position < ConvertBinaryMsfToAdjustedFramePosition(tuple_address))
            break;
        ++track_index;
    }

    last_track_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
    if(track_index > last_track_index)
        track_index = last_track_index;
    return FirmwareRead8(FW_HUGE_UNKNOWN_00F2B6 + (fw_addr_t)((u16)track_index * 6u));
}

/* RAM:feda7a */
u32 PLEXTOR_NEAR_FUNCTION SelectRawReadBufferAddressForLength(u32 component_length_bytes, u32 fallback_address)
{
    if(component_length_bytes == 12ul || component_length_bytes == 16ul)
        return ReadBigEndianU32Near(FW_NEAR_UNKNOWN_156E) + (ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1266) << 4);
    if(component_length_bytes == 96ul)
        return ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1566) + MultiplyU32Low(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1262), 96ul);
    if(component_length_bytes == 2056ul || component_length_bytes == 2324ul || component_length_bytes == 2332ul || component_length_bytes == 2336ul || component_length_bytes == 2352ul
        || component_length_bytes == 2048ul)
        return fallback_address;
    return ReadBigEndianU32Near(FW_NEAR_UNKNOWN_126A);
}

/* RAM:f8b76f */
void PersistVendorF3CalibrationHeaderBytes(u8 configuration_byte)
{
    u8 first_byte = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x7fu);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB4, first_byte);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB5, configuration_byte);
    if(ProgramVendorF3CalibrationHeaderPair((u16)(((u16)first_byte << 8) | configuration_byte)) == 0u)
        InitializeCoroutine1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
}

/* RAM:fce50c */
void PLEXTOR_NEAR_FUNCTION SetRadialModelMeasurementAndScale(u16 measurement)
{
    volatile float scale;

    if(measurement >= 1236u)
        measurement = 1236u;
    else if(measurement <= 396u)
        measurement = 396u;

    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_F99C, measurement);
    scale = (float)measurement;
    scale = scale * 686.0f;
    scale = 678522624.0f / scale;
    scale = scale * 65536.0f;
    scale = scale / 1000.0f;
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5, (u32)(s32)scale);
}

/* RAM:f8877b */
u8 PLEXTOR_NEAR_FUNCTION ExecuteReadAcquisitionDirectionSelectedMove(void)
{
    u8 result;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0x40u) == 0u)
        result = DispatchSelectReadAcquisitionTargetForDirectionBitClear(1u);
    else
        result = DispatchSelectReadAcquisitionTargetForDirectionBitSet(1u);
    if(result != 0u)
        return result;

    DispatchConfigureControllerPorts8085And8086Value80AndYield();
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBD) <= 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBD, 0x14u);
        DispatchResetControllerPort8087And808A808BUnlessFlag0E();
        DispatchClearControllerPorts8088And8089UnlessFlag0E();
        DispatchMovePickupByDistanceInChunks((u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) >> 6) & 1u), 10u);
    }
    return 0u;
}

/* RAM:fa6da1 */
void RunVendorF6ReadAcquisitionCoroutine1(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) & 0xfcu) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) | 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);

    if(ExecuteReadAcquisitionCalibrationSequence() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) & 0xfbu));
    RestartCoroutine1AtF85D74AndYield();
}

/* RAM:f8858b */
u8 ReturnMediaState6CalibrationSequenceSuccess(void)
{
    return 1u;
}

/* RAM:f89590 */
void ApplyMediaCalibrationOffsetForCurrentWriteSpeed(void)
{
    u8 write_speed_x = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);

    if(write_speed_x == 4u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FABD, FirmwareReadNear8(FW_NEAR_UNKNOWN_FADD));
    else if(write_speed_x == 8u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FABE, FirmwareReadNear8(FW_NEAR_UNKNOWN_FADE));
    else if(write_speed_x == 16u || write_speed_x == 20u || write_speed_x == 24u || write_speed_x == 32u || write_speed_x == 40u || write_speed_x == 52u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FABF, FirmwareReadNear8(FW_NEAR_UNKNOWN_FADF));
}

/* RAM:f89fb5 */
u8 PLEXTOR_NEAR_FUNCTION SetWriteSpeedStatusBit3AndReturnFalse(void)
{
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0008u));
    return 0u;
}

/* RAM:f98993 */
void RefineMediaStrategyRecordIndexFromAtipIdentification(void)
{
    u16 identifier;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) != 0xfcu)
        return;
    identifier = BuildMediaStrategyIdentifierFromAtipBytes();
    (void)GetAtipStrategyLowNibble();
    switch(identifier)
    {
    case 0x0241u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd2u);
        break;
    case 0x0244u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd1u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xe1u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xb1u);
        break;
    case 0x0245u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd3u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xe4u);
        break;
    case 0x0248u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xc6u);
        break;
    case 0x0249u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd5u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xe2u);
        break;
    case 0x024au:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd6u);
        break;
    case 0x024bu:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd4u);
        break;
    case 0x024cu:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd7u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xe3u);
        break;
    case 0x024fu:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd8u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xe1u);
        break;
    case 0x0243u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u ? 0xd0u : 0xbcu);
        break;
    default:
        break;
    }
}

/* RAM:f98953 */
void SelectMediaStrategyRecordIndexFromAtipLeadIn(void)
{
    s32 lead_in = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) | 1u));
    if(lead_in == -11594l)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd8u);
    else if(lead_in == -10925l)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xe1u);
    else if(lead_in == -10926l)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd1u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0xd0u);
    RefineMediaStrategyRecordIndexFromAtipIdentification();
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 DivideU32QuotientWithoutRuntimeLibrary(u32 dividend, u32 divisor)
{
    u32 quotient = 0ul;
    u32 remainder = 0ul;
    u8 bit_count = 32u;

    if(divisor == 0ul)
        return 0ul;

    do
    {
        u8 high_bit = (u8)(dividend >> 31);
        u8 remainder_high_bit = (u8)(remainder >> 31);
        dividend <<= 1;
        remainder = (remainder << 1) | (u32)high_bit;
        quotient <<= 1;
        if(remainder_high_bit != 0u || remainder >= divisor)
        {
            remainder -= divisor;
            quotient |= 1ul;
        }
        bit_count--;
    } while(bit_count != 0u);
    return quotient;
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 DivideS32ByU16TruncatingTowardZero(s32 dividend, u16 divisor)
{
    u32 magnitude = (u32)dividend;
    u32 quotient;

    if(dividend < 0)
        magnitude = 0ul - magnitude;
    quotient = DivideU32QuotientWithoutRuntimeLibrary(magnitude, (u32)divisor);
    return dividend < 0 ? (s32)(0ul - quotient) : (s32)quotient;
}

/* RAM:fa3173 */
u32 ComputeAndPublishMediaCapacityGeometry(void)
{
    u32 adjusted = ReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION) - 0x0a78ul;
    u8 group = (u8)DivideU32QuotientWithoutRuntimeLibrary(adjusted, 0x15f0ul);
    u32 group_span = MultiplyU32Low((u32)group, 0x15f0ul);
    u16 remainder = (u16)(adjusted - group_span);
    u16 subgroup = (u16)DivideU32QuotientWithoutRuntimeLibrary((u16)(remainder - 0x0138u), 0x27ul);
    u32 geometry = ((u32)subgroup << 5) + MultiplyU32Low((u32)group, 0x1100ul);

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C96, MultiplyU32Low((u32)group, 0x0088ul) + subgroup);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C9A, geometry - 1ul);
    return geometry;
}

/* RAM:fa311d */
u8 InitializeMediaCapacityGeometryResponse(void)
{
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B2A, 0x001a0101ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B2E, 0x0007b200ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B32, 0ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B36, 0ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B3A, 0ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B3E, 0x20ul);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u)
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B42, 0x400ul);
    else
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B42, ComputeAndPublishMediaCapacityGeometry());
    return 1u;
}

/* RAM:fafab0 */
u8 IsWritePacketSizeExactMultiple(u32 value)
{
    u32 packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B);
    u32 quotient = DivideU32QuotientWithoutRuntimeLibrary(value, packet_size);

    return (u8)(MultiplyU32Low(packet_size, quotient) == value);
}

/* RAM:fb5c02 */
void PLEXTOR_NEAR_FUNCTION AdvanceWorkingTrackPacketBoundaryFromProgress(void)
{
    u32 packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    u32 progress = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B);
    u32 packet_count = DivideU32QuotientWithoutRuntimeLibrary(progress, packet_size);
    u32 remainder;
    u32 span;

    packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    progress = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B);
    remainder = progress - MultiplyU32Low(packet_size, packet_count);
    if(remainder != 0ul)
        packet_count++;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        u32 candidate;

        packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
        span = MultiplyU32Low(packet_size + 7ul, packet_count);
        candidate = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD49) + span;
        if(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45) < candidate)
        {
            packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
            span = MultiplyU32Low(packet_size + 7ul, packet_count);
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD45, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD49) + span);
        }
    }
    else
    {
        packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
        span = MultiplyU32Low(packet_size + 7ul, packet_count);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD45, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45) + span);
    }
}

/* RAM:fafa20 */
u8 PLEXTOR_NEAR_FUNCTION ValidateAndSetReservedTrackEndFrame(u32 requested_block_count)
{
    u32 packet_size;
    u32 end_frame;
    s32 lead_out_limit;

    if(requested_block_count == 0ul)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) != 0u)
    {
        u32 packet_count;

        packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B);
        if(packet_size == 0ul)
        {
            SetCommandSequenceErrorSense();
            return 0u;
        }
        if(IsWritePacketSizeExactMultiple(requested_block_count) == 0u)
        {
            HandleUnsupportedScsiCommand();
            return 0u;
        }

        packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B);
        packet_count = DivideU32QuotientWithoutRuntimeLibrary(requested_block_count, packet_size);
        packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B);
        end_frame = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) + MultiplyU32Low(packet_size + 7ul, packet_count) - 5ul;
    }
    else
    {
        end_frame = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) + requested_block_count + 2ul;
    }
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD41, end_frame);

    lead_out_limit = (s32)ReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION);
    end_frame = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41);
    if((s32)end_frame > lead_out_limit)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }
    return 1u;
}

/* RAM:fb892e */
void PLEXTOR_NEAR_FUNCTION FinalizePmaStartFramePosition(void)
{
    s32 fallback_start = (s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F) - 1000ul);
    s32 lead_in_start = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F);
    s32 pma_start = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1143);

    if(pma_start <= fallback_start)
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1143, (u32)fallback_start);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x80u));
        return;
    }

    if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) < (s32)((u32)lead_in_start - 1ul))
    {
        s32 delta = (s32)((u32)lead_in_start - ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1143));
        s32 quotient = DivideS32ByU16TruncatingTowardZero(delta, 10u);
        s32 remainder = (s32)((u32)delta - MultiplyU32Low((u32)quotient, 10ul));

        if(remainder >= 5)
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1143, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1143) - (u32)(10 - remainder));
        else if(remainder > 0)
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1143, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1143) + (u32)remainder);
    }
    else if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1143) < (s32)((u32)lead_in_start - 7ul))
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1143, (u32)lead_in_start - 10ul);
    }
    else
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1143, (u32)lead_in_start);
    }
}

/* RAM:fbd7d6 */
void ExecuteVendorF6ReadAcquisitionAndReportStatus(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 1u)
    {
        u8 status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        u16 asc_ascq = ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + ((fw_addr_t)status << 1));
        u8 sense_key;

        status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        sense_key = FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + status);
        SetSenseData(sense_key, asc_ascq);
        return;
    }

    NormalizeWriteSpeedForCurrentMedia();
    InitializeCoroutine1(FW_ENTRY_RUN_VENDOR_F6_READ_ACQUISITION_COROUTINE1);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) | 4u));

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) == 1u)
        YieldCoroutine0();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 0u)
    {
        SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));
        return;
    }

    while(((FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) >> 2) & 1u) == 1u)
    {
        YieldCoroutine0();
        if(ServiceImmediatePendingHostCommandAndReturnTrue() == 0u)
            return;
    }
    SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08D));
}

/* RAM:fbc51e */
u32 MapFixedPacketLogicalToPhysicalAddress(u32 logical_block_address)
{
    u32 packet_start;
    u32 packet_size;
    u32 completed_packet_count;
    u32 completed_packet_span;
    u32 offset_within_packet;
    u32 expanded_packet_span;

    if(IsFixedPacketAddressExpansionEnabled() == 0u)
        return logical_block_address;

    packet_start = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) - 0x96ul;
    /* The source reaches DIV through RAM:fd95b1.  Its callers establish a
       nonzero WorkingTrackPacketSize before this fixed-packet route. */
    packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    completed_packet_count = DivideU32QuotientWithoutRuntimeLibrary(logical_block_address - packet_start, packet_size);
    packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    completed_packet_span = MultiplyU32Low(packet_size, completed_packet_count);
    packet_start = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) - 0x96ul;
    offset_within_packet = logical_block_address - (packet_start + completed_packet_span);
    packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    expanded_packet_span = MultiplyU32Low(packet_size + 7ul, completed_packet_count);
    packet_start = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) - 0x96ul;

    return packet_start + expanded_packet_span + offset_within_packet;
}

/* RAM:fbd046 */
u8 FUN_RAM_fbd046(u32 target_frame_position)
{
    u32 working_packet_size = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD57);
    u32 physical_offset;
    u32 expanded_packet_size;
    u32 quotient;
    u32 remainder;

    if(working_packet_size != ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B))
        return 0u;

    physical_offset = MapFixedPacketLogicalToPhysicalAddress(target_frame_position - 0x96ul);
    physical_offset = physical_offset + 0x96ul - ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D);
    expanded_packet_size = working_packet_size + 7ul;
    /* The source invokes RAM:fd95b1 directly.  This path requires the
       wrapping packet-size-plus-seven divisor to remain nonzero. */
    quotient = DivideU32QuotientWithoutRuntimeLibrary(physical_offset, expanded_packet_size);
    remainder = physical_offset - MultiplyU32Low(expanded_packet_size, quotient);
    if(remainder != 0u || physical_offset > ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41))
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }
    return 1u;
}

/* RAM:fb9737 */
void UpdateWriteSpeedFlagsAfterPositionCountThreshold(void)
{
    u8 current_operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u32 previous_position;
    u32 current_position;
    s16 position_delta;
    u16 write_speed_flags;
    u8 threshold_enabled;

    if(current_operating_code < 0x14u)
        return;

    previous_position = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FB76);
    current_position = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FB6A);
    position_delta = (s16)(u16)(current_position - previous_position);
    write_speed_flags = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7);
    if((write_speed_flags & 0x0001u) == 0u || (ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0002u) == 0u)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
            threshold_enabled = 1u;
        else
            threshold_enabled = (u8)((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x1000u) != 0u);
    else
        threshold_enabled = 1u;
    if(threshold_enabled != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x10u)
        {
            if(position_delta > 199)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD6) | 1u));
                WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) | 0x0200u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) | 0x08u));
            }
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x18u && position_delta > 399)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD6) | 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) | 0x08u));
            AccumulateWriteSpeedFlagsFromPositionAndOperatingCode();
        }
    }
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_FB76, current_position);
}

/* RAM:fb96fd */
void FUN_RAM_fb96fd(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_125C) == FirmwareReadNear8(FW_NEAR_UNKNOWN_125D))
        return;
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) >> 1) & 3u) != 3u)
        return;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) & 2u) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u))
        UpdateWriteSpeedFlagsAfterPositionCountThreshold();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125D, FirmwareReadNear8(FW_NEAR_UNKNOWN_125C));
}

/* RAM:fbe56d */
void FUN_RAM_fbe56d(u8 value)
{
    WriteControllerParameterWord(0xee01u);
    WriteControllerParameterEF00AndConditionalD0Value(value);
    SetNoSenseData();
}

/* RAM:fbe98f */
void FUN_RAM_fbe98f(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfeu));
    SetNoSenseData();
}

/* RAM:fbe11f */
void PLEXTOR_NEAR_FUNCTION FUN_RAM_fbe11f(void)
{
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0BB0, ReadBigEndianU16Near(FW_NEAR_UNKNOWN_F06B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0BB2, FirmwareReadNear8(FW_NEAR_UNKNOWN_F06D));
}

/* RAM:fde9cf */
/* WR6 input, DR4 full product output, ERET. Both transfer-start callers
 * save the returned dword; zero bypasses the batch-count RAM read. */
u32 FUN_RAM_fde9cf(u16 value)
{
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0F18, value);
    if(value == 0u)
        return 0ul;
    return MultiplyU32Low(value, ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1463));
}

/* RAM:fbd495 */
/* ACC is an exact 0/1 selector: other byte values return without accesses.
 * Original callers use LCALL/RET; internal C callers use the C251 ABI. */
void PLEXTOR_NEAR_FUNCTION SetWritePipelineControllerEnable(u8 enabled)
{
    u8 shadow;

    if(enabled > 1u)
        return;
    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, enabled == 1u ? 0x80u : 0u);
    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
    if(enabled == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 1u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xfeu));
}

/* RAM:fbe586 */
/* The Boolean is ACC bit zero, not a C nonzero-byte test. */
void UpdateControllerPortSetupFlag6FromBoolean(u8 value)
{
    if((value & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0xbfu));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) | 0x40u));
        ConfigureControllerPorts8086And809DThrough80AE();
    }
    SetSenseData(0u, 0u);
}

/* RAM:fc2557 */
u16 PLEXTOR_NEAR_FUNCTION ScaleMediaStateTimingFromWord(u8 scale)
{
    u8 high0 = FirmwareReadNear8(FW_NEAR_UNKNOWN_156A);
    u8 high1 = FirmwareReadNear8(FW_NEAR_UNKNOWN_156B);
    u8 low0 = FirmwareReadNear8(FW_NEAR_UNKNOWN_156C);
    u8 low1 = FirmwareReadNear8(FW_NEAR_UNKNOWN_156D);
    u32 raw = ((u32)high0 << 24) | ((u32)high1 << 16) | ((u32)low0 << 8) | low1;
    u16 value = (u16)(raw - 6ul);
    if(scale == 0u)
        return value;
    return (u16)(MultiplyU32Low(value, scale) >> 8);
}

/* RAM:fc2495 */
/* The source helper returns WR6; its untyped decompiler call does not expose
 * that result. Publish each word before reading the next page byte. */
void PLEXTOR_NEAR_FUNCTION ApplyModePage02RuntimeTimingParameters(void)
{
    u16 value = ScaleMediaStateTimingFromWord(FirmwareReadNear8(FW_NEAR_UNKNOWN_E631));
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0C3F, value);
    value = ScaleMediaStateTimingFromWord(FirmwareReadNear8(FW_NEAR_UNKNOWN_E632));
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0C41, value);
}

/* RAM:fb94b5 */
u16 BuildMediaStrategyIdentifierFromAtipBytes(void)
{
    u16 high = (u16)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB89) >> 2) & 0x1fu);
    return (u16)((high << 7) | (u16)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB8A) & 0x7fu));
}

/* RAM:fb94da */
u8 GetAtipStrategyLowNibble(void)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB8B) & 0x0fu);
}

/* RAM:fc2412 */
void PLEXTOR_NEAR_FUNCTION ApplyModePage01RuntimeState(void)
{
    u8 first = FirmwareReadNear8(FW_NEAR_UNKNOWN_E625);
    u8 second;

    if(first != FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3C, first);
    second = FirmwareReadNear8(FW_NEAR_UNKNOWN_E626);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3D, second);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) + 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) + 0x14u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C3E, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_E627) & 1u) != 0u));
}

/* RAM:fc278a */
void PLEXTOR_NEAR_FUNCTION ApplyModePage07RuntimeState(void)
{
    u8 second = FirmwareReadNear8(FW_NEAR_UNKNOWN_E676);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C75, FirmwareReadNear8(FW_NEAR_UNKNOWN_E675));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C76, second);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C76) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C76, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C76) + 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C76, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C76) + 0x14u));
}

/* RAM:fc281c */
void PLEXTOR_NEAR_FUNCTION UpdateMediaStateCountdownFromModePage0D(void)
{
    u8 exponent = FirmwareReadNear8(FW_NEAR_UNKNOWN_E68A);
    u32 previous;
    u32 reload;

    if(exponent == 0u)
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FB4, 0ul);
        return;
    }
    previous = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB4);
    if((s8)exponent <= 9)
    {
        reload = 0x3ful;
        do
        {
            exponent--;
            if(exponent == 0u)
                break;
            reload += reload;
        } while(1);
    }
    else
    {
        reload = 0x7530ul;
        exponent = (u8)(exponent + 0xf6u);
        while(exponent != 0u)
        {
            reload += reload;
            exponent--;
        }
    }
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FB4, reload);
    /* The native code reloads RAM:000fb4 for the comparison, then again
       before the changed-value store.  Retain both volatile read groups. */
    reload = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB4);
    if(previous != reload)
    {
        reload = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB4);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FB0, reload);
    }
}

/* RAM:fe1f20 */
fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void)
{
    u16 descriptor_address;
    u32 cursor;
    u32 ring_span;

#define PLEXTOR_BINDING_SECTION 73
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    descriptor_address = GetLatestQueuedRawSectorDescriptorAddress();
    if(descriptor_address == 0u)
        cursor = 0u;
    else
        cursor = ReadBigEndianU32Near((u16)(descriptor_address + 4u)) + 0x930ul;
    ring_span = MultiplyU32Low(0x930ul, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_156A));
    if(ring_span - 1ul < cursor)
        cursor = 0u;
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_126E, cursor);
    return cursor;
}

/* RAM:f892dd */
/* Keep the short-circuit order: BITS:0032 suppresses both RAM reads, and a
 * zero reload suppresses the countdown read. */
u8 PLEXTOR_NEAR_FUNCTION HasMediaStateCountdownExpired(void)
{
    if(FirmwareReadBit0032() != 0u)
        return 0u;
    if(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB4) == 0ul)
        return 0u;
    return (u8)((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB0) < 0l);
}

/* RAM:fee370 */
u32 ComputeBlocksThroughNextThirtyTwoBoundary(u32 logical_address)
{
    u32 quotient_multiple;

    /* The source calls the signed divider. Its quotient is truncated toward
     * zero before the five shifts reconstruct a 32-block multiple. Negative
     * nonmultiples therefore deliberately produce 33 through 63 here. */
    if((logical_address & 0x80000000ul) == 0u)
    {
        quotient_multiple = logical_address & 0xffffffe0ul;
    }
    else
    {
        u32 magnitude = (u32)(~logical_address + 1ul);

        quotient_multiple = (u32)(0ul - ((magnitude >> 5) << 5));
    }
    return (u32)(0x20ul - (logical_address - quotient_multiple));
}

/* RAM:fee38a */
u8 DoesTransferCrossNextThirtyTwoBoundary(u32 logical_address)
{
    u32 blocks_to_boundary = ComputeBlocksThroughNextThirtyTwoBoundary(logical_address);

    return (u8)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B06) > blocks_to_boundary);
}

/* RAM:fee39e */
u32 InitializeHostTransferRange(void)
{
    u32 command_start = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02);
    u32 first_frame = (u32)(command_start + 0x96ul);
    u32 block_count = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B06);
    u16 block_size;

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1455, block_count);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_12BA, (u32)(block_count + first_frame - 1ul));
    block_size = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_0B00);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_145D, (u8)(block_size >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_145E, (u8)block_size);
    return first_frame;
}

/* RAM:fee3d0 */
void ApplyControllerRegisterBlock34And36Through3F(void)
{
    u8 value;
    fw_addr_t profile;
    u8 index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9F) != 1u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) == 3u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBC2) == 0u)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA1) == 0x0eu || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA1) == 0x10u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA1) == 0x05u)
            value = 2u;
        else
            value = 0x82u;
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA1) == 0x0eu || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA1) == 0x10u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA1) == 0x05u)
        value = 3u;
    else
        value = 0x83u;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, value);
    profile = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FBBE);
    for(index = 0u; index != 10u; ++index)
        FirmwareWriteController8((fw_addr_t)(FW_CONTROLLER_UNKNOWN_018036 + index), FirmwareRead8(FirmwareAddressIncrementLow16(profile, index)));
}

/* RAM:feefdb */
void EnqueueRawSectorDescriptor(void)
{
    u16 queued_count = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292);
    u16 high_water = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_129C);
    u16 descriptor_address;
    u16 source_address;
    u8 field;

    if(queued_count >= high_water)
        return;
    descriptor_address = (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_129E) + (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1296) * 0x16u));
    source_address = ReadBigEndianU16Near((u16)(descriptor_address + 2u));
    for(field = 0u; field != 4u; ++field)
    {
        u16 offset = (u16)(field << 2);

        WriteBigEndianU16Near((u16)(descriptor_address + offset), ReadBigEndianU16Near((u16)(source_address + offset)));
        WriteBigEndianU16Near((u16)(descriptor_address + offset + 2u), descriptor_address);
    }
    WriteBigEndianU16Near((u16)(descriptor_address + 0x10u), ReadBigEndianU16Near((u16)(source_address + 0x10u)));
    WriteBigEndianU16Near((u16)(descriptor_address + 0x14u), ReadBigEndianU16Near((u16)(source_address + 4u)));
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1296, (u16)((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1296) + 1u) & ReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A)));
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1292, (u16)(queued_count + 1u));
}

/* RAM:fedae4 */
void UpdateRawReadDescriptorFlagsFromControllerStatus31(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) <= 3u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_146C) & 0x40u) != 0x40u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12B6) & 0xbfu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_128A) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1287, 0u);
        return;
    }
    if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018031) & 0x18u) == 0x10u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12B6) & 0xbfu));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12B6) | 0x40u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_128A) == 1u)
    {
        u8 status = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018031);
        if((status & 0xf8u) == 0u)
            status = 1u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1287, status);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1287) & 0x40u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1287, 0x40u);
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1287) & 0x20u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1287, 0x20u);
    }
}

/* RAM:fef170 */
void ResetRawReadControllerTransferAndResume(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1462, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128A, 1u);
    UpdateRawReadDescriptorFlagsFromControllerStatus31();
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 74
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; preserves native low-word-first load order. */
static u32 ReadBigEndianU32FarLowWordFirst(fw_addr_t address)
{
    u16 low_word = FirmwareReadU16(FirmwareAddressIncrement24(address, 2u));
    u16 high_word = FirmwareReadU16(address);

    return ((u32)high_word << 16) | low_word;
}

/* Reconstruction helper; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 75
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 104
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 105
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; preserves the native two-word store order. */
static void WriteBigEndianU32FarLowWordFirst(fw_addr_t address, u32 value)
{
    FirmwareWriteU16(FirmwareAddressIncrement24(address, 2u), (u16)value);
    FirmwareWriteU16(address, (u16)(value >> 16));
}

/* RAM:fcff2e */
void InitializeWriteRawReadTransferLayout(void) PLEXTOR_REENTRANT
{
    u8 packet[4];

    WriteBigEndianU16(FW_NEAR_UNKNOWN_146D, 0x0800u);
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_1470, 0x00000800ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1469, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146B, 0xc0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146C, 0x43u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1280, 0u);

    DispatchPublishWriteRawReadTransferPacket((s32)ReadBigEndianU32Far(FW_NEAR_UNKNOWN_1598), packet);
}

/* RAM:fd8380 */
void MovePickupByScaledRadialDelta(u8 inward_direction, u16 magnitude)
{
    u16 scaled_delta;
    u16 radial_position;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 4u);
    scaled_delta = (u16)(MultiplyU32Low(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1000), magnitude) >> 16);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_112E, scaled_delta);
    radial_position = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1031);
    if(inward_direction == 0u)
        radial_position = (u16)(radial_position + scaled_delta);
    else
        radial_position = (u16)(radial_position - scaled_delta);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1031, radial_position);
    ExecuteScaledRadialPickupMoveSequence(inward_direction, scaled_delta);
}

/* RAM:fd4544 */
void HandlePlayAudio12Command(void)
{
    s32 start_frame_position;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D4D)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        SetSenseData(5u, 0x6400u);
        return;
    }

    start_frame_position = (s32)ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B0C);
    if(start_frame_position != -1l)
        start_frame_position += 150l;
    ValidateAndStartPlayAudioFrameRange(start_frame_position, (s32)ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B10), 0u);
}

/* RAM:fd4081 */
void HandleReadMasterCueCommand(void)
{
    u32 required_end;
    u32 last_possible_lead_out;
    u32 transfer_block_count;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D21)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    transfer_block_count = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B10) >> 8;
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B06, transfer_block_count);
    if(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B06) < 8ul)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B02, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1) + 0x1b76ul);
    required_end = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02) + 0x96ul;
    last_possible_lead_out = ReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION);
    if((s32)required_end > (s32)last_possible_lead_out)
    {
        SetSenseData(5u, 0x2100u);
        SetSenseInformation(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B02) + 20ul);
        return;
    }

    DispatchExecuteReadMasterCueResponse();
}

/* RAM:fd4f13 */
void StartFormat2RawSectorReadAtFramePosition(s32 frame_position)
{
    DispatchClearFormat2RawReadModeBits();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0B, 0u);
    DispatchResetControllerPortsForFormat2RawRead();

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018079, 0x44u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807D, 0x54u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807A, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018078, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807C, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018079, 0x40u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807D, 0x54u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0F4E, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0F50, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    DispatchSetFormat2RawReadModeBit();

    DispatchExecuteRawSectorReadRangeForFormat2((s32)((u32)frame_position + 2ul), (s32)((u32)frame_position + 3ul), 2u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B1A) == 0u)
    {
        DispatchBuildControllerShadowsForFormat2RawRead();
        DispatchApplyControllerShadowsForFormat2RawRead();
    }
    else
    {
        DispatchClearFormat2RawReadModeBit();
    }
}

/* RAM:fd3206 */
void HandleRead6Command(void)
{
    u32 transfer_block_count;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F) & 0xfcu) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B02, ReadBigEndianU32Far(FW_NEAR_HOST_COMMAND_OPCODE) & 0x001ffffful);
    transfer_block_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E);
    if(transfer_block_count == 0ul)
        transfer_block_count = 0x100ul;
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B06, transfer_block_count);
    PrepareReadVerifyCommandState();
    SetBit0046ForReadCommandAtomic();
    ExecuteReadCommandRawSectorTransfer();
    ClearBit0046ForReadCommandAtomic();
    FinalizeReadVerifyCommandState();
}

/* RAM:fd3340 */
void HandleReadCdCommand(void)
{
    u32 transfer_block_count;

    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B02, ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B0C));
    transfer_block_count = ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B10) >> 8;
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B06, transfer_block_count);
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_1455, transfer_block_count);
    PrepareReadVerifyCommandState();
    if((s32)ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B02) < 0 && FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
        SetBit0021ForReadCdAtomic();
    SetBit0046ForReadCommandAtomic();
    ExecuteReadCdCommand();
    ClearBit0046ForReadCommandAtomic();
    FinalizeReadVerifyCommandState();
}

/* RAM:fd338d */
void HandleReadCdMsfCommand(void)
{
    s32 start_frame_position;
    s32 end_frame_position;
    u32 transfer_block_count;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) != 0u || IsBinaryMsfTupleValid(FW_NEAR_UNKNOWN_0B0D) == 0u || IsBinaryMsfTupleValid(FW_NEAR_UNKNOWN_0B10) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) >= 90u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) <= 99u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 1u);

    start_frame_position = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_0B0D) - 150l;
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B02, (u32)start_frame_position);
    end_frame_position = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_0B10);
    transfer_block_count = (u32)(end_frame_position - start_frame_position - 150l);
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B06, transfer_block_count);
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_1455, transfer_block_count);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_102F) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);

    PrepareReadVerifyCommandState();
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && start_frame_position < 0)
        SetReadCdMsfNegativeRangeBitsAtomic();
    SetBit0046ForReadCommandAtomic();
    ExecuteReadCdCommand();
    ClearBit0046ForReadCommandAtomic();
    FinalizeReadVerifyCommandState();
    ClearReadCdMsfTemporaryBitsIfSetAtomic();
}

/* RAM:fd324d */
void HandleRead10Command(void)
{
    u8 invalid_fields;
    u32 transfer_block_count;

    invalid_fields = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x17u);
    invalid_fields |= FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10);
    invalid_fields |= (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13) & 0xfcu);
    if(invalid_fields != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 8u) != 0u)
        ResetRawReadQueueAndTransferState();
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B02, ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B0C));
    transfer_block_count = ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B10) >> 8;
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B06, transfer_block_count);
    if(transfer_block_count == 0ul)
    {
        DispatchPrepareAndDispatchReadOrSeekAcquisitionAtLba((s32)ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B02));
        return;
    }

    PrepareReadVerifyCommandState();
    SetBit0046ForReadCommandAtomic();
    ExecuteReadCommandRawSectorTransfer();
    ClearBit0046ForReadCommandAtomic();
    FinalizeReadVerifyCommandState();
}

/* RAM:fd32b6 */
void HandleRead12Command(void)
{
    u8 invalid_fields;
    u32 transfer_block_count;

    invalid_fields = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x17u);
    invalid_fields |= FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14);
    invalid_fields |= (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B15) & 0xfcu);
    if(invalid_fields != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 8u) != 0u)
        ResetRawReadQueueAndTransferState();
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B02, ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B0C));
    transfer_block_count = ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B10);
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0B06, transfer_block_count);
    if(transfer_block_count == 0ul)
    {
        DispatchPrepareAndDispatchReadOrSeekAcquisitionAtLba((s32)ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0B02));
        return;
    }

    PrepareReadVerifyCommandState();
    SetBit0046ForReadCommandAtomic();
    ExecuteReadCommandRawSectorTransfer();
    ClearBit0046ForReadCommandAtomic();
    FinalizeReadVerifyCommandState();
}

/* RAM:fd3173 */
u8 PLEXTOR_NEAR_FUNCTION ValidateStagedFirmwareRevisionPolicy(u8 buffer_id)
{
    u8 minimum_revision;
    u8 tens_digit;
    u8 ones_digit;
    u8 staged_revision;
    u32 controller_offset;

    ReadDriveConfigurationBytes(0x003aul, FW_NEAR_UNKNOWN_0B2A, 2u);
    minimum_revision = FirmwareRead8(buffer_id == 0xffu ? FW_NEAR_UNKNOWN_0B2A : FW_NEAR_UNKNOWN_0B2B);
    if(minimum_revision == 0xffu || minimum_revision == 0u)
        return 1u;

    controller_offset = buffer_id == 0xffu ? 0x00007fc0ul : 0ul;
    (void)CopyFromControllerBufferWindow(controller_offset, FW_NEAR_UNKNOWN_0B2A, 0x30u);
    tens_digit = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B44) - 0x30u);
    ones_digit = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B45) - 0x30u);
    staged_revision = (u8)((u8)(tens_digit * 10u) + ones_digit);
    return (u8)(staged_revision >= minimum_revision);
}

/* RAM:fd3124 */
u8 ExecuteBufferDataTransfer(u32 buffer_address, u32 transfer_length, u8 scsi_opcode)
{
    if(transfer_length == 0ul)
        return 5u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 8u);
    if(scsi_opcode == 0x3cu)
        DispatchExecuteControllerDataInTransfer(buffer_address, transfer_length);
    else
        DispatchExecuteControllerDataOutTransfer(buffer_address, transfer_length);
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) == 0x0du ? 5u : 4u;
}

/* RAM:fd2d40 */
u8 ValidateStagedMainFirmwareHeader(void) PLEXTOR_REENTRANT
{
    u8 installed_identity_length;
    u8 index;
    u8 staged_identity[30];
    u8 header_range_bytes[4];
    u32 controller_offset;
    u32 header_range_field;

    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_1316, 0ul);
    installed_identity_length = 0u;
    while(installed_identity_length < 30u && FirmwareRead8(PLEXTOR_IMAGE_HEADER_ADDRESS(FW_ROM_UNKNOWN_F80030) + installed_identity_length) != 0u)
        installed_identity_length++;
    if(installed_identity_length == 30u)
        return 0u;

    DispatchCopyControllerBufferToHeaderLocal(staged_identity, 0x30ul, installed_identity_length);
    for(index = 0u; index < installed_identity_length; ++index)
        if(staged_identity[index] != FirmwareRead8(PLEXTOR_IMAGE_HEADER_ADDRESS(FW_ROM_UNKNOWN_F80030) + (u32)index))
            return 0u;

    controller_offset = ReadBigEndianU32Far(FW_NEAR_UNKNOWN_1316);
    DispatchCopyControllerBufferToHeaderLocal(header_range_bytes, controller_offset, 4u);
    header_range_field = ((u32)header_range_bytes[0] << 24) | ((u32)header_range_bytes[1] << 16) | ((u32)header_range_bytes[2] << 8) | header_range_bytes[3];
    return (u8)((header_range_field & 0x00fffffful) < 0x00fffffful);
}

/* RAM:fd7694 */
s8 PLEXTOR_NEAR_FUNCTION ValidateLatestCdTextRawSectorPackBatch(void)
{
    u8 raw_read_status;
    u8 sense_index;
    u16 descriptor_address;
    u16 pack_count;
    u16 pack_index;
    u16 failure_count;
    u8 group_index;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        return 0;

    raw_read_status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    if(raw_read_status != 0u)
    {
        sense_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        failure_count = ReadBigEndianU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (u16)sense_index * 2u);
        sense_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        SetSenseData(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + sense_index), failure_count);
        return 0;
    }

    descriptor_address = GetLatestQueuedRawSectorDescriptorAddress();
    if(descriptor_address == 0u)
        return (s8)0xfe;
    if(FirmwareReadNear8((u16)(descriptor_address + 0x13u)) != 6u)
    {
        if(FirmwareReadNear8((u16)(descriptor_address + 0x10u)) != 0u)
            return 0x63;
        return IsRawReadQueueAtHighWater() != 0u ? 0 : (s8)0xfe;
    }

    pack_count = (u16)((u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0F50) - 2u) / 0x12u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_15EF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15F1, 0u);
    for(pack_index = 0u; pack_index < pack_count; ++pack_index)
    {
        if(ValidateCdTextPackAndAccumulatePayloadLengths(pack_index) == 0u)
        {
            failure_count = (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_15EF) + 1u);
            WriteBigEndianU16Near(FW_NEAR_UNKNOWN_15EF, failure_count);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15EE) > 1u)
            {
                for(group_index = 0u; group_index < 5u; ++group_index)
                {
                    u16 group_address = (u16)(0xf58cu + (u16)group_index * 2u);

                    if(ReadBigEndianU16Near(group_address) == 0xffffu)
                    {
                        WriteBigEndianU16Near(group_address, pack_index);
                        break;
                    }
                }
            }
        }
        YieldCoroutine0();
    }

    failure_count = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_15EF);
    if(failure_count == 0u)
        return 1;
    if(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_15EF) < 15u)
        SetBit001EAtomic();
    return 0x63;
}

/* RAM:fd77a1 */
void PLEXTOR_NEAR_FUNCTION InitializeCdTextRawReadAcquisition(u8 start_acquisition)
{
    u32 controller_buffer_capacity;
    u32 cdtext_destination;

    controller_buffer_capacity = ReadBigEndianU32Far(FW_NEAR_UNKNOWN_1556);
    cdtext_destination = ((u32)(u16)((u16)(controller_buffer_capacity >> 16) + 0x10u) << 16) | (u16)controller_buffer_capacity;
    cdtext_destination -= 0x00008ffcul;
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0F56, cdtext_destination);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0F50, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0F4E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4B, 0u);
    ClearCdTextRawReadSetupBitsAtomic();
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_F798, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_F796, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F06, 4u);
    ClearBit0013ForCdTextRawReadAtomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);

    if((start_acquisition & 1u) == 1u)
    {
        WriteBigEndianU32Far(FW_NEAR_UNKNOWN_1598, 0x96ul);
        DispatchReadAcquisitionRequest5ForPipelineTarget();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) != 0u)
            YieldCoroutine0();
    }

    FinalizeCdTextRawReadModeBitsAtomic();
}

/* RAM:fa453f */
void ResetRawReadPipelineForOptionalTarget(s32 target_position, u8 target_mode)
{
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();

    if(target_mode != 0u)
    {
        if(target_mode == 1u)
            target_position = MapLogicalBlockAddressToControllerFramePosition(target_position);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, (u32)target_position);
        DispatchReadAcquisitionRequest5ForPipelineTarget();
    }
    YieldCoroutine0();
}

/* RAM:fb0c9d */
void PersistControllerCalibrationStateBlock(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB1) != 1u)
        return;

    if(IsIndexedDriveStateActive(5u) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB1) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC4, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC2, 0x80u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC3, 0x80u);
    }
    (void)UpdateAndProgramDriveConfigurationRange(FW_NEAR_UNKNOWN_FAB4, 0x20u, 0x1au);
}

/* RAM:fbb396 */
void PLEXTOR_NEAR_FUNCTION ApplyPostTocControllerCalibrationProfile(void)
{
    u16 word_0319;

    (void)RunControllerParameterCalibrationAndReferenceCapture();
    DispatchCaptureControllerSampleSeriesForPostTocCalibration();
    word_0319 = ReadControllerWord(0x0319u);
    SubmitControllerWordPair((u16)(word_0319 + ReadBigEndianU16Near(FW_NEAR_UNKNOWN_F992)), 0x0319u);
    SelectAndWriteControllerParameterFCForOperatingCode(1u);
    SubmitControllerWordPair(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAB8), 0x0204u);
    SelectAndWriteControllerParameterF6ForOperatingCode(1u);
    SubmitControllerWordPair(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FBDE), 0x030au);
    SubmitControllerWordPair(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FBE0), 0x031au);
    WriteControllerParameterFBValueWithBit0Set(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16));
}

/* RAM:fba0bb */
/* The native inherited-B point index is explicit in the C interface. */
u8 PLEXTOR_NEAR_FUNCTION ResetTocAcquisitionAndOptionallyReinitializeController(u8 reinitialize_controller, u8 first_point_index)
{
    u32 target = 0x10000000ul;

    ResetTocAcquisitionStateFromPointIndexAndFinalize(first_point_index);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        target = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F) + 0x1194ul;
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, target);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));

    if(reinitialize_controller == 0u)
        return 1u;
    ResetDriveControlStateToEight();
    return InitializeControllerOperatingStateAndRunCalibration() != 0u ? 1u : 0u;
}

/* RAM:fb408e */
u8 ExecuteIndexedState0FMediaOperation(void)
{
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    YieldCoroutine0();

    if(EnsureOpcForSelectedWriteSpeed() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) | 0x01u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B8, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F1B7, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) & 0xf3u) | 0x04u));
        for(;;)
        {
            DispatchServiceIndexedState0FTimedOutputSequence();
            YieldCoroutine0();
        }
    }

    BuildFormatPlanPrimaryStagedRawQRecordSequence();
    BuildFormatPlanSecondaryStagedRawQRecordSequence();
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1455, 0ul);
    return PrepareFormatOperationRawReadState();
}

/* RAM:fb327f */
void PLEXTOR_NEAR_FUNCTION ExecuteFormatUnitMediaOperation(void)
{
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();
    YieldCoroutine0();

    if(DispatchEnsureOpcForFormatUnit() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        return;
    }
    BuildFormatPlanPrimaryStagedRawQRecordSequence();
    BuildFormatPlanSecondaryStagedRawQRecordSequence();
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1455, 0u);
    if(PrepareFormatOperationRawReadState() != 0u)
    {
        DispatchFormatUnitSuccessfulPreparation();
        SetNoSenseData();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 8u));
    DispatchFormatUnitFailedPreparation();
}

/* RAM:fb13b3 */
u8 PLEXTOR_NEAR_FUNCTION ConfigureControllerPorts80B2Through80C5ForCalibration(void)
{
    DispatchLoadControllerProfileForCalibrationSetup(0u);
    DispatchSetIndexedChannelForCalibrationSetup(0u, 0x99u);
    DispatchWriteControllerPortForCalibrationSetup(FW_CONTROLLER_UNKNOWN_0180B9, 0u);
    DispatchWriteControllerPortForCalibrationSetup(FW_CONTROLLER_UNKNOWN_0180B7, 0u);
    DispatchWriteControllerPortForCalibrationSetup(FW_CONTROLLER_UNKNOWN_0180B6, 0x18u);
    DispatchWriteControllerPortForCalibrationSetup(FW_CONTROLLER_UNKNOWN_0180B5, 1u);
    DispatchWriteControllerPortForCalibrationSetup(FW_CONTROLLER_UNKNOWN_0180B3, 0x30u);
    DispatchWriteControllerPortForCalibrationSetup(FW_CONTROLLER_UNKNOWN_0180B4, 0xd0u);
    DispatchWriteControllerPortForCalibrationSetup(FW_CONTROLLER_UNKNOWN_0180C5, 1u);
    DispatchWriteControllerPortForCalibrationSetup(FW_CONTROLLER_UNKNOWN_0180B2, 0x0fu);
    return 1u;
}

/* RAM:f8b384 */
void PulseControllerWord0253Value8000ForDuration(u16 duration_units)
{
    DispatchSubmitControllerWordForPulse0253(0x8000u, 0x0253u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_11FC, (u32)(s32)(s16)duration_units);
    while((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_11FC) > 0)
        DispatchYieldCoroutine1ForPulse0253();
    DispatchSubmitControllerWordForPulse0253(0u, 0x0253u);
}

/* RAM:f8b351 */
void PulseControllerWord0253Value7F00ForDuration(u16 duration_units)
{
    DispatchSubmitControllerWordForPulse0253(0x7f00u, 0x0253u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_11FC, (u32)(s32)(s16)duration_units);
    while((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_11FC) > 0)
        DispatchYieldCoroutine1ForPulse0253();
    DispatchSubmitControllerWordForPulse0253(0u, 0x0253u);
}

/* RAM:fb230a */
void PLEXTOR_NEAR_FUNCTION WaitForConfigurationBit7ClearAndRestartControllerState(void)
{
    DispatchSetP1BitsForControllerRestart();
    DispatchPulseWord0253ForControllerRestart(75u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 0u);
    DispatchSetP1BitsForControllerRestart();
    DispatchLoadProfileForControllerRestart(0x34u);
    DispatchWritePortForControllerRestart(FW_CONTROLLER_UNKNOWN_0180A8, 0x14u);
    DispatchSubmitWordForControllerRestart(0x0540u, 0x035fu);
    DispatchSetChannelForControllerRestart(3u, 0xaau);
    do
    {
        DispatchYieldForControllerRestart();
    } while((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAAC) & 0x80u) != 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 1u);
    DispatchResolveDriveControlForControllerRestart();
    DispatchRestartCoroutine1ForControllerRestart();
}

/* RAM:fb1286 */
u8 PLEXTOR_NEAR_FUNCTION EnterControllerState5AndMeasurePort80A9Minimum(void)
{
    u8 minimum_sample = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 5u);
    DispatchWritePortForState5Minimum(FW_CONTROLLER_UNKNOWN_0180A8, 2u);
    DispatchSetP1BothForState5Minimum();
    DispatchPulse7F00ForState5Minimum(75u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1221, 50u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1221) > 0)
    {
        u8 sample;

        DispatchWritePortForState5Minimum(FW_CONTROLLER_UNKNOWN_0180AA, 2u);
        sample = DispatchReadPortForState5Minimum(FW_CONTROLLER_UNKNOWN_0180A9);
        if(sample != 0u && (minimum_sample == 0u || sample <= minimum_sample))
            minimum_sample = sample;
    }

    if(minimum_sample < 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 4u));
        return 0u;
    }
    if(minimum_sample < 0x40u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBEC, 0u);
    if(DispatchIndexedStateForState5Minimum(4u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x20u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBEC, 1u);

    DispatchWritePortForState5Minimum(FW_CONTROLLER_UNKNOWN_0180A8, 0x12u);
    DispatchSetP1Bit1OnlyForState5Minimum();
    DispatchPulse8000ForState5Minimum(125u);
    DispatchClearP1BothForState5Minimum();
    DispatchSetP1BothForState5Minimum();
    DispatchSetP1BothForState5Minimum();
    DispatchPulse7F00ForState5Minimum(60u);
    DispatchSetChannelForState5Minimum(3u, 0xaau);
    return 1u;
}

/* RAM:fb135b */
u8 RetryControllerPort80F8Calibration(u8 calibration_mode)
{
    u8 attempts;
    u8 result;

    if(DispatchConfigureControllerPortsForCalibrationRetry() != 1u)
        return 0u;

    attempts = 3u;
    do
    {
        result = DispatchRunControllerCalibrationRetryAttempt(calibration_mode);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 1u) == 0u)
            DispatchSubmitControllerWordForCalibrationRetry(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FBDE), 0x030au);
        if(result == 1u)
        {
            DispatchLoadControllerProfileForCalibrationRetry(4u);
            return 1u;
        }
        --attempts;
    } while(attempts != 0u);

    DispatchPulseControllerWordForCalibrationRetry(500u);
    DispatchSetControllerFlagForCalibrationRetry(0u);
    return 0u;
}

/* RAM:fb9e47 */
void ResetDriveControlAndInitializeRadialModelDefaults(void)
{
    ResetDriveControlStateToEight();
    ResetControllerChannelsAndApplyDriveCode5(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 100u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) >= 0)
    {
    }
    ResetIndexedControllerChannel(2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1008, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9, 0x0632u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0CF5, 0x00013333ul);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0FFA, 0x0e20u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
}

/* RAM:fbb9fa */
void HandleWrite6Command(void)
{
    u32 start_address;
    u32 block_count;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F) & 0xfcu) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    start_address = ReadBigEndianU32Near(FW_NEAR_HOST_COMMAND_OPCODE) & 0x001ffffful;
    block_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E);
    if(DispatchPrepareWriteCommandState(start_address) == 1u)
        (void)StartOrContinueWriteCommandTransfer(start_address, block_count);
    else
        ExecuteWriteCommandRange(start_address, block_count);
}

/* RAM:fbba53 */
void HandleWrite10Command(void)
{
    u32 start_address;
    u32 block_count;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1fu) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13) & 0x3cu) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    start_address = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0C);
    block_count = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B11);
    if(DispatchPrepareWriteCommandState(start_address) == 1u)
        (void)StartOrContinueWriteCommandTransfer(start_address, block_count);
    else
        ExecuteWriteCommandRange(start_address, block_count);
}

/* RAM:fbbab0 */
void HandleWrite12Command(void)
{
    u32 start_address;
    u32 block_count;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x17u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) & 0x7fu) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B15) & 0xfcu) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    start_address = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0C);
    block_count = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B10);
    if(DispatchPrepareWriteCommandState(start_address) == 1u)
        (void)StartOrContinueWriteCommandTransfer(start_address, block_count);
    else
        ExecuteWriteCommandRange(start_address, block_count);
}

/* RAM:fd601d */
void HandleScsiOpcodeF1(void)
{
    u16 configuration_offset = (u16)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0C);
    u16 allocation_length = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B12);
    u16 response_length = MinU16(allocation_length, (u16)(0x00a0u - configuration_offset));

    ReadDriveConfigurationBytes((fw_addr_t)configuration_offset, FW_NEAR_UNKNOWN_0B2A, response_length);
    if(CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, response_length) != 4u)
        SetNoSenseData();
}

/* Reconstruction helpers; no direct Ghidra function address. */
static void SetP3Bit0AndClearP3Bit1ForReadPositionStatus(void)
{
#define PLEXTOR_BINDING_SECTION 76
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* Reconstruction helper; no direct Ghidra function address. */
static void SetP3Bit0AndSetP3Bit1ForState1Calibration(void)
{
#define PLEXTOR_BINDING_SECTION 77
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* Reconstruction helpers; no direct Ghidra function address. */
static u8 IsReadAcquisitionPmaMode(void)
{
    return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) != 0u);
}

static fw_addr_t ReadAcquisitionTocMsfTuple(u8 track_number)
{
    fw_addr_t address = FW_NEAR_UNKNOWN_F2B9;
    while(track_number != 0u)
    {
        address += 6u;
        track_number--;
    }
    return address;
}

static fw_addr_t ReadAcquisitionPmaMsfTuple(u8 track_number)
{
    fw_addr_t address = FW_HUGE_UNKNOWN_2BB042;
    while(track_number != 0u)
    {
        address += 10u;
        track_number--;
    }
    return address;
}

static fw_addr_t ReadAcquisitionPmaPreviousMsfTuple(u8 track_number)
{
    fw_addr_t address = FW_HUGE_UNKNOWN_2BB03E;
    while(track_number != 0u)
    {
        address += 10u;
        track_number--;
    }
    return address;
}

static void PublishReadAcquisitionTargetFromTrack(u8 track_number)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, track_number);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B,
        (u32)ConvertBinaryMsfToAdjustedFramePosition(IsReadAcquisitionPmaMode() != 0u ? ReadAcquisitionPmaMsfTuple(track_number) : ReadAcquisitionTocMsfTuple(track_number)));
}

static u8 IsReadAcquisitionPmaCursorAtOrBeyondPosition(s32 tolerance)
{
    u8 cursor = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);
    if(IsReadAcquisitionPmaMode() == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1148) != cursor)
        return 0u;
    return (u8)((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) >= ConvertBinaryMsfToAdjustedFramePosition(ReadAcquisitionPmaPreviousMsfTuple(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E42))) - tolerance);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u8 IsReadAcquisitionPmaFirstTrackAtOrBeyondPosition(s32 tolerance)
{
    u8 cursor = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);
    if(IsReadAcquisitionPmaMode() == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1147) != cursor)
        return 0u;
    return (u8)((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) >= ConvertBinaryMsfToAdjustedFramePosition(ReadAcquisitionPmaPreviousMsfTuple(cursor)) - tolerance);
}

/* RAM:f8858e */
/* The caller invokes this only while a controller packet is pending
 * (RAM:001203 positive); otherwise the native stack-local result byte would
 * be uninitialized. */
u8 AcquireMediaState6PacketNearTargetAndUpdatePhase(s32 target_frame_position)
{
    u8 phase_advanced;
    u8 packet_result;
    s32 current_frame_position;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
    ArmControllerThreeBytePacketCapture();
    for(;;)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
            break;
        do
        {
            packet_result = ParseMediaState6PacketForTargetWindow();
        } while(packet_result == 0xfeu);
        ArmControllerThreeBytePacketCapture();
        current_frame_position = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
        if(current_frame_position >= target_frame_position && current_frame_position <= target_frame_position + 4 && FUN_RAM_f896ee() == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEF) == 3u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) + 1u));
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) > 100u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
                    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, 0x96ul);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x1du);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
                    return 1u;
                }
                phase_advanced = 0u;
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEF) + 1u));
                phase_advanced = 1u;
            }
            break;
        }
        if(current_frame_position >= target_frame_position + 4 && current_frame_position < target_frame_position + 6)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEF) == 1u)
            {
                phase_advanced = 1u;
                break;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEF) - 1u));
            target_frame_position += 5;
            continue;
        }
        if(current_frame_position >= target_frame_position + 6)
        {
            phase_advanced = 0u;
            break;
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
    return phase_advanced != 0u ? 0u : 2u;
}

/* RAM:f895d1 */
u8 PLEXTOR_NEAR_FUNCTION AcquireAndClassifyMediaState6AtipPositionWindow(void)
{
    s32 current_frame_position;
    s32 window_bound;
    u8 offset;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1209, 40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
    ArmControllerThreeBytePacketCapture();

    for(;;)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) <= 0)
            break;

        if(ParseMediaState6PacketForTargetWindow() != 0xfeu)
        {
            ArmControllerThreeBytePacketCapture();

            window_bound = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FE9) - 0x46al;
            if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) >= window_bound && EvaluateMediaState6AtipWindowPulseResult() == 1u)
            {
                current_frame_position = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
                window_bound = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FE9) - 0x46al;
                if(current_frame_position >= window_bound)
                {
                    window_bound = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FE9) - 0x406l;
                    if(current_frame_position < window_bound)
                    {
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEE, (u8)(window_bound - current_frame_position));
                        break;
                    }
                }
            }

            window_bound = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FE9) - 0x406l;
            if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) >= window_bound)
            {
                window_bound = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FE9) - 0x3fcl;
                if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) < window_bound)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEE, 0u);
                    break;
                }
            }

            window_bound = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FE9) - 0x3fcl;
            if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) >= window_bound)
                return 2u;
        }

        YieldCoroutine1();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) <= 0)
        return 2u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEF, 3u);
    offset = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) + 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEE, offset);
    offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE);
    if(offset < 102u)
    {
        if(offset >= 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) - 1u));
        return 0u;
    }
    if(offset >= 105u)
        return 2u;

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, 0x96ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x1du);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    return 1u;
}

/* RAM:f887d4 */
/* Other signed status values enter the firmware's terminal self-loop at
 * RAM:f88902 and are intentionally outside this C-facing valid-status model. */
u8 PLEXTOR_NEAR_FUNCTION HandleDiscReadPositionAcquisitionStatus(s8 acquisition_status)
{
    u8 flags;

    if(acquisition_status == 0)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x52u);
                RestartCoroutine1AtF853cfAndYield();
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 1u);
            YieldCoroutine1();
            return 0u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0x7fu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        SetP3Bit0AndClearP3Bit1ForReadPositionStatus();
        return 1u;
    }
    if(acquisition_status == -1)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0x7fu));
        SetP3Bit0AndClearP3Bit1ForReadPositionStatus();
        return 1u;
    }
    if(acquisition_status != 1)
        for(;;)
        {
        }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
        return 1u;
    }
    flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
    {
        DisableControllerOperatingStateForReadPositionStatus();
        ServiceMediaQPacketCaptureForReadPositionStatus();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & ((flags & 0x30u) == 0u ? 0xffu : 0xbfu)) | ((flags & 0x30u) == 0u ? 0x40u : 0u)));
        return 1u;
    }
    if((flags & 0x30u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xbfu));
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(flags | 0x80u));
    }
    YieldCoroutine1();
    return 0u;
}

/* RAM:f87d40 */
/* Acquisition statuses other than zero, one, and ff enter the canonical
 * terminal self-loop at RAM:f87db6. */
u8 AcquireDiscReadPositionWithControllerReinitializationRetry(void)
{
    u8 attempt = 0u;

    while(attempt <= 2u)
    {
        u8 acquisition_status;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u)
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, 0x96ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        acquisition_status = DispatchAcquireDiscReadPositionForReinitializationRetry();

        if(acquisition_status == 1u || acquisition_status == 0xffu)
            return 1u;
        if(acquisition_status != 0u)
            for(;;)
            {
            }

        if(attempt > 1u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3C) & 1u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x17u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            DispatchYieldCoroutine1ForReinitializationRetry();
            return 0u;
        }

        DispatchResetDriveControlForReinitializationRetry();
        if(DispatchInitializeControllerForReinitializationRetry() == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 7u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            DispatchYieldCoroutine1ForReinitializationRetry();
            return 0u;
        }
        DispatchSetControllerEnableForReinitializationRetry(1u);
        ++attempt;
    }

    return 0u;
}

/* RAM:f88909 */
u8 PLEXTOR_NEAR_FUNCTION ServiceMediaState1ControllerCalibrationAttempt(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1007, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1007) - 1u));
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1007) < 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        SetP3Bit0AndClearP3Bit1ForReadPositionStatus();
        return 0u;
    }
    ResetDriveControlStateToEightForState1Calibration();
    if(InitializeControllerOperatingStateAndRunCalibrationForState1() != 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1007, 3u);
        return 1u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, 7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
    SetP3Bit0AndClearP3Bit1ForReadPositionStatus();
    return 0u;
}

/* RAM:f88984 */
/* DR0 holds the signed 32-bit comparison limit 0x00020f58; the JSL path
 * accepts only signed decoded positions greater than 0x00020f57. */
u8 PLEXTOR_NEAR_FUNCTION InitializeControllerCalibrationForState1AfterFrame3928(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u || (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) <= 0x00020f57l)
        return 0u;

    InitializeControllerCalibrationByteFromFeedback();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 0x40u));
    PersistControllerCalibrationStateBlockForState1();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
    SetP3Bit0AndSetP3Bit1ForState1Calibration();
    UpdateCalibrationOutcomeStateForState1(4u);
    UpdateCalibrationOutcomeStateForState1(5u);
    return 1u;
}

/* RAM:f889c9 */
u8 PLEXTOR_NEAR_FUNCTION CalibrateControllerPort8097ForState1AfterFrame45750(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u || (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) <= 0x0000b2b5l)
        return 0u;
    CalibrateControllerPort8097WithCommand08ForState1();
    return 1u;
}

/* RAM:f889e4 */
u8 PLEXTOR_NEAR_FUNCTION ServiceMediaState1ReadTargetWindow(void)
{
    s32 target = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B);
    s32 current = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD1) <= 0)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD2, 0u);

    if(current >= target)
    {
        if(current < target + 112l)
            return 1u;
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, (u32)(target + 75l));
        return 0u;
    }

    if(current + 37l < target)
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, (u32)(target + 75l));
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD1, 100u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD2, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD2) + 1u));
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD2) <= 3)
        return 1u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD2, 0u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, (u32)(target + 75l));
    return 0u;
}

/* RAM:f88a63 */
/* ACC selects a three- or 300-frame signed PMA tolerance; ACC returns the
 * action code.  Names describe only the observed software flag state. */
u8 PLEXTOR_NEAR_FUNCTION SelectReadAcquisitionTargetForDirectionBitClear(u8 use_wide_position_tolerance)
{
    s32 tolerance = use_wide_position_tolerance == 1u ? 300l : 3l;
    u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBE);
    u8 parsed_q = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 4u) != 0u);

    if(mode == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBF) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB))
        {
            PublishReadAcquisitionTargetFromTrack(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBF));
            return 1u;
        }
        return 0u;
    }
    if(mode == 2u)
    {
        if(parsed_q != 0u)
        {
            PublishReadAcquisitionTargetFromTrack(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF));
            return 1u;
        }
        if(IsReadAcquisitionPmaCursorAtOrBeyondPosition(tolerance) != 0u)
        {
            PublishReadAcquisitionTargetFromTrack(FirmwareReadNear8(FW_NEAR_UNKNOWN_1147));
            return 1u;
        }
        return 0u;
    }
    if(mode == 3u)
    {
        if(parsed_q != 0u || IsReadAcquisitionPmaCursorAtOrBeyondPosition(tolerance) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FC0));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FC0));
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FC2));
            return 2u;
        }
        return 0u;
    }
    if(mode == 4u)
    {
        if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) < (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FC6))
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FC0));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FC0));
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FC2));
        return 2u;
    }
    if(mode == 0u)
    {
        if(parsed_q != 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) != 0u)
            {
                WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, 150u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF1, 3u);
                return 2u;
            }
            if((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB) + 1u) == FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
                SetP3Bit0AndClearP3Bit1ForReadPositionStatus();
                return 1u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBB, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB) + 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBF, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB));
            PublishReadAcquisitionTargetFromTrack(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB));
            return 2u;
        }
        if(IsReadAcquisitionPmaCursorAtOrBeyondPosition(tolerance) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            SetP3Bit0AndClearP3Bit1ForReadPositionStatus();
            return 1u;
        }
        return 0u;
    }
    if(mode == 5u)
    {
        u8 advance = 0u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0x44u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 8u) == 0u
            || (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FC6))
            advance = 1u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x40u) != 0u && (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB) + 1u) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1233)
            && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0x10u) == 0u)
            advance = 1u;
        if(advance != 0u)
        {
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FC6) + 1u);
            CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_FAF2, FW_NEAR_UNKNOWN_1245, 10u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 2u) & 0xf2u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
            return 1u;
        }
    }
    return 0u;
}

/* RAM:f88e6f */
/* ACC selects a three- or 300-frame signed tolerance and returns action
 * zero, one, or two. The direction name only identifies this software path. */
u8 SelectReadAcquisitionTargetForDirectionBitSet(u8 use_wide_position_tolerance)
{
    s32 tolerance = use_wide_position_tolerance == 1u ? 300l : 3l;
    u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBE);
    u8 parsed_q = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 8u) != 0u);
    u8 cursor = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBB);

    if(mode == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBF) == cursor)
            return 0u;
        PublishReadAcquisitionTargetFromTrack(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBF));
        return 1u;
    }
    if(mode == 2u)
    {
        if(parsed_q != 0u)
        {
            PublishReadAcquisitionTargetFromTrack(cursor);
            return 1u;
        }
        if(IsReadAcquisitionPmaFirstTrackAtOrBeyondPosition(tolerance) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBF, cursor);
            PublishReadAcquisitionTargetFromTrack(cursor);
            return 1u;
        }
        return 0u;
    }
    if(mode == 3u || mode == 4u || mode == 5u)
    {
        if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FC2))
            return 0u;
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FC2));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        return 1u;
    }
    if(mode == 0u)
    {
        if(parsed_q != 0u || IsReadAcquisitionPmaFirstTrackAtOrBeyondPosition(tolerance) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBF, cursor);
            PublishReadAcquisitionTargetFromTrack(cursor);
            SetP3Bit0AndClearP3Bit1ForReadPositionStatus();
            return 2u;
        }
    }
    return 0u;
}

/* RAM:f890e8 */
void AdjustReadAcquisitionTargetForModes(void)
{
    if(ReadBit0013ForReadTargetAdjustment() == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u
        && (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B) >= 75l)
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B) - 30u);

    if(ReadBit0013ForReadTargetAdjustment() != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        {
            if(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B) == 0x10000000ul)
                WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0F52) - 4500u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 1u));
        }
    }
    if(ReadBit001cForReadTargetAdjustment() != 0u || ReadBit001dForReadTargetAdjustment() != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 0x40u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
            SetControllerOperatingEnableForReadTargetAdjustment();
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F) - 999u);
        if(ReadBit001cForReadTargetAdjustment() != 0u)
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_114F) - 9u);
    }
    if(ReadBit001bForReadTargetAdjustment() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        SetControllerOperatingEnableForReadTargetAdjustment();
}

/* RAM:fed307 */
void HandleRawReadHostTransferCompletion(void)
{
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        ResetRawReadControllerTransferAndResume();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1462, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128A, 1u);

    if(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1455) == ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1459) && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) == 0u
        && ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) > 10u)
    {
        SetNoSenseData();
        PrepareAtaCompletionStatusFromSense();
        PublishAtaCommandCompletion();
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x02u));
    }
    UpdateRawReadDescriptorFlagsFromControllerStatus31();
}

/* RAM:fd66c7 */
void ReadDriveConfigurationBytes(fw_addr_t offset, fw_addr_t destination, u16 count)
{
    u16 index;

    for(index = 0u; index < count; ++index)
    {
        u16 source_offset = (u16)(offset + index);
        fw_addr_t source =
            FirmwareAddressIncrement24(source_offset < 0x60u ? PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFC000) : PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFA000), source_offset);

        FirmwareWrite8(FirmwareAddressIncrementLow16(destination, index), FirmwareRead8(source));
    }
}

/* RAM:f84cb1 */
void LoadMediaCalibrationOffsetsAndControllerParameterFc(void)
{
    u8 calibration_offset;

    calibration_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_FABD);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FADD, calibration_offset == 0xffu ? 0u : calibration_offset);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FADC, FirmwareReadNear8(FW_NEAR_UNKNOWN_FADD));

    calibration_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_FABE);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FADE, calibration_offset == 0xffu ? 0u : calibration_offset);

    calibration_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_FABF);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FADF, calibration_offset == 0xffu ? 0u : calibration_offset);

    ReadDriveConfigurationBytes(0x002cul, FW_NEAR_UNKNOWN_FAC0, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, 0xa0u);
    calibration_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC0);
    if(calibration_offset != 0xffu)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, (u8)(calibration_offset + 0xd8u));
}

/* RAM:fda699 */
void PLEXTOR_NEAR_FUNCTION RefreshConfigurationByte45Flag(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_131A, (u8)(FirmwareRead8(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFC045)) == 1u));
}

/* RAM:fd6664 */
u16 ReadDriveConfigurationWord(u16 word_index)
{
    u16 byte_offset = (u16)(word_index << 1);
    fw_addr_t window = byte_offset < 0x60u ? PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFC000) : PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFA000);
    fw_addr_t first = FirmwareAddressIncrement24(window, byte_offset);

    return (u16)(((u16)FirmwareRead8(first) << 8) | FirmwareRead8(FirmwareAddressIncrement24(first, 1u)));
}

/* RAM:fd672e */
u16 CalculatePersistentPrimaryConfigurationChecksum(void)
{
    u16 checksum = 0u;
    u8 offset = 0u;

    do
    {
        checksum = (u16)(checksum + FirmwareRead8((fw_addr_t)(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFC000) + offset)));
        offset++;
    } while(offset < 0x5eu);
    return checksum;
}

/* RAM:fd6712 */
u8 ValidatePrimaryDriveConfigurationChecksum(void)
{
    return (u8)(ReadDriveConfigurationWord(0x2fu) == CalculatePersistentPrimaryConfigurationChecksum());
}

/* RAM:fd607f */
void RecalculateDriveConfigurationChecksums(void)
{
    u16 index;

    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_12FC, 0u);
    for(index = 0u; index < 0x5eu; ++index)
    {
        u16 checksum = (u16)(FirmwareReadNear8((u16)(0x1620u + index)) + ReadBigEndianU16Near(FW_NEAR_UNKNOWN_12FC));
        WriteBigEndianU16Near(FW_NEAR_UNKNOWN_12FC, checksum);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_167E, FirmwareReadNear8(FW_NEAR_UNKNOWN_12FC));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_167F, FirmwareReadNear8(FW_NEAR_UNKNOWN_12FD));

    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_12FE, 0u);
    for(index = 0u; index < 0x7eu; ++index)
    {
        u16 checksum = (u16)(FirmwareReadNear8((u16)(0x1620u + index)) + ReadBigEndianU16Near(FW_NEAR_UNKNOWN_12FE));
        WriteBigEndianU16Near(FW_NEAR_UNKNOWN_12FE, checksum);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_169E, FirmwareReadNear8(FW_NEAR_UNKNOWN_12FE));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_169F, FirmwareReadNear8(FW_NEAR_UNKNOWN_12FF));
}

/* RAM:fd6891 */
void CopyMonitorConfigurationPageToAddress(fw_addr_t destination_address)
{
    u16 index;

    for(index = 0u; index < 0x100u; ++index)
        FirmwareWrite8(FirmwareAddressIncrementLow16(destination_address, index), FirmwareRead8(FirmwareAddressIncrementLow16(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFFF00), index)));
}

/* Reconstruction helper; no direct Ghidra function address.
 * C-facing form of RAM:feea1c.  The canonical DR4/ACC A251 boundary remains
 * available for original-ABI callers; this form retains its interrupt window
 * and reads the byte after the source +0x10 controller-buffer bias. */
static u8 ReadControllerBufferByteAtomicFromC(fw_addr_t address)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 value;

#define PLEXTOR_BINDING_SECTION 78
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    value = FirmwareRead8(FirmwareAddressIncrement24(address, 0x10u));
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
#define PLEXTOR_BINDING_SECTION 79
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    return value;
}

/* Reconstruction helper; no direct Ghidra function address. */
static void ClearBit0547ForConfigurationProgramming(void)
{
#define PLEXTOR_BINDING_SECTION 80
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* Reconstruction helper; no direct Ghidra function address. */
static void SetConfigurationProgramP3StateBeforeMonitorCall(void)
{
#define PLEXTOR_BINDING_SECTION 81
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* Reconstruction helper; no direct Ghidra function address. */
static void SetConfigurationProgramP3StateAfterMonitorCall(void)
{
#define PLEXTOR_BINDING_SECTION 82
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* Reconstruction helper; no direct Ghidra function address. */
static u8 VerifyDriveConfigurationRange(fw_addr_t persistent_address, u16 buffer_address, u16 count)
{
    u16 index;

    for(index = 0u; index < count; ++index)
        if(FirmwareRead8(FirmwareAddressIncrement24(persistent_address, index)) != FirmwareReadNear8((u16)(buffer_address + index)))
            return 0u;
    return 1u;
}

/* Reconstruction helper; no direct Ghidra function address. */
static void ResetRawReadQueueAfterConfigurationProgram(void)
{
    u32 slot_count = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_156A);
    u32 auxiliary_c2_ring_base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1562);

    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1298, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1294, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1296, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1292, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_129A, (u16)(slot_count - 1ul));
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_129C, (u16)(slot_count - 6ul));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_126A, auxiliary_c2_ring_base);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_126E, 0ul);
}

/* RAM:fd643b */
u8 ProgramAndVerifyDriveConfiguration(u8 extended_only)
{
    u8 saved_controller_status;
    u8 use_p3_handoff;
    u8 result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) | 0x04u));
    saved_controller_status = FirmwareReadController8(FW_CONTROLLER_COMMAND_TRIGGER);
    use_p3_handoff = (u8)ReadDriveConfigurationWord(0x000fu);
    CopyMonitorConfigurationPageToAddress(0x00001700ul);
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    if(use_p3_handoff != 0u)
        SetConfigurationProgramP3StateBeforeMonitorCall();

    result = InvokeRecoveryConfigurationProgram(extended_only);
    if(result == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, saved_controller_status);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfbu));
        return 0u;
    }

    if(use_p3_handoff != 0u)
        SetConfigurationProgramP3StateAfterMonitorCall();
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, saved_controller_status);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfbu));

    if(extended_only == 0u && VerifyDriveConfigurationRange(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C000), 0x1620u, 0x60u) == 0u)
        return 0u;
    if(VerifyDriveConfigurationRange(FW_ROM_UNKNOWN_F7A060, 0x1680u, 0x40u) == 0u)
        return 0u;

    ResetRawReadQueueAfterConfigurationProgram();
    return 1u;
}

/* RAM:fd60e4 */
/* Internal wrapper only: hardware programming remains behind the existing
 * monitor interface; the host tests invoke a simulated monitor callback. */
void PersistDriveConfigurationValidationFailureCode(u8 failure_code)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1692, failure_code);
    RecalculateDriveConfigurationChecksums();
    (void)ProgramAndVerifyDriveConfiguration(0u);
}

/* RAM:fd67ba */
void UpdateDriveConfigurationByteRangeAndProgram(fw_addr_t source_address, u16 destination_offset, u16 count)
{
    u8 restart_required;
    u16 index;
    u16 checksum = 0u;

    ClearBit0547ForConfigurationProgramming();
    ReadDriveConfigurationBytes(0u, 0x00001620ul, 0xa0u);
    restart_required = FirmwareReadNear8(FW_NEAR_UNKNOWN_163F);
    for(index = 0u; index < count; ++index)
        FirmwareWriteNear8((u16)(0x1620u + destination_offset + index), FirmwareRead8(FirmwareAddressIncrement24(source_address, index)));
    for(index = 0u; index < 0x7eu; ++index)
        checksum = (u16)(checksum + FirmwareReadNear8((u16)(0x1620u + index)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_169E, (u8)(checksum >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_169F, (u8)checksum);

    if(restart_required == 1u && ProgramAndVerifyDriveConfiguration(1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12F7, 2u);
        InitializeCoroutine1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
    }
    else if(restart_required != 1u)
    {
        (void)ProgramAndVerifyDriveConfiguration(1u);
    }
}

/* RAM:f8f0ce */
void PersistControllerOperatingStatusByte(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) != 4u)
            UpdateDriveConfigurationByteRangeAndProgram(0x0000fae7ul, 0x77u, 1u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) != 4u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) != 0x39u)
    {
        UpdateDriveConfigurationByteRangeAndProgram(0x0000fae7ul, 0x72u, 1u);
    }
}

#define PLEXTOR_BINDING_SECTION 83
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f8ea76 */
void SIGNAL_DRIVE_STATUS_FUNCTION(void)
{
    u8 first_pattern_both_low = 0u;
    u8 second_pattern_rxd_low = 0u;
    u8 second_pattern_both_low = 0u;
    u8 pulse_count;

    SIGNAL_DRIVE_STATUS_PERSIST();
    pulse_count = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) & 0x0fu);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x2au)
    {
        second_pattern_rxd_low = 1u;
        pulse_count = 1u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x3au)
    {
        second_pattern_rxd_low = 1u;
        pulse_count = 1u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x3bu)
    {
        second_pattern_rxd_low = 1u;
        second_pattern_both_low = 1u;
        pulse_count = 2u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x3du)
    {
        second_pattern_rxd_low = 1u;
        second_pattern_both_low = 1u;
        pulse_count = 3u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x3cu)
    {
        first_pattern_both_low = 1u;
        pulse_count = 1u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x3eu)
    {
        first_pattern_both_low = 1u;
        pulse_count = 2u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x27u)
        second_pattern_rxd_low = 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x39u)
        second_pattern_rxd_low = 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x25u)
        second_pattern_rxd_low = 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 60u);
    SIGNAL_DRIVE_STATUS_OUTPUT(1u, 1u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121A) > 0)
    {
    }

    for(;;)
    {
        u8 pulse_index;

        for(pulse_index = 0u; pulse_index < pulse_count; ++pulse_index)
        {
            if(first_pattern_both_low != 0u)
                SIGNAL_DRIVE_STATUS_OUTPUT(0u, 0u);
            else
                SIGNAL_DRIVE_STATUS_OUTPUT(1u, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 60u);
            while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121A) > 0)
                YieldCoroutine1();

            if(second_pattern_rxd_low != 0u)
                if(second_pattern_both_low != 0u)
                    SIGNAL_DRIVE_STATUS_OUTPUT(0u, 0u);
                else
                    SIGNAL_DRIVE_STATUS_OUTPUT(0u, 1u);
            else
                SIGNAL_DRIVE_STATUS_OUTPUT(1u, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 30u);
            while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121A) > 0)
                YieldCoroutine1();
        }

        SIGNAL_DRIVE_STATUS_OUTPUT(1u, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 60u);
        while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121A) > 0)
            YieldCoroutine1();
    }
}

/* RAM:f8f08b */
void FUN_RAM_f8f08b(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) | 0x20u));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD7) & 0x08u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) + 2u));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) | 0x30u));
    }
    PersistControllerOperatingStatusByte();
}

/* RAM:f93a5e */
void PLEXTOR_NEAR_FUNCTION FUN_RAM_f93a5e(void)
{
    u8 index;
    u8 aggregate = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F25) != 1u)
        return;

    for(index = 0u; index != 4u; ++index)
    {
        fw_addr_t main_data_ring_cursor = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_126E);

        aggregate = (u8)(aggregate | ReadControllerBufferByteAtomicFromC(FirmwareAddressIncrement24(main_data_ring_cursor, (u16)(index + 0x810u))));
    }
    if(aggregate == 0u)
    {
        u16 limit = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 3u);
        u16 countdown = (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24);

        if(countdown <= limit)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, 1u);
    }
}

/* RAM:f8873d */
void FUN_RAM_f8873d(void)
{
    u8 saved_status;

    if(IsIndexedDriveStateActive(0x0au) != 1u)
        return;
    saved_status = FirmwareReadNear8(FW_NEAR_UNKNOWN_1009);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, (u8)DivideU32QuotientWithoutRuntimeLibrary(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_11F0), 0x00000e10ul));
    UpdateP3Bits0And1PatternState(1u);
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u)
    {
        UpdateP3Bits0And1PatternState(0u);
        YieldCoroutine1();
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, saved_status);
}

/* RAM:f8a765 */
void PLEXTOR_NEAR_FUNCTION UpdateMediaState5LeadOutProgressStage(void)
{
    u32 lead_out = ReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION);
    u32 staged_base = ((u32)(u16)((lead_out >> 16) - 1u) << 16) | (lead_out & 0xffffu);
    s32 first_threshold = (s32)(staged_base - 0x368cu);
    s32 current = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
    u8 stage;

    if(current < first_threshold)
        return;
    stage = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAFE);
    if(stage == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFE, 1u);
        SetMediaState5LeadOutTransitionBitsAtomic();
        return;
    }
    if(current < (s32)(first_threshold + 0x09c4l))
        return;
    if(stage == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFE, 2u);
        SetMediaState5LeadOutTransitionBitsAtomic();
        return;
    }
    if(current < (s32)(first_threshold + 0x1388l) || stage != 2u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFE, 3u);
    SetMediaState5LeadOutTransitionBitsAtomic();
}

/* RAM:f8bc59 */
void PLEXTOR_NEAR_FUNCTION SelectSecondaryOperatingCodeForThresholdValue(s32 threshold_value)
{
    u8 previous = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
    u8 media_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF);
    u8 selected = previous;
    u8 current_code;

    if(media_state == 6u)
    {
        u8 flags;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 0x14u)
            return;
        flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF5);
        if((flags & 0x10u) == 0u)
            selected = 0x10u;
        else if((flags & 0x08u) != 0u)
            selected = threshold_value > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9F6) ? 0x28u : 0xffu;
        else if((flags & 0x04u) != 0u)
            selected = threshold_value > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9F2) ? 0x20u : 0xffu;
        else if((flags & 0x02u) != 0u)
            selected = threshold_value > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9EE) ? 0x18u : 0xffu;
        else if((flags & 0x01u) != 0u)
            selected = threshold_value > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9EA) ? 0x14u : 0xffu;
        else
            selected = 0x10u;
    }
    else
    {
        if(media_state != 1u && media_state != 2u && media_state != 3u && media_state != 5u)
            return;
        current_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
        if(current_code == 0x14u)
            selected = threshold_value > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9EA) ? 0x14u : 0xffu;
        else if(current_code == 0x18u)
            selected = threshold_value > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9EE) ? 0x18u : 0xffu;
        else if(current_code == 0x20u)
            selected = threshold_value > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9F2) ? 0x20u : 0xffu;
        else if(current_code == 0x28u)
            selected = threshold_value > (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F9F6) ? 0x28u : 0xffu;
        else if(current_code == 0x34u)
            selected = 0xffu;
        else
            return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, selected);
    if(previous != FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x08u));
}

/* RAM:f87aa0 */
void PLEXTOR_NEAR_FUNCTION UpdateSecondaryOperatingCodeForSupportedCurrentCode(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

    if(operating_code < 0x14u)
        return;
    if(operating_code == 0x14u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) != 0xffu)
        return;
    if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018011) & 0x02u) == 0u)
        return;

    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    if(operating_code == 0x14u || operating_code == 0x18u || operating_code == 0x20u || operating_code == 0x28u || operating_code == 0x34u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, operating_code);
}

/* RAM:f87a7c */
/* The trigger test and clear are separate volatile reads in the native body. */
void PLEXTOR_NEAR_FUNCTION ServiceSecondaryOperatingCodeUpdateTrigger(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 0x08u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 0xf7u));
        SetMediaState5LeadOutTransitionBitsAtomic();
    }
    UpdateSecondaryOperatingCodeForSupportedCurrentCode();
}

/* RAM:f8e1e6 */
/* The value at RAM:00fab8 is reread for the mirror store after the controller call. */
void ApplyWord0204AndConfigureCalibrationForCurrentState(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x20u) == 0u)
    {
        SubmitControllerWordPair(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAB8), 0x0204u);
        WriteBigEndianU16Near(FW_NEAR_UNKNOWN_F990, ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAB8));
    }
    (void)LoadParameterFCAndCalibrateWord0319ForState(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008));
}

/* RAM:fb1d93 */
/* The value at RAM:001215 is asynchronously decremented by the calibration owner. */
u8 PLEXTOR_NEAR_FUNCTION ValidateControllerState5AndCalibrateWord0319(void)
{
    u8 controller_enabled = FirmwareReadNear8(FW_NEAR_UNKNOWN_1006);

    if(controller_enabled != 0u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0x80u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x01u) != 0x01u)
        return 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, controller_enabled != 0u ? 10u : 5u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) > 0)
    {
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 5u)
    {
        if(CalibrateControllerWord0319Offset() != 0u)
            return 1u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x04u));
        return 0u;
    }

    if(controller_enabled != 0u)
        DispatchConfigureControllerPort8006AndChannel3ForState5Calibration(1u);
    return 1u;
}

/* RAM:f8e20f */
u8 LoadParameterFCAndCalibrateWord0319ForState(u8 parameter_state)
{
    u16 captured_word0319 = ReadControllerWord(0x0319u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        if(CalibrateControllerWord0319Offset() == 0u)
            return 0u;
        SelectAndWriteControllerParameterFCForOperatingCode(1u);
        return 1u;
    }

    if(parameter_state == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC0));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC0) == 0xffu)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC1));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC1) == 0xffu)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, 0u);
    }

    SelectAndWriteControllerParameterFCForOperatingCode(1u);
    if(CalibrateControllerWord0319Offset() == 0u)
    {
        if(parameter_state == 0u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x10u) == 0u)
                SubmitControllerWordPair((u16)(captured_word0319 + ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FABA)), 0x0319u);
        }
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x02u) == 0u)
        {
            SubmitControllerWordPair((u16)(captured_word0319 + ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FABA)), 0x0319u);
        }
    }
    return 1u;
}

/* RAM:f8e182 */
void PLEXTOR_NEAR_FUNCTION ApplyControllerWord0204CalibrationAndSettle(void)
{
    if(IsIndexedDriveStateActive(8u) != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
    {
        SetControllerOperatingEnableState(1u);
        SelectAndConfigureOperatingCodeForThreshold(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235));
    }

    ApplyCurrentOperatingCodeToControllerChannels0Through3();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 2u);
    WriteControllerParameterWord(0xf025u);
    WriteControllerParametersD412AndE2Value58();
    WriteControllerParameterF4WithDownAdjustment();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 100u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) > 0)
        YieldCoroutine1();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 100u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) > 0)
        YieldCoroutine1();
}

/* RAM:f8a9a1 */
/* The three consecutive 0x80 writes are instruction-visible MMIO strobes. */
void InitializeDefaultControllerOperatingProfile(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180F4, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 0u);
    LoadControllerOperatingProfile(0u);
    ResetControllerOperatingState();
}

/* RAM:f8e029 */
u16 PLEXTOR_NEAR_FUNCTION RunControllerCommand0ETransaction(void)
{
    IssueControllerCommand0E();
    return CompleteControllerCommand0EAndReadPorts80C7C6();
}

/* RAM:f8e068 */
u16 PLEXTOR_NEAR_FUNCTION CompleteControllerCommand0EAndReadPorts80C7C6(void)
{
    u16 result = 0u;

    if(WaitControllerPort80F8Bit7Clear500AndYield() == 0u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        WaitControllerPort80F8Bit7ClearWithoutTimeout();
    }
    else
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        result = (u16)((u16)WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180C7) << 8);
        result |= WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180C6);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
    }
    return result;
}

/* RAM:f8e0d9 */
void PLEXTOR_NEAR_FUNCTION ConfigureControllerOperatingStateAndCalibratePort8097(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120B, 3u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120B) > 0)
        YieldCoroutine1();
    WaitForControllerMeasurementWithinToleranceOrExpiryFalseFalse();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_120B, 2u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120B) > 0)
        YieldCoroutine1();
    if(IsIndexedDriveStateActive(8u) != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && TestP1Cex0PulseSuccessThreshold(10u) == 1u)
        SetControllerOperatingEnableState(0u);

    ApplyCurrentOperatingCodeToControllerChannels0Through3();
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1200, 20u);
    while((s16)ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1200) > 0)
        YieldCoroutine1();
    CalibrateControllerPort8097WithCommand08();
    WriteControllerParameterWord(0xf028u);
    WriteControllerParameterWord((u16)(((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC14) - 0x24u) | 0xf400u));
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1200, 20u);
    while((s16)ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1200) > 0)
        YieldCoroutine1();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 2u);
}

/* RAM:fcf35d */
void PLEXTOR_NEAR_FUNCTION AcknowledgeControllerQPacketCapture(void)
{
    SetExternalInterrupt1EnabledAtomic(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0x7fu));
    FirmwareWriteDirectSfr8(91, (u8)(FirmwareReadDirectSfr8(91) & 0xfdu));
    SetExternalInterrupt1EnabledAtomic(1u);
}

/* RAM:fcf345 */
void RearmControllerQPacketCaptureWithTimeout(void)
{
    SetExternalInterrupt1EnabledAtomic(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1205, 0x1eu);
    FirmwareWriteDirectSfr8(91, (u8)(FirmwareReadDirectSfr8(91) & 0xfdu));
    SetExternalInterrupt1EnabledAtomic(1u);
}

/* RAM:f89295 */
void ServiceConditionalControllerOperatingDisable(void)
{
#define PLEXTOR_BINDING_SECTION 84
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) == 0xffu || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u))
    {
        SetControllerOperatingEnableState(0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, 0x28u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1215, 0x14u);
        while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1215) > 0)
        {
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xbfu));
}

/* RAM:fbe5ac */
u8 PersistVendorF3ControllerWordIfPending(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F1BA) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_F1BA) == 1u && UpdateAndProgramDriveConfigurationRangeF087Offset34Count2() == 0u)
    {
        SetSenseData(4u, 0x4000u);
        return 0u;
    }
    SetNoSenseData();
    return 1u;
}

/* RAM:fb6196 */
void PrepareControllerEventBit4OperatingUpdate(void)
{
    u8 value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xefu);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, value);
    value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) & 0x1fu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, value);
    value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) & 0xefu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, value);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, value);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_154D) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9F, 2u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9F, 0u);

    if(FirmwareReadBit0032() == 0u)
    {
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, value);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_8002, value);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C0, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180CF, FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) == 2u ? 4u : 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180C3, 0x60u);
        value = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, value);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_8002, value);
    }
}

/* RAM:f8edcf */
/* The native routine exits through ERET; this is its value-free state model. */
void AdvanceMediaReadConfigurationStateMachine(void)
{
    switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_F977))
    {
    case 1u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F977, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        break;
    case 2u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F977, 1u);
        WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC) & 0xfffbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E0, 0x3cu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E1, 0xffu);
        break;
    case 3u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F977, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E0, 0x3cu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E1, 0xffu);
        break;
    case 0u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F977, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E0, 0x78u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F9E1, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfbu));
        break;
    default:
        break;
    }
}

/* RAM:f8bde2 */
/* The native routine exits through ERET; this C form only models its internal
 * state selection and must not substitute for that coroutine-return boundary. */
void SelectRequestedOperatingCodeForMediaState(void)
{
    u8 selection;
    u8 requested;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) & 0xfeu));
    selection = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44);
    if(selection == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 0u);
    }
    else if(selection == 4u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 4u);
    }
    else if(selection == 5u)
    {
        switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46))
        {
        case 1u:
            requested = 0u;
            break;
        case 4u:
            requested = 4u;
            break;
        case 8u:
            requested = 8u;
            break;
        case 0x18u:
            requested = 0x18u;
            break;
        case 0x20u:
            requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x20u : 0x18u;
            break;
        case 0x28u:
            requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x28u : 0x18u;
            break;
        case 0x34u:
            requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x34u : 0x18u;
            break;
        default:
            requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x28u : 0x18u;
            break;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, requested);
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u && FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) != 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) <= 10u ? 10u : 0x18u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B) >= 8u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 8u);
    }
    else if(selection == 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 8u);
    }
    else if(selection == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4A) == 1u)
        {
            switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46))
            {
            case 4u:
                requested = 4u;
                break;
            case 8u:
                requested = 8u;
                break;
            case 0x18u:
                requested = 0x18u;
                break;
            case 0x20u:
                requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x20u : 0x18u;
                break;
            case 0x28u:
                requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x28u : 0x18u;
                break;
            case 0x34u:
                requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x34u : 0x18u;
                break;
            case 1u:
                requested = 0u;
                break;
            default:
                requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x28u : 0x18u;
                break;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, requested);
            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u && FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) != 1u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) <= 10u ? 10u : 0x18u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B) >= 8u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 8u);
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 3u
                && ((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB8) < 0 || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x08u) == 0u)))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB8, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0xf7u));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) > 3u)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 4u);
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) > 0x17u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FB9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FB9) | 1u));
            }
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, FirmwareReadBit000E() != 0u ? 4u : 1u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B) >= 8u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B) == 1u)
                && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) > 8u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 8u);
        }
    }
}

/* RAM:f8b7cc */
/* The original exits through ERET.  This C value form models only its
 * instruction-proven ACC Boolean result; an eventual reachable target caller
 * must retain the native coroutine return boundary separately. */
u8 ExecuteReadPositionTimingHandshake(void)
{
    u16 duration;
    u16 reload;

    if(READ_POSITION_P1_BIT3())
        return 1u;

    duration = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u ? 0x03deu : 0x1356u;
    reload = (u16)(0xffffu - duration);
    WRITE_READ_POSITION_TH1((u8)(reload >> 8));
    WRITE_READ_POSITION_TL1((u8)reload);
    CLEAR_READ_POSITION_TIMER_OVERFLOW();
    START_READ_POSITION_TIMER();

    for(;;)
    {
        if(READ_POSITION_TIMER_OVERFLOW())
        {
            STOP_READ_POSITION_TIMER();
            CLEAR_READ_POSITION_TIMER_OVERFLOW();
            if(FirmwareReadBit0407() != 0u)
                return 1u;

            WRITE_READ_POSITION_TH1((u8)(reload >> 8));
            WRITE_READ_POSITION_TL1((u8)reload);
            CLEAR_READ_POSITION_TIMER_OVERFLOW();
            START_READ_POSITION_TIMER();
            for(;;)
            {
                if(READ_POSITION_TIMER_OVERFLOW())
                {
                    STOP_READ_POSITION_TIMER();
                    CLEAR_READ_POSITION_TIMER_OVERFLOW();
                    return 0u;
                }
                if(FirmwareReadBit0407() != 0u)
                {
                    STOP_READ_POSITION_TIMER();
                    CLEAR_READ_POSITION_TIMER_OVERFLOW();
                    return 1u;
                }
            }
        }
        if(READ_POSITION_P1_BIT3())
        {
            STOP_READ_POSITION_TIMER();
            CLEAR_READ_POSITION_TIMER_OVERFLOW();
            return 1u;
        }
    }
}

/* RAM:f8b83f */
u8 CheckP1Cex0PulseWithinTimer1Window(void)
{
    u8 index;

    WRITE_READ_POSITION_TH1(0xffu);
    WRITE_READ_POSITION_TL1(0xb0u);
    CLEAR_READ_POSITION_TIMER_OVERFLOW();
    START_READ_POSITION_TIMER();
    if(READ_POSITION_P1_BIT3())
    {
        while(!READ_POSITION_TIMER_OVERFLOW())
            YieldCoroutine1();
        STOP_READ_POSITION_TIMER();
        CLEAR_READ_POSITION_TIMER_OVERFLOW();
        return 1u;
    }
    do
    {
        if(READ_POSITION_TIMER_OVERFLOW())
        {
            STOP_READ_POSITION_TIMER();
            CLEAR_READ_POSITION_TIMER_OVERFLOW();
            return 0u;
        }
        YieldCoroutine1();
    } while(!READ_POSITION_P1_BIT3());
    for(index = 0u; index != 0x13u; ++index)
    {
        if(!READ_POSITION_P1_BIT3())
        {
            STOP_READ_POSITION_TIMER();
            CLEAR_READ_POSITION_TIMER_OVERFLOW();
            return 0u;
        }
    }
    STOP_READ_POSITION_TIMER();
    CLEAR_READ_POSITION_TIMER_OVERFLOW();
    return 1u;
}

/* RAM:f8b88c */
u8 TestP1Cex0PulseSuccessThreshold(u8 sample_count)
{
    u8 index = 0u;
    u8 success_count = 0u;

    while(index < sample_count)
    {
        if(CheckP1Cex0PulseWithinTimer1Window() == 1u)
            success_count++;
        index++;
    }
    return success_count >= (u8)(sample_count / 3u) ? 1u : 0u;
}

/* RAM:fc8a3e */
void ApplyControllerD1ModeBitsAfterPulseTest(void)
{
    if(TestP1Cex0PulseSuccessThreshold(10u) == 1u)
        ApplyControllerParameterD1ModeBits();
}

/* RAM:fc8af5 */
void ResetReadAcquisitionRecoveryState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1207, 0x4bu);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1207, 0x37u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xfbu));
    RampControllerWord0255TowardFeedbackValue();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE2, 0u);
}

/* RAM:fddb71 */
void ResetAndArmControllerMeasurementCaptureWindow(void)
{
    u32 sample_index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 2u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1520, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_151F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_151B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_151C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_151D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_151E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1517, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1518, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1519, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_151A, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018014, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018015, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018016, 0x0cu);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018017, 0xc0u);

    for(sample_index = 0ul; sample_index < 0x7ffful; ++sample_index)
    {
        while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018015) & 0x40u) == 0u)
        {
        }
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018030, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018031, 0u);
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018017, 0u);
}

/* RAM:fa3ee0 */
u8 AppendFormatUnitCode11ScanTargetPositionsFromState(u8 requested_count, u8 processed_count, u8 output_index)
{
    while(processed_count < requested_count)
    {
        u32 position = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_15DC) + 1ul;
        u16 output_address = (u16)(0xfa2bu + (u16)((u16)output_index * 4u));

        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_15DC, position);
        WriteBigEndianU32Near(output_address, position);
        output_index = (u8)(output_index + 1u);
        WriteBigEndianU32Far(FW_HUGE_UNKNOWN_2BC6FB, ReadBigEndianU32Far(FW_HUGE_UNKNOWN_2BC6FB) + 1ul);
        processed_count = (u8)(processed_count + 1u);
    }
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B06, 1ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    return output_index;
}

/* RAM:fa3ea3 */
/* The RAM:fa3ee0 output cursor inherited through R7 is explicit here. */
u8 AppendFormatUnitCode11ScanTargetPositions(u8 requested_count, u8 initial_output_index)
{
    return AppendFormatUnitCode11ScanTargetPositionsFromState(requested_count, 0u, initial_output_index);
}

/* RAM:fedc36 */
void InitializeWriteModePage05AndTrackStateDefaults(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C44, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C46, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) | 0x02u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C47, 7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C48, 0x0au);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C4A, 0x20u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B, 0x20ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C71, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C72, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C73, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C74, 0u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD57, 0x20ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0CC1, 0x20ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CC0, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD9C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD9C) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CDF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0CDF) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E641, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E642, 0x27u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E643, 0x0au);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E647, 0x20u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E649, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E64A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E64B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E64C, 0x20u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E66F, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E670, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E671, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E672, 0u);
}

/* RAM:fd2c93 */
u8 ValidateStagedFirmwareChecksum(u32 end_offset, u32 start_offset)
{
    fw_addr_t source_address = start_offset;
    fw_addr_t checksum_address;
    u32 position;
    u16 checksum = 0u;

    if(end_offset == 0x00079ffeul)
    {
        source_address = FW_ADDRESS_BANK_STRIDE;
        for(position = start_offset; position < 0x00070000ul; ++position)
        {
            checksum = (u16)(checksum + FirmwareRead8(source_address));
            source_address = FirmwareAddressIncrement24(source_address, 1u);
        }
        source_address = FW_HUGE_UNKNOWN_178000;
        for(position = 0x00078000ul; position < end_offset; ++position)
        {
            checksum = (u16)(checksum + FirmwareRead8(source_address));
            source_address = FirmwareAddressIncrement24(source_address, 1u);
        }
    }
    if(end_offset == 0x0000fffeul)
    {
        source_address = FW_ADDRESS_BANK_STRIDE;
        for(position = start_offset; position < 0x00008000ul; ++position)
        {
            checksum = (u16)(checksum + FirmwareRead8(source_address));
            source_address = FirmwareAddressIncrement24(source_address, 1u);
        }
        source_address = FW_HUGE_UNKNOWN_10FF00;
        for(position = 0x0000ff00ul; position < end_offset; ++position)
        {
            checksum = (u16)(checksum + FirmwareRead8(source_address));
            source_address = FirmwareAddressIncrement24(source_address, 1u);
        }
    }
    checksum_address = source_address;
    return (u8)(checksum == (u16)(((u16)FirmwareRead8(checksum_address) << 8) | FirmwareRead8(FirmwareAddressIncrement24(checksum_address, 1u))));
}

/* RAM:fafcd3 */
void UpdateOpcCalibrationSpeedClass(u8 write_speed_x)
{
    u8 speed_class;
    u8 invalidate = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(write_speed_x >= 0x10u)
            speed_class = 0x10u;
        else if(write_speed_x >= 8u)
            speed_class = 8u;
        else if(write_speed_x >= 4u)
            speed_class = 4u;
        else
        {
            speed_class = 0u;
            invalidate = 1u;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
    {
        if(write_speed_x >= 0x18u)
            speed_class = 0x10u;
        else
        {
            speed_class = 0u;
            invalidate = 1u;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
    {
        if(write_speed_x >= 10u)
            speed_class = 10u;
        else if(write_speed_x >= 4u)
            speed_class = 4u;
        else
        {
            speed_class = 0u;
            invalidate = 1u;
        }
    }
    else if(write_speed_x >= 4u)
    {
        speed_class = 4u;
    }
    else
    {
        speed_class = 0u;
        invalidate = 1u;
    }

    if(invalidate != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74D, speed_class);
}

/* RAM:fce4e6 */
void PLEXTOR_NEAR_FUNCTION UpdateP1Cex0PulseCalibrationStatus(void)
{
    u8 result;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        return;
    result = TestP1Cex0PulseSuccessThreshold(10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xdfu) | ((result & 1u) << 5)));
}

/* RAM:f896ee */
u8 PLEXTOR_NEAR_FUNCTION FUN_RAM_f896ee(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u16 duration = 10u;
    u8 sample_count = 3u;

    if(operating_code < 0x10u)
    {
        if(operating_code == 0x0cu)
            duration = 0x1eu;
        else if(operating_code == 0x08u)
            duration = 0x32u;
        else if(operating_code == 0x04u)
            duration = 0x64u;
        else
            duration = (u16)(0x015eu / operating_code);
        sample_count = 5u;
    }
    WaitTimer1ScaledDuration(duration, 0u);
    return TestP1Cex0PulseSuccessThreshold(sample_count);
}

/* RAM:fc67ff */
void ResetRawReadProgressCountersAndPhaseFlags(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15E2) >= 10u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 8u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37) >= 0x28u
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u)
        ResetRawReadQueueAndTransferState();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E3, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E1, 0u);
    FirmwareWriteBit003E(0u);
    FirmwareWriteBit003D(0u);
    FirmwareWriteBit0040(1u);
}


/* RAM:fb8e03 */
void ArmControllerThreeBytePacketCapture(void)
{
    u8 saved_ie = FirmwareReadDirectSfr8(A8);
    u8 saved_a9 = FirmwareReadDirectSfr8(A9);
    u8 controller_shadow;

    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1206, 0x1eu);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) | 0x04u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_shadow);
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}


/* RAM:fb418e */
void NormalizeWriteSpeedForCurrentMedia(void)
{
    u8 controller_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    u8 current_speed;

    if(controller_mode == 1u)
    {
        current_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
        if(current_speed < 8u)
            current_speed = 4u;
        else if(current_speed < 0x10u)
            current_speed = 8u;
        else if(current_speed < 0x14u)
            current_speed = 0x10u;
        else if(current_speed < 0x18u)
            current_speed = 0x14u;
        else if(current_speed < 0x20u)
            current_speed = 0x18u;
        else if(current_speed < 0x28u)
            current_speed = 0x20u;
        else if(current_speed < 0x34u)
            current_speed = 0x28u;
        else
            current_speed = 0x34u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, current_speed);
    }
    else if(controller_mode == 2u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x18u);
        }
        else
        {
            u8 upper_speed = (FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 4u
                                 || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 0u)
                               ? 4u
                               : 10u;
            current_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 10u ? 4u : 10u;
            if(upper_speed < current_speed)
                current_speed = upper_speed;
            if(current_speed < 4u)
                current_speed = 4u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, current_speed);
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x10u) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7E, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) | 0x10u));
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7E, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xefu));
    }
}


/* RAM:fbd4f5 */
u8 SelectStateEligibleLowestWriteSpeed(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) < 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) != 3u)
        return 0u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x2000u) != 0u)
        return 4u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x1000u) != 0u)
        return 8u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0800u) != 0u)
        return 10u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0400u) != 0u)
        return 12u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0200u) != 0u)
        return 16u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0100u) != 0u)
        return 20u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0080u) != 0u)
        return 24u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0040u) != 0u)
        return 32u;
    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x0020u) != 0u)
        return 40u;
    return 52u;
}

/* RAM:fd68ba */
/* ERET leaf; only stages bytes, with no programming or activation call. */
void StagePrimaryDriveConfigurationForAuxiliaryUpdate(void)
{
    u8 index;
    for(index = 0u; index < 0x60u; ++index)
        FirmwareWrite8(FW_HUGE_UNKNOWN_10C000 + index, FirmwareRead8(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FFC000) + index));
}

/* RAM:fe01a6 */
/* JLE compares the masked backlog as a signed word. Capacity is reread on
 * the consuming path, and each subtraction truncates before shifting. */
void LimitRawReadSecondaryCursorBacklog(void)
{
    u16 backlog = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1298);
    u16 capacity;
    backlog = (u16)(backlog - ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1294));
    backlog &= ReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A);
    capacity = (u16)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_156A) - 6ul);
    if((s16)backlog > (s16)(capacity >> 1))
    {
        capacity = (u16)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_156A) - 6ul);
        ConsumeRawReadDescriptors((u16)(capacity >> 2));
    }
}

/* RAM:fde972 */
/* Host interrupt calls when RAM:000f47 is nonzero. Batch size is reread
 * after backlog maintenance before advancing the completed count. */
void CompleteBatchedHostTransferBuffers(void)
{
    u16 completed, cursor;
    u32 total;
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
    completed = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1463);
    cursor = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1298);
    cursor = (u16)(cursor + completed);
    cursor &= ReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1298, cursor);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F47, 0u);
    LimitRawReadSecondaryCursorBacklog();
    completed = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1463);
    total = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1459);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1459, total + completed);
}

/* RAM:fde9e4 */
/* The retained-transfer flag is tested only after the four unconditional
 * controller-port writes. The saved status and error bytes are then read in
 * their native order before the completion state is published. */
void ResetRawReadHostTransferControllerState(void)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1597) != 0u)
        return;
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2F));
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C30));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 6u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x02u));
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 3u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
}

/* RAM:fcbb85 */
/* The mode byte is sampled once. Values outside zero through three take the
 * firmware's intentional fail-stop path. */
void PrepareReadVerifyCommandState(void)
{
    u8 mode = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);

#define PLEXTOR_BINDING_SECTION 85
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if(mode == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 0u);
        return;
    }

    if(mode <= 3u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) != 2u)
            InitializeReadVerifyWorkingTrackState();
        return;
    }

    for(;;)
    {
    }
}

/* RAM:fedb7d */
void ExecuteWriteAndVerifyReadback(void)
{
    u32 start_logical_block_address;
    u32 end_logical_block_address;
    u8 original_mode_bit = 0u;
    u8 restore_mode_bit = 0u;
    u8 mode_bits;

    start_logical_block_address = ReadBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BC707);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_0B02, start_logical_block_address);
    end_logical_block_address = ReadBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BC70B);
    start_logical_block_address = ReadBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BC707);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_0B06, end_logical_block_address - start_logical_block_address);

    PrepareReadVerifyCommandState();
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >> 7) & 1u) != ((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >> 2) & 1u))
    {
        original_mode_bit = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >> 7) & 1u);
        mode_bits = (u8)((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >> 2) & 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x7fu) | (u8)(mode_bits << 7)));
        restore_mode_bit = 1u;
    }

    ExecuteReadVerifyRange();
    if(restore_mode_bit != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x7fu) | (u8)(original_mode_bit << 7)));
    FinalizeReadVerifyCommandState();
}

/* RAM:fb532c */
void PLEXTOR_NEAR_FUNCTION FinalizeWriteType2TrackTableState(void)
{
    u8 next_record_index;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 1u)
        return;

    UpdateTrackTableReferenceFromCurrentSessionPointB0();
    DispatchPopulateTrackTableFromDecodedTocTrackRange();
    SetType2LayoutBitAtomic(EvaluateThirtySevenTrackLayoutForType2Finalization());
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u)
        return;

    InitializeTrackTableSessionFrameBounds();
    next_record_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, next_record_index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, next_record_index);
    FirmwareWriteNear8((u16)(0xdd87u + (u16)next_record_index * 19u), 0u);
    SnapshotTrackTableTailState();
}

/* RAM:fb5301 */
/* The adjusted value is derived from the just-published snapshot, matching
 * the original volatile reread of RAM:00157a rather than RAM:00dd41. */
void PLEXTOR_NEAR_FUNCTION CommitTrackTailState(void)
{
#define PLEXTOR_BINDING_SECTION 86
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_157A, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_157E, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_157A) - 150ul);
    UpdateTrackTableFromWorkingTrack();
    ClearWorkingTrackInformation();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 1u);
    SnapshotTrackTableTailState();
    UpdateTrackRecordControlFlag000A();
}

/* RAM:fbcd3c */
void UpdateWorkingTrackAfterWriteFailure(void)
{
    u16 sense_code = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B24);

    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_0C3A, sense_code);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 0x01u) != 0u)
        return;

    if(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0C3A) == 0x0c09u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 0x01u) == 0u)
    {
        u32 cursor = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D) + 2ul;

        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, cursor);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD45, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D));
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD5B, 0ul);
        DispatchFinalizeTrackStateForWriteFailure(0x35u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_128C) == 0u)
        return;
    if(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0C3A) == 0x0200u && FirmwareReadBit0031() == 0u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x80u));
    {
        u8 track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);

        if(track_type == 0u || track_type == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 6u);
        else if(track_type == 2u || track_type == 3u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 5u);
    }
    {
        u32 failure_frame = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1484);

        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD51, failure_frame);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D, failure_frame);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, FirmwareReadNear8(FW_NEAR_UNKNOWN_1287));
    DispatchUpdateTrackTableForWriteFailure();
}

/* RAM:fb51d5 */
void FinalizeTrackStateForScsiOpcode(u8 scsi_opcode)
{
    if(scsi_opcode == 0x35u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 1u);
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u)
        {
            u32 working_end = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41);
            u32 compared_end = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD45);

            if(compared_end == working_end)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
            }
        }
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
    }
    CommitTrackTailState();
}

/* RAM:fb52b3 */
void PLEXTOR_NEAR_FUNCTION CommitTrackCloseStateForCurrentWriteSpeed(void)
{
    u8 data_block_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    u8 track_mask;

    if(data_block_type == 9u)
        track_mask = 0x20u;
    else if(data_block_type == 10u || data_block_type == 11u || data_block_type == 13u)
        track_mask = 0x10u;
    else if(data_block_type == 12u)
        track_mask = 0x08u;
    else
        track_mask = 0x40u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, track_mask);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
    CommitTrackTailState();
}

/* RAM:fb521b */
void PLEXTOR_NEAR_FUNCTION FinalizeWriteType1WorkingTrackState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, FirmwareReadNear8(FW_NEAR_UNKNOWN_1287));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u)
        StoreWorkingTrackAtEmptyTrackTableTail();
    else
        CommitTrackTailState();
}

/* RAM:fb5112 */
u8 EnsureOpcForSelectedWriteSpeed(void)
{
    if(IsOpcRequiredForWriteSpeed(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38)) != 1u)
        return 1u;
    return DispatchRunOpcForSelectedWriteSpeed() != 0u ? 1u : 0u;
}

/* RAM:fb616e */
void ServicePendingHostCommandIfAny(void)
{
    if(FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) != 1u)
        ServicePendingHostCommandDuringMediaOperation();
}

/* RAM:fb4701 */
u8 ServiceImmediatePendingHostCommandAndReturnTrue(void)
{
    if(FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) != 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) != 0u)
        ServicePendingHostCommandDuringMediaOperation();
    return 1u;
}

/* RAM:fa8605 */
void DispatchEightChannelMeasurementCaptureForCurrentQ(void)
{
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) >> 3) & 1u) == 1u && DoesCurrentDecodedFrameMatchFullTocPointA2ForDispatch() == 0u)
        ServiceEightChannelMeasurementCaptureForDispatch();
}

/* RAM:fd07bb */
void EnsureStateFlag11EFBit3Initialized(void)
{
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 3) & 1u) == 0u)
    {
        DispatchLoadControllerCalibrationConfigurationStateForGuard();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 0x08u));
    }
}

/* RAM:fd26e5 */
void HandleSeek10Command(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DFA)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    PrepareAndDispatchReadOrSeekAcquisitionAtLbaForSeek10(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0C));
}

/* RAM:fb237d */
void HandleRezeroUnitCommand(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DEC)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E, 0x96ul);
    (void)DispatchAndWaitForReadAcquisitionResult();
}

/* RAM:fb28b3 */
void HandleReceiveDiagnosticResultsCommand(void)
{
    u16 allocation_length;
    u8 transfer_length;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DDE)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCBD, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCBE, 6u);
    allocation_length = ReadBigEndianU16(FW_NEAR_UNKNOWN_0B0D);
    if(allocation_length < 9u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    transfer_length = MinU8((u8)allocation_length, 8u);
    if(CopyResponseToControllerBufferAndExecuteDataIn(FW_NEAR_UNKNOWN_DCBD, (u32)transfer_length) != 4u)
        SetNoSenseData();
}

/* RAM:fdcc69 */
void ExecuteReadAcquisitionAndWaitForCompletion(u8 allow_target_reached_return)
{
    if(DispatchAndWaitForReadAcquisitionResult() == 0u)
        return;

    if(allow_target_reached_return != 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) >> 4) & 1u) != 0u)
    {
        for(;;)
        {
            s32 current_position = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);
            s32 target_position = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E);

            if(target_position <= current_position)
                return;
            YieldCoroutine0();
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 1u) == 0u)
            {
                SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBC));
                return;
            }
        }
    }

    while((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 1u) != 0u)
        YieldCoroutine0();
    SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBC));
}

/* RAM:fcb4ba */
void DispatchVendorF3ReadAcquisitionMode(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
        return;
    }

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) >> 1) & 1u) == 0u)
    {
        SetSenseData(2u, 0x0401u);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) >> 4) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_131A) != 0u)
            DispatchInitializeVendorF3AlternateReadAcquisitionState();
        else
            HandleUnsupportedScsiCommand();
        return;
    }

    if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u)
        HandleUnsupportedScsiCommand();
    else
        DispatchInitializeVendorF3ReadAcquisitionMeasurementState();
}

/* RAM:fcb515 */
void StartVendorF3ReadAcquisitionBetweenMsfTuples(fw_addr_t start_msf_address, fw_addr_t end_msf_address)
{
    s32 end_position = ConvertBinaryMsfToAdjustedFramePosition(end_msf_address);
    s32 start_position = ConvertBinaryMsfToAdjustedFramePosition(start_msf_address);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x04u));
    if(end_position == start_position)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 3u);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E, (u32)end_position);
        (void)DispatchAndWaitForReadAcquisitionResult();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8D, 0u);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E, (u32)end_position);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C92, (u32)start_position);
        ExecuteReadAcquisitionAndWaitForCompletion(1u);
    }
}

/* RAM:fcb587 */
void HandleScsiOpcodeF4(void)
{
    s32 reference_position;
    u8 next_track_index;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 0x02u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x02u) != 0u)
        reference_position = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_0B0F);
    else
        reference_position = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0E);

    if(reference_position >= (s32)ReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION))
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x01u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1149, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74C, 0u);
    }
    ClearWorkingTrackInformation();
    ResetTrackInformationTableState();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) < 99u)
        next_track_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) + 1u);
    else
        next_track_index = 99u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, next_track_index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, next_track_index);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD7A, (u32)reference_position);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x7fu));
    SnapshotTrackTableTailState();
}

/* RAM:fbf1b6 */
void ResetControllerAndIdleCoroutine1(void)
{
    ResetControllerOperatingState();
    DispatchResetControllerHardwareRegisters();
    for(;;)
        YieldCoroutine1();
}

/* RAM:fd4f8e */
void RunConfiguredRawSectorReadCoroutine0(void)
{
    DispatchStartFormat2RawSectorReadAtFramePosition((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FBF8));
    for(;;)
        YieldCoroutine0();
}

/* RAM:fdd897 */
void RunControllerRequestServiceLoopCoroutine3(void)
{
    ConfigureReadAcquisitionControllerForCurrentState();
    for(;;)
    {
        ServiceControllerRequestFlags11E5();
        YieldCoroutine3();
    }
}

/* RAM:fab021 */
void WaitForWritePipelineStatusCompletion(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    if(FirmwareRead8(FW_HUGE_UNKNOWN_2BA5C6) == 0u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    while(PollWritePipelineTransferBatchStatus() == 0xfeu)
        YieldCoroutine0();
}

/* RAM:fc444e */
u8 DispatchReadAcquisitionAndWaitForResult(void)
{
    ClearP3Bit0Atomic();
    ClearP3Bit1Atomic();
    DispatchPendingReadAcquisitionRequest();
    YieldCoroutine3();

    for(;;)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) == 0u)
        {
            SetP3Bit0Atomic();
            ClearP3Bit1Atomic();
            return FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) == 0u ? 1u : 0u;
        }
        YieldCoroutine3();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        {
            SetP3Bit0Atomic();
            ClearP3Bit1Atomic();
            return 0u;
        }
    }
}

/* RAM:fab52e */
u8 ServiceFirstPendingType2ControllerRecord(void)
{
    u8 index;

    for(index = 0u; index < 8u; ++index)
    {
        fw_addr_t state_address = FW_NEAR_UNKNOWN_FB2B + (fw_addr_t)index * 5ul;

        if(FirmwareRead8(state_address) == 1u)
        {
            FirmwareWrite8(state_address, 0u);
            return AcquireFortyFrameType2ControllerRecordBaseWindow() != 0u ? 1u : 0u;
        }
    }
    return 1u;
}

/* RAM:fa6dfb */
u8 ExecuteReadAcquisitionCalibrationForMode2(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) & 0xfcu) | 2u));
    if(ExecuteReadAcquisitionCalibrationSequence() != 0u)
        return 1u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, 0x1eu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x1eu);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
    return 0u;
}

/* RAM:fadaa5 */
void AccumulateReadMeasurementWindowResult(u8 sample_divisor)
{
    u32 accumulator = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_150B);
    u16 middle = (u16)(accumulator >> 8);
    u16 quotient;
    u16 folded;
    u32 result;

    quotient = (u16)((u16)(middle / 75u) << 8);
    quotient = (u16)(quotient + (u16)((u16)(((u16)(middle % 75u) << 8) | (u16)(accumulator & 0xffu)) / 75u));
    folded = (u16)(quotient << 1);
    if(sample_divisor != 0u)
        folded = (u16)(folded / sample_divisor);

    result = (u32)folded;
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_150F, result);
    result = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_150F);
    accumulator = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_14D6);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_14D6, accumulator + result);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_150B, 0ul);
}

/* RAM:fa98ca */
void AdvanceType2ControllerRecordCaptureCursor(void)
{
    u16 low_word;
    u16 high_word;
    u32 cursor;

    if(FirmwareRead8(FW_HUGE_UNKNOWN_2BA524) == 0u)
    {
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA527, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA528, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA525, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA526, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA524, 1u);
        return;
    }

    low_word = (u16)(FirmwareReadU16(FW_HUGE_UNKNOWN_2BA527));
    high_word = (u16)(FirmwareReadU16(FW_HUGE_UNKNOWN_2BA525));
    cursor = (((u32)high_word << 16) | low_word) + 1ul;
    FirmwareWriteU16(FW_HUGE_UNKNOWN_2BA527, (u16)cursor);
    FirmwareWriteU16(FW_HUGE_UNKNOWN_2BA525, (u16)(cursor >> 16));
}

/* RAM:fa759c */
u8 PLEXTOR_NEAR_FUNCTION WaitForReadMeasurementRangeStart(void)
{
    u8 packet_result;
    s32 start_threshold;

    ArmControllerThreeBytePacketCapture();
    for(;;)
    {
        do
        {
            packet_result = ParseReadMeasurementRangeStartPacket();
            if(packet_result == 0xfeu)
                YieldCoroutine1();
        } while(packet_result == 0xfeu);

        ArmControllerThreeBytePacketCapture();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94));
            return 0u;
        }

        start_threshold = (s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_F091) - 10ul);
        if((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235) >= start_threshold)
        {
            if(FirmwareReadBit0407() == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, 8u);
                return 0u;
            }
            return 1u;
        }
    }
}

/* RAM:fa7602 */
void HandleControllerEventBit2MeasurementService(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0x02u) != 0u)
        DispatchAdvanceControllerMeasurementSeries00And01Sequence();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0x08u) != 0u)
        AdvanceControllerD8D9MeasurementSeries();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0x04u) != 0u)
        DispatchAdvanceFixedControllerParameterSequence();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0x01u) != 0u)
        ServicePeriodicControllerD8D9DifferenceSampling();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0x20u) != 0u || (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) < 0)
        DispatchServiceControllerOperatingProfileInputMatches();
}

/* RAM:fa7740 */
void CaptureOffsetSfrDbAverageForMeasurementIndex(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u16 sample_sum = 0u;
    u8 sample_index;
    fw_addr_t output_address;

    if(operating_code == 2u)
        WaitTimer1ScaledDuration(350u, 0u);
    else if(operating_code == 4u)
        WaitTimer1ScaledDuration(180u, 0u);
    else if(operating_code == 8u)
        WaitTimer1ScaledDuration(100u, 0u);
    else if(operating_code == 10u)
        WaitTimer1ScaledDuration(80u, 0u);

    FirmwareWriteDirectSfr8(E1, 1u);
    SetRxdatBit3Atomic();
    SetBit070FAtomic();
    for(sample_index = 0u; sample_index < 10u; ++sample_index)
    {
        while((FirmwareReadDirectSfr8(F1) & 0x08u) == 0u)
        {
        }
        sample_sum = (u16)(sample_sum + FirmwareReadDirectSfr8(DB));
    }
    ClearBit070FAtomic();
    ClearRxdatBit3Atomic();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F077 + FirmwareReadNear8(FW_NEAR_UNKNOWN_EF76), (u8)(sample_sum / 10u));
    output_address = FW_NEAR_UNKNOWN_F077 + FirmwareReadNear8(FW_NEAR_UNKNOWN_EF76);
    FirmwareWrite8(output_address, (u8)(FirmwareRead8(output_address) + 0x11u));
}

/* RAM:fa766f */
void AdvanceControllerMeasurementSeries00And01Sequence(void)
{
    u8 phase;
    u8 index;
    u16 series_value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 0u);
        ConfigureControllerRegisters32Through35ForOperatingCode();
        index = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF76);
        series_value = ReadBigEndianU16Near((u16)(0xef89u + (u16)index * 2u));
        WriteControllerParameterEF00AndConditionalD0Value((u8)series_value);
        index = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF76);
        series_value = ReadBigEndianU16Near((u16)(0xefa7u + (u16)index * 2u));
        WriteControllerParametersEAEBFromScaledValue(series_value);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, 1u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) >= 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) <= 15u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) == 1u)
        {
            index = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF76);
            series_value = ReadBigEndianU16Near((u16)(0xefa7u + (u16)index * 2u));
            WriteControllerParametersEAEBFromScaledValue(series_value);
            WriteControllerParameterWord((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0D)));
        }
        else
        {
            index = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF76);
            series_value = ReadBigEndianU16Near((u16)(0xef89u + (u16)index * 2u));
            WriteControllerParameterEF00AndConditionalD0Value((u8)series_value);
            index = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF76);
            series_value = ReadBigEndianU16Near((u16)(0xefa7u + (u16)index * 2u));
            WriteControllerParametersEAEBFromScaledValue(series_value);
        }
        CaptureOffsetSfrDbAverageForMeasurementIndex();
        phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, (u8)(phase + 1u));
        index = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF76);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF76, (u8)(index + 1u));
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) == 16u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 1u);
        CompleteControllerMeasurementSequence();
        WriteControllerParameterWord((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0E)));
        phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, (u8)(phase + 1u));
    }
}

/* RAM:fa77e5 */
void AdvanceFixedControllerParameterSequence(void)
{
    u8 phase;
    u16 value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 0u);
        ConfigureControllerRegisters32Through35ForOperatingCode();
        value = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_F066);
        WriteControllerParameterEF00AndConditionalD0Value((u8)value);
        WriteControllerParametersEAEBFromScaledValue(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_F068));
        phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, (u8)(phase + 1u));
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) == 1u)
    {
        WriteControllerParametersEAEBFromScaledValue(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_F068));
        WriteControllerParameterWord((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0D)));
        phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, (u8)(phase + 1u));
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75) == 2u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0x04u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 1u);
            CompleteControllerMeasurementSequence();
            WriteControllerParameterWord((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0E)));
        }
        phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_EF75);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, (u8)(phase + 1u));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 1u);
    CompleteControllerMeasurementSequence();
    WriteControllerParameterWord((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0E)));
}

/* RAM:fa78b1 */
void CompleteControllerMeasurementSequence(void)
{
    u8 control_shadow;

#define PLEXTOR_BINDING_SECTION 87
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
    ClearBit0547Atomic();
    control_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, control_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, control_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0x30u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0x10u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BD, 1u);
    control_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, control_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, control_shadow);
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetSfrA9Bit2Atomic();
    DispatchApplyControllerOperatingParametersForState2Path();
}

/* RAM:fa66f5 */
void ServiceControllerOperatingProfileInputMatches(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_10B6) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1133) && FirmwareReadNear8(FW_NEAR_UNKNOWN_10B7) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1134)
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_10B8) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1135))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xdfu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfeu));
        ApplyControllerOperatingAdjustmentAndSelectResponse();
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0x80u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_10BA) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1133)
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_10BB) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1134) && FirmwareReadNear8(FW_NEAR_UNKNOWN_10BC) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1135))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0x7fu));
        DISPATCH_COMPUTE_CONTROLLER_OPERATING_PROFILE_ADJUSTMENT();
    }
}

/* RAM:fddff4 */
u8 ExecuteDataOutAndCopyFromControllerBuffer(fw_addr_t destination, u32 byte_count)
{
    u32 buffer_offset;
    fw_addr_t source;
    u32 index;

    if(byte_count == 0ul)
        return 5u;

    buffer_offset = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    ExecuteControllerDataOutTransferFromBufferOffset(byte_count, buffer_offset);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) != 13u)
        return 4u;

    source = FirmwareAddressIncrement24(buffer_offset, FW_ADDRESS_BANK_STRIDE);
    for(index = 0ul; index < byte_count; ++index)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        destination = FirmwareAddressIncrementLow16(destination, 1u);
        source = FirmwareAddressIncrement24(source, 1u);
    }
    return 5u;
}

/* RAM:fdce3c */
void InitializeMediaReadControlAcquisitionState(void)
{
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, 150ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_159C, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1309) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, 150ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 3u) < 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
        return;
    }

    InitializeCoroutine2AtF90000ForC();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1591, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
}

/* RAM:f85e15 */
void FinalizeMediaCoroutineState2(void)
{
    DispatchFinalizeControllerCommand0EMeasurementCapture();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 0x08u));
}

/* RAM:f85e42 */
void FinalizeMediaCoroutineState5(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0x7fu));
    ResetControllerPorts80C4And80C5State();
    WriteControllerParametersD412AndE2Value58();
    RestoreFourSpeedWriteCalibrationOffset();
    if(FirmwareReadBit0032() != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E50, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F));
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 3) & 1u) == 0u)
        StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018080, 0u);
    ClearBit0036Atomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 0x08u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    ClearRxdatBit3Atomic();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806A, 0u);
}

/* RAM:fa6b8a */
void FinalizeControllerEventBit4OperatingUpdate(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) & 1u) == 1u)
        ClearPositionSnapshotFlagsAndCaptureCurrentFrame();

    ResetControllerPorts80C4And80C5State();
    if(ReadBit0038Atomic() == 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        DispatchApplyControllerOperatingParametersForState2Path();
        WriteControllerParameterWord((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0E)));
    }
    else
    {
        DispatchApplyControllerOperatingParametersForState1Path();
    }
    WriteControllerParameterWord((u16)(0xe700u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15)));
    ClearBit0038Atomic();
}

/* RAM:fd1367 */
void TimerDispatchOddPhase(void)
{
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_11F8, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_11F8) - 1ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FB0, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB0) - 1ul);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_11FC, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_11FC) - 1ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1221, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1221) - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1222, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1222) - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1223, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1223) - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_121F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_121F) - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) - 1u));

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) >> 6) & 1u) != 1u)
        ServiceControllerChannel2GateTimeoutTick();
    if(IsIndexedDriveStateActive(5u) != 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) >> 2) & 1u) != 0u)
        ServiceControllerWord0251SweepSampleTick();
}

/* RAM:fd100b */
void InitializeScaledRadialPickupMoveProfile(void)
{
    static const u8 descending_ramp[14] = { 0x7fu, 0x64u, 0x55u, 0x49u, 0x45u, 0x43u, 0x3cu, 0x3cu, 0x33u, 0x2fu, 0x2fu, 0x2fu, 0x2fu, 0x2fu };
    static const u8 ascending_ramp[14] = { 0x2fu, 0x2fu, 0x30u, 0x31u, 0x32u, 0x32u, 0x33u, 0x33u, 0x3cu, 0x3cu, 0x42u, 0x4cu, 0x5bu, 0x77u };
    u32 range_limit;
    u8 index;

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_10C4, 15ul);
    range_limit = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_10C4);
    for(index = 0u; index < 5u; ++index)
        range_limit *= 2ul;
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_10C8, (u16)range_limit);

    for(index = 0u; index < 14u; ++index)
        FirmwareWriteNear8((u16)(0x10cau + index), descending_ramp[index]);
    for(; index < 0x56u; ++index)
        FirmwareWriteNear8((u16)(0x10cau + index), 0x2fu);
    for(; index < 100u; ++index)
        FirmwareWriteNear8((u16)(0x10cau + index), ascending_ramp[index - 0x56u]);
}

/* RAM:fd00a5 */
void MovePickupByByteDistance(u8 distance, u8 move_direction)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018007, 0x48u);
    ClearControllerChannel86ForOperatingThresholds();
    if(ExecutePickupMoveControllerSequence((u16)distance, move_direction) == 0u)
    {
        WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FC2B, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2D, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2E, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2F, 3u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0xfeu));
}

/* RAM:fd0000 */
void MovePickupByDistanceInChunks(u8 move_direction, u16 distance)
{
    u16 chunk_limit;
    u16 chunk_distance;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018007, 0x48u);
    ClearControllerChannel86ForOperatingThresholds();

    chunk_limit = 50u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 4u)
        chunk_limit = 100u;

    while(distance != 0u)
    {
        if(distance < chunk_limit)
        {
            chunk_distance = distance;
            distance = 0u;
        }
        else
        {
            distance = (u16)(distance - chunk_limit);
            chunk_distance = chunk_limit;
        }

        if(ExecutePickupMoveControllerSequence(chunk_distance, move_direction) == 0u)
        {
            WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FC2B, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2D, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2E, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2F, 3u);
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0xfeu));
}

/* RAM:fd0a87 */
void InitializePickupSeekControlParameters(void)
{
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FC2B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2D, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2E, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2F, 3u);

    SubmitControllerWordPair(0x6000u, 0x0224u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E9, 0x00u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E7, 0x06u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180EB, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180DF, 0x0au);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E0, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E1, 0xa3u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D7, 0x10u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E2, 0x52u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E3, 0xacu);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E8, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180EC, 0x00u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180DA, 0x01u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180DE, 0x00u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D6, 0x82u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018496, 0xc9u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1002, 0x10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1003, 0xf0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018498, FirmwareReadNear8(FW_NEAR_UNKNOWN_1002));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018499, FirmwareReadNear8(FW_NEAR_UNKNOWN_1003));
    InitializeScaledRadialPickupMoveProfile();
}

/* RAM:fd0bc6 */
void PLEXTOR_NEAR_FUNCTION ConfigurePickupMoveControllerForDistance(u16 distance, u8 inward_direction)
{
    u8 needs_distance_profile;

    if(distance != ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FC2B))
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E0, distance == 1u ? 0x60u : 0x80u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180EE, (u8)distance);
    }

    if(distance != ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FC2B) || inward_direction != FirmwareReadNear8(FW_NEAR_UNKNOWN_FC2D))
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180EF, (u8)((u8)distance | (inward_direction == 1u ? 0x80u : 0x00u)));

    needs_distance_profile = 0u;
    if(distance != ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FC2B))
    {
        needs_distance_profile = 1u;
    }
    else
    {
        u8 cached_operating_code_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC2E);
        u8 current_operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

        if((u8)(current_operating_code >= 4u) != cached_operating_code_class)
            needs_distance_profile = 1u;
    }
    if(needs_distance_profile != 0u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180DA, 0x01u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E3, distance >= 10u ? 0xafu : 0xacu);
    }

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E2, 0x52u);
}

/* RAM:fd0d1c */
void PLEXTOR_NEAR_FUNCTION ProgramPickupMoveProfileForDistance(u8 direction, u16 distance)
{
    u16 word_011d = 0x00b0u;
    u16 word_011e;
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

    if(operating_code < 4u)
        word_011e = direction == 1u ? 0x0700u : 0x0840u;
    else
        word_011e = direction == 1u ? 0x02c0u : 0x03f0u;

    if(distance < 0x0100u)
    {
        fw_addr_t table_address;
        u8 bin;

        if(distance < 0x0010u)
            bin = 0u;
        else if(distance < 0x0020u)
            bin = 1u;
        else if(distance < 0x0040u)
            bin = 2u;
        else if(distance < 0x0060u)
            bin = 3u;
        else if(distance < 0x0080u)
            bin = 4u;
        else if(distance < 0x00a0u)
            bin = 5u;
        else if(distance < 0x00c0u)
            bin = 6u;
        else if(distance < 0x00e0u)
            bin = 7u;
        else if(distance < 0x00ffu)
            bin = 8u;
        else
            bin = 9u;

        operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
        if(operating_code < 4u)
            table_address = direction == 1u ? FW_ROM_PICKUP_MOVE_PROFILE_LOW_INWARD : FW_ROM_PICKUP_MOVE_PROFILE_LOW_OUTWARD;
        else
            table_address = direction == 1u ? FW_ROM_PICKUP_MOVE_PROFILE_HIGH_INWARD : FW_ROM_PICKUP_MOVE_PROFILE_HIGH_OUTWARD;
        table_address += (fw_addr_t)bin * 4ul;
        word_011d = FirmwareReadU16(table_address);
        word_011e = FirmwareReadU16(table_address + 2ul);
    }

    SubmitControllerWordPair(word_011d, 0x011du);
    SubmitControllerWordPair(word_011e, 0x011eu);
}

/* RAM:fd0ff2 */
void WriteControllerPort80E4Value03AndWait(void)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E4, 3u);
    WaitControllerPort80F8Bit7ClearWithoutTimeout();
}

/* RAM:fd0ecf */
u8 PLEXTOR_NEAR_FUNCTION FinalizePickupMoveControllerSequenceAndClassifyStatus(void)
{
    u8 pulse_failed = 0u;
    u8 controller_status = (u8)(WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180E3) & 0xc3u);

    if(controller_status == 0u)
        return 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3E) + 1u));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D8, 0u);
    if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) & 0x02u) == 0u)
        return 2u;

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A2, 0u);
    SubmitControllerWordPair(0u, 0x0251u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0u);
    if(TestP1Cex0PulseSuccessThreshold(20u) == 0u)
        pulse_failed = 1u;

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_11FC, 10ul);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_11FC, 25ul);
    while((s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_11FC) > 0)
        YieldCoroutine1();

    if(pulse_failed != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
    {
        ClearControllerParameterD1ShadowBit6();
        WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC) | 0x0020u));
    }

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E2, 0x52u);
    SetIndexedControllerChannelValue(1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98D));
    WriteControllerPort80E4Value03AndWait();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E2, 0x52u);
    SetIndexedControllerChannelValue(2u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));

    if((controller_status & 0x03u) != 0u)
        return 3u;
    if((controller_status & 0x40u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EF9) + 1u));
        return 4u;
    }
    return 0u;
}

/* RAM:fe2aa0 */
void EnqueueCurrentRawSectorDescriptorAtomic(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128A, 0u);
    ClearBit0547Atomic();
    BuildRawSectorDescriptorScratchZeroInputsForAtomicEnqueue();
    EnqueueRawSectorDescriptor();
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetBit054AAtomic();
}

/* RAM:fdd8a5 */
void ServiceControllerRequestFlags11E5(void)
{
    ServiceControllerRequestFlag11E5Bit0();
    ServiceControllerRequestFlag11E5Bit1();
    ServiceControllerRequestFlag11E5Bit5();
}

/* RAM:f9d448 */
void BuildControllerInterpolationTablesForMeasurementModes2Or3(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
        BuildControllerOperatingInterpolationGrid();
}

/* RAM:f9e0fd */
void BuildControllerOperatingInterpolationGrid(void)
{
#define PLEXTOR_BINDING_SECTION 88
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1058, 0x14u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1059, 0x18u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1057, 0u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1057) <= 0x17)
    {
        DispatchBuildControllerOperatingInterpolationGridColumn();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1057, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1057) + 1u));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1057, 0u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1057) <= 3)
    {
        DispatchBuildControllerOperatingInterpolationTableSlice();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1057, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1057) + 1u));
    }
}

/* RAM:f9ec58 */
s8 PLEXTOR_NEAR_FUNCTION LookupControllerProfileAuxiliaryValue(float profile_value, u8 auxiliary_index)
{
    fw_addr_t table_address;
    u8 alternate_family = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5E);

    if(profile_value > 20.0f)
        table_address = alternate_family == 1u ? PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83EF2) : PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83EC5);
    else
        table_address = alternate_family == 1u ? PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83EE3) : PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83EB6);

    return (s8)FirmwareRead8(FirmwareAddressIncrementLow16(table_address, auxiliary_index));
}

/* RAM:fb83a4 */
u8 PLEXTOR_NEAR_FUNCTION ValidateAndApplyAtipPacketForMode(u8 packet_validation_mode)
{
    u8 selector;
    u8 valid = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) >= 0xe0u)
    {
        selector = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E) & 7u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 1u) != 0u && packet_validation_mode == 2u)
            selector = 7u;

        if(selector == 6u)
            valid = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) == 0xf8u);
        else if(selector == 7u)
            valid = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) == 0xfcu);
        else if(selector == 4u)
            valid = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) == 0xf0u);
        else
            valid = 1u;
    }

    if(valid != 0u)
        DispatchDecodeAtipTimeFieldsForPacketValidation();
    return valid;
}

/* RAM:fb8319 */
void DecodeAtipTimeFieldsAndFramePositions(void)
{
    u8 inherited_offset = DispatchCaptureInheritedOffsetForAtipDecode();
    s32 frame_position;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_113F, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB53)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1140, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB54)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1141, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB55)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_113B, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB57)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_113C, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB58)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_113D, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB59)));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 1u);
    frame_position = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_113F);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_114F, (u32)frame_position);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FE9, (u32)frame_position);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);

    frame_position = ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_113B);
    WriteBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION, (u32)frame_position);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_FB8C, (u32)frame_position);
    DispatchApplyMediaStrategyForAtipDecode();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        DispatchPublishMediaStrategyOffsetForAtipDecode(inherited_offset);
}

/* RAM:fae3db */
void PLEXTOR_NEAR_FUNCTION BuildBasicReadDiscInformationResponse(void)
{
    u8 sentinel;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_E74B) << 3) & 0xf8u) + 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0x0eu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, DispatchFindFirstTrackForBasicDiscInformation());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, DispatchSelectDiscTypeForBasicDiscInformation());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3B, 0xffu);
    sentinel = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B3B);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B41, sentinel);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B40, sentinel);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3F, sentinel);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3D, sentinel);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3C, sentinel);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B4B, 0u);
}

/* RAM:fae4c0 */
u8 PLEXTOR_NEAR_FUNCTION SelectDiscInformationDiscTypeFromFullToc(void)
{
    u16 descriptor_count;
    u16 descriptor_index;
    fw_addr_t descriptor;
    u8 current_session;
    u8 next_session;
    u8 point;
    u8 disc_type = 0u;
    u8 has_a0 = 0u;
    u8 has_qualifying_track = 0u;

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        return 0x20u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) == 1u)
        return FirmwareRead8(FW_HUGE_UNKNOWN_2BB5E3);

    descriptor_count = (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_12CA) / 11u);
    descriptor = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2B8000, 11u);
    current_session = FirmwareRead8(descriptor);

    for(descriptor_index = 1u; descriptor_index <= descriptor_count; ++descriptor_index)
    {
        descriptor = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2B8000, (u16)(descriptor_index * 11u));
        if(FirmwareRead8(descriptor) == current_session)
        {
            point = FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 3u));
            if(point == 0xa0u)
            {
                disc_type = FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 9u));
                has_a0 = 1u;
            }
            point = FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 3u));
            if(point <= 99u && (FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 1u)) & 0x40u) != 0u)
                has_qualifying_track = 1u;
        }

        next_session = FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 11u));
        if(next_session != current_session)
        {
            if(has_a0 != 0u && has_qualifying_track != 0u)
                return disc_type;
            current_session = next_session;
            has_a0 = 0u;
            has_qualifying_track = 0u;
        }
    }

    for(descriptor_index = 1u; descriptor_index <= descriptor_count; ++descriptor_index)
    {
        descriptor = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2B8000, (u16)(descriptor_index * 11u));
        if(FirmwareRead8(descriptor) == 1u && FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 3u)) == 0xa0u)
            return FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 9u));
    }
    return 0xffu;
}

/* RAM:fae613 */
void HandleReadTrackInformationCommand(void)
{
    u16 allocation_length;

    if(DispatchValidateReadTrackInformationCdb(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CF5)) == 0u)
    {
        DispatchUnsupportedReadTrackInformation();
        return;
    }

    allocation_length = DispatchReadTrackInformationAllocationLength(FW_NEAR_UNKNOWN_0B11);
    allocation_length = DispatchClampReadTrackInformationLength(allocation_length, 0x001cu);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if(DispatchBuildReadTrackInformationFromFullToc() == 0u)
            return;
    }
    else if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        if(DispatchInitializeMediaCapacityForReadTrackInformation() == 0u)
            return;
    }
    else if(DispatchBuildReadTrackInformationFromTrackState() == 0u)
    {
        return;
    }

    if(allocation_length == 0u)
        DispatchSetNoSenseForReadTrackInformation();
    else if(DispatchTransferReadTrackInformationResponse(FW_NEAR_UNKNOWN_0B2A, allocation_length) != 4u)
        DispatchSetNoSenseForReadTrackInformation();
}

/* RAM:faf5b1 */
s32 PLEXTOR_NEAR_FUNCTION ComputeReadTrackInformationTrackSize(u8 track_number)
{
    fw_addr_t track_record = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2BB03B, (u16)((u16)track_number * 10u));
    s32 first_position;
    s32 second_position;

    if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 2u)) != track_number)
        return DispatchComputeFullTocTrackSpanForTrackSize(track_number);

    first_position = DispatchConvertMsfForReadTrackInformationTrackSize(FirmwareAddressIncrementLow16(track_record, 7u));
    second_position = DispatchConvertMsfForReadTrackInformationTrackSize(FirmwareAddressIncrementLow16(track_record, 3u));
    return (s32)((u32)second_position - (u32)first_position - 2ul);
}

/* RAM:fafee3 */
void FinalizeWorkingTrackAfterRepairOrCacheSync(void)
{
    u8 track_type;

    if(FirmwareReadBit0030() != 0u)
        DispatchClearBit0030ForWorkingTrackFinalizer();
    DispatchResetRawReadForWorkingTrackFinalizer();
    DispatchInitializeCoroutine2ForWorkingTrackFinalizer(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    DispatchYieldCoroutine0ForWorkingTrackFinalizer();
    if(DispatchEnsureOpcForWorkingTrackFinalizer() == 0u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x40u));
    if(DispatchPrepareWorkingTrackRawReadRangeForFinalizer() == 0u)
    {
        DispatchUpdateWorkingTrackAfterWriteFailureForFinalizer();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x7fu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xbfu));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xbfu));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD45, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1488));
    track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    if(track_type == 6u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 1u);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 3u);
    DispatchFinalizeTrackStateForWorkingTrackFinalizer(0x35u);
}

/* RAM:fdd287 */
void ResetRawReadQueueAndPrepareWritePipelineState(void)
{
    DispatchNormalizeWriteSpeedForPipelinePreparation();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    DispatchResetRawReadForPipelinePreparation();
    DispatchInitializeCoroutine2ForPipelinePreparation(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    DispatchYieldCoroutine0ForPipelinePreparation();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xbfu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    DispatchPrepareWritePipelineControllerRecordState();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0x7fu));
}

/* RAM:fdc5be */
void InitializeDriveSubsystems(void)
{
    u32 reserved_region_base;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 0u);
    ClearP3Bit0Atomic();
    ClearP3Bit1Atomic();
    FirmwareWriteDirectSfr8(85, 0u);
    FirmwareWriteDirectSfr8(86, 0x1bu);
    DispatchInitializeControllerAsicForDriveSubsystems();
    DispatchZeroMemoryForDriveSubsystems(FW_NEAR_UNKNOWN_0B00, 0x0d00u);
    DispatchZeroMemoryForDriveSubsystems(FW_NEAR_UNKNOWN_8000, 0x8000u);

    SetExternalInterrupt0EnableAtomic();
    SetBit0547Atomic();
    DispatchFillControllerBufferForDriveSubsystems(FW_HUGE_UNKNOWN_1B0000, FW_HUGE_UNKNOWN_010000, 0u);
    ClearBit0547Atomic();
    ClearExternalInterrupt0EnableAtomic();

    DispatchSelectMainInterruptHandlersForDriveSubsystems();
    DispatchInitializeRuntimeDriveStateForDriveSubsystems();
    DispatchInitializeDefaultProfileForDriveSubsystems();
    DispatchLoadFixedControllerTablesForDriveSubsystems();
    DispatchInitializePickupParametersForDriveSubsystems();
    DispatchResetControllerStateForDriveSubsystems();
    DispatchInitializeDriveConfigurationForDriveSubsystems();
    DispatchInitializeDiscControlForDriveSubsystems();

    FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) | 0x10u));
    FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) & 0xf5u));
    FirmwareWriteDirectSfr8(91, (u8)(FirmwareReadDirectSfr8(91) & 0xf5u));
    SetExternalInterrupt0EnableAtomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0u);
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetSfrA9Bit2Atomic();

    reserved_region_base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    DispatchFillControllerBufferForDriveSubsystems(reserved_region_base, FW_HUGE_UNKNOWN_1B0000 - reserved_region_base, 0u);
    ClearBit0547Atomic();
}

/* RAM:f92769 */
void PLEXTOR_NEAR_FUNCTION BuildRawSectorDescriptorForParsedQStatus(u16 base_descriptor_status)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) >> 2) & 1u) != 0u)
    {
        DispatchBuildRawSectorDescriptorScratch(0x6300u, 5u);
    }
    else if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) >> 5) & 1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) | 0x40u));
        DispatchBuildRawSectorDescriptorScratch(0x6400u, 5u);
    }
    else
    {
        DispatchBuildRawSectorDescriptorScratch((u16)(base_descriptor_status | 0x0200u), 3u);
    }
}

/* RAM:f934d7 */
void PLEXTOR_NEAR_FUNCTION EnqueueRawSectorDescriptorForParsedQStatus(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) >> 2) & 1u) != 0u)
    {
        DispatchBuildRawSectorDescriptorScratch(0x6300u, 5u);
        DispatchEnqueueParsedQRawSectorDescriptor();
        return;
    }

    DispatchBuildRawSectorDescriptorScratch(0x0200u, 3u);
    if(DispatchCheckP1Bit3StableForParsedQDescriptor() == 0u || (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598) >= (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_12C1))
        DispatchBuildRawSectorDescriptorScratch(0x2100u, 5u);
    DispatchEnqueueParsedQRawSectorDescriptor();
}

/* RAM:fdbdff */
void ConfigureMainDataRingGeometry(u8 geometry_mode)
{
    u32 ring_size;
    u8 controller_state;

    if(geometry_mode == 1u)
    {
        ring_size = 0x00093000ul;
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_155E, ring_size);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_156A, 0x00000100ul);
    }
    else if(geometry_mode == 2u)
    {
        ring_size = 0x00126000ul;
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_155E, ring_size);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_156A, 0x00000200ul);
    }
    else
    {
        return;
    }

    ClearBit0547Atomic();
    controller_state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018013, (u8)ring_size);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018014, (u8)(ring_size >> 8));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018015, (u8)(ring_size >> 16));
    controller_state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_state);
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetSfrA9Bit2Atomic();
}

/* RAM:fa844c */
void ServiceEightChannelMeasurementCapture(void)
{
    u8 saved_ie;
    u8 saved_a9;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 1u) == 0u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        return;

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) >> 2) & 1u) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_105D) < 4u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_105D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105D) + 1u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_105D) < 4u)
            return;

        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) >> 1) & 1u) != 0u)
        {
            s32 minimum_frame = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_105E);
            s32 current_frame = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235);

            if(current_frame < minimum_frame)
                return;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) | 4u));
    }

    saved_ie = FirmwareReadDirectSfr8(A8);
    saved_a9 = FirmwareReadDirectSfr8(A9);
    ClearBit0547Atomic();
    DispatchAccumulateEightChannelMeasurementCapture();
    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
}

/* RAM:fa81c9 */
s16 SelectControllerWord0255TargetForCurrentState(void)
{
    u16 target;

    if(DispatchIsIndexedDriveStateActiveForWord0255Target(0x1fu) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
        {
            target = FirmwareReadNear8(FW_NEAR_UNKNOWN_1041);
        }
        else
        {
            u8 index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1045);
            u8 adjustment = FirmwareReadNear8(FW_NEAR_UNKNOWN_104D + index);

            target = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1041) + (u16)((u16)adjustment * 3u));
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1041) == 0xffu)
            return (s16)0x8000u;
        target = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1042) * 3u);
        target = (u16)(target + FirmwareReadNear8(FW_NEAR_UNKNOWN_1041));
    }
    else
    {
        target = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC2);
    }
    return (s16)(u16)(target - 0x80u);
}

/* RAM:fc427a */
u8 PLEXTOR_NEAR_FUNCTION ProbeRawQAcrossSixFramesFromTarget(s32 initial_frame_position)
{
    u8 probe_index;
    u8 status = 0u;

    for(probe_index = 0u; probe_index < 6u; ++probe_index)
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, (u32)initial_frame_position + (u32)((u16)probe_index * 10u));
        status = DispatchProbeWorkingTrackRawQAtPipelineTarget();
        if(status == 1u || status == 3u)
            return status;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
            return 5u;
    }
    return status;
}

/* RAM:fc5c21 */
u8 ProbeWorkingTrackRawQAtPipelineTarget(void)
{
    u8 offset;
    s32 target;
    u8 status;
    u8 bit0017_set;

    for(offset = 0u; offset < 36u; offset = (u8)(offset + 2u))
    {
        if(offset < 10u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F64 + offset, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F65 + offset, 0u);
        }
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BB919 + offset, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BB91A + offset, 0u);
    }

    target = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598);
    if(target >= 0 && target < 451)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x10u) != 0u)
            return 4u;
#define PLEXTOR_BINDING_SECTION 89
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
        if(bit0017_set != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) != 0u)
            return 4u;
        FirmwareWriteBit0017(0u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    ClearP3Bit0Atomic();
    ClearP3Bit1Atomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 11u);
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2(FW_ENTRY_RUN_RAW_READ_POSITION_PROBE_COROUTINE2);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F07, 1u);
    do
    {
        YieldCoroutine3();
        status = DispatchInspectLatestRawSectorDescriptorForWorkingTrackProbe();
    } while(status == 0xfeu);
    SetP3Bit0Atomic();
    SetP3Bit1Atomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    return status;
}

/* RAM:fc3df5 */
u8 ProbeAndClassifyWorkingTrackBeforeStart(void)
{
    u8 status = ProbeRawQAcrossSixFramesFromTarget((s32)(u32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) - 90ul));

    if(status == 2u || status == 4u || status == 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, 15u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD37, 15u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 7u);
        return 1u;
    }
    if(status == 3u)
        return 1u;
    if(status != 1u)
        return 0u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F64) & 0xd0u) == 0x50u)
        return (u8)(ValidateWorkingTrackRawQState() != 0u);
    DecodeWorkingTrackModeFromTdiRecord(FW_NEAR_UNKNOWN_DD38, FW_NEAR_UNKNOWN_DD37);
    return 1u;
}

/* RAM:fc43dd */
u8 PLEXTOR_NEAR_FUNCTION ProbeRawQAtTargetAndCopyPacket(s32 frame_position, fw_addr_t packet_destination)
{
    u8 acquired;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E, (u32)frame_position);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) | 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) | 2u));
    acquired = DispatchRawQTargetReadAcquisition();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 0xfdu));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        return 5u;
    if(acquired == 0u)
        return 4u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 1u) == 0u)
        return 3u;
    CopyBytesWith8BitCount(packet_destination, FW_NEAR_UNKNOWN_1245, 10u);
    return 2u;
}

/* RAM:fc450b */
void PLEXTOR_NEAR_FUNCTION RefineRawQFrameBoundaryByStep(s32 *upper_frame, s32 *lower_frame, s32 step) PLEXTOR_REENTRANT
{
    while((s32)((u32)*upper_frame - (u32)*lower_frame) >= step)
    {
        s32 candidate = (s32)((u32)*lower_frame + (u32)step);
        u8 status;

        if(candidate >= *upper_frame)
            return;
        status = DispatchRefineRawQFrameProbe(candidate);
        if(status == 2u)
        {
            *lower_frame = candidate;
        }
        else if(status == 3u || status == 4u)
        {
            *upper_frame = candidate;
            return;
        }
    }
}

/* RAM:fc4483 */
s32 PLEXTOR_NEAR_FUNCTION RefineRawQFrameBoundaryAcrossStepSizes(s32 initial_frame, s32 upper_frame)
{
    s32 lower_frame = initial_frame;

    RefineRawQFrameBoundaryByStep(&upper_frame, &lower_frame, 45000l);
    RefineRawQFrameBoundaryByStep(&upper_frame, &lower_frame, 9000l);
    RefineRawQFrameBoundaryByStep(&upper_frame, &lower_frame, 1800l);
    RefineRawQFrameBoundaryByStep(&upper_frame, &lower_frame, 360l);
    RefineRawQFrameBoundaryByStep(&upper_frame, &lower_frame, 72l);
    return (s32)((u32)lower_frame - 7ul);
}

/* RAM:fc47e7 */
void PLEXTOR_NEAR_FUNCTION ComputeWorkingTrackFramePositionFromRawQDescriptor(fw_addr_t destination_address, fw_addr_t raw_q_descriptor_address)
{
    s32 first_position = ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(raw_q_descriptor_address, 3u));
    s32 second_position = ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(raw_q_descriptor_address, 7u));
    u32 result;

    if(FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_descriptor_address, 2u)) == 0u)
        result = (u32)first_position + (u32)second_position + 1ul;
    else
        result = (u32)second_position - (u32)first_position;

    WriteBigEndianU32Low16(destination_address, result);
}

/* RAM:fc4763 */
u8 PLEXTOR_NEAR_FUNCTION ProbeRawQAtThreeDecrementingFrameOffsets(s32 initial_frame_position, s32 reference_frame_position)
{
    u8 attempt;
    u8 status;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        return 5u;

    for(attempt = 0u; attempt < 3u; ++attempt)
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, (u32)initial_frame_position - (u32)((u8)(attempt * 10u)));
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0F36, (u32)reference_frame_position);
        status = DispatchProbeRawReadQPositionAtTargetForOffsets();
        if(status == 1u || (status == 6u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x50u) == 0x50u))
            return status;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
            return 5u;
    }
    return status;
}

/* RAM:fc42c9 */
u8 PLEXTOR_NEAR_FUNCTION InitializeWorkingTrackStateFromTdiPayload(void)
{
    fw_addr_t packet_address;
    u8 control_flags;
    u8 probe_status;
#define PLEXTOR_BINDING_SECTION 100
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    DecodeWorkingTrackModeFromTdiRecord(FW_NEAR_UNKNOWN_DD38, FW_NEAR_UNKNOWN_DD37);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        u8 signature_offset = 4u;
        fw_addr_t signature_address;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38) != 0x20u)
            signature_offset = 12u;
        signature_address = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2BB919, signature_offset);
        if(FirmwareRead8(signature_address) != 0x54u || FirmwareRead8(FirmwareAddressIncrementLow16(signature_address, 1u)) != 0x44u
            || FirmwareRead8(FirmwareAddressIncrementLow16(signature_address, 2u)) != 0x49u)
            return 1u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 1u);
    ComputeWorkingTrackFramePositionFromRawQDescriptor(FW_NEAR_UNKNOWN_DD3D, FW_NEAR_UNKNOWN_0F64);
    control_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F64);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, control_flags);
    if((control_flags & 0x50u) == 0x50u)
    {
        if(ParseWorkingTrackTdiRecordMetadata() == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 7u);
            return 1u;
        }
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 0u);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD57, 0ul);
    }

#define PLEXTOR_BINDING_SECTION 90
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    probe_status = ProbeRawQAtTargetAndCopyPacket((s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) + 15ul), packet_address);
    if(probe_status == 5u)
        return 0u;
    if(probe_status == 2u)
        ComputeWorkingTrackFramePositionFromRawQDescriptor(FW_NEAR_UNKNOWN_DD3D, packet_address);

    if((control_flags & 0x50u) == 0x50u)
        return (u8)(DispatchApplyRawQProbeStatusToWorkingTrackStateForTdi() != 0u);
    return (u8)(DispatchApplyRawQProbeStatusToWorkingTrackTailForTdi() != 0u);
}

/* RAM:fc41e6 */
u8 ClassifyAndInitializeWorkingTrackFromRawQProbe(void)
{
    s32 reference_frame = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39);
    s32 lead_out_frame = (s32)ReadBigEndianU32Near(FW_NEAR_CURRENT_FRAME_POSITION);
    u8 status;

    if(reference_frame >= lead_out_frame)
        return 1u;

    status = ProbeRawQAcrossSixFramesFromTarget((s32)((u32)reference_frame + 60ul));
    if(status == 2u || status == 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, 15u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD37, 15u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 7u);
        return 1u;
    }
    if(status == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 6u);
        ComputeWorkingTrackFramePositionFromRawQDescriptor(FW_NEAR_UNKNOWN_DD3D, FW_NEAR_UNKNOWN_0F64);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, FirmwareReadNear8(FW_NEAR_UNKNOWN_0F64));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 0u);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD57, 0ul);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD51, (u32)RefineRawQFrameBoundaryAcrossStepSizes(reference_frame, lead_out_frame));
        return 1u;
    }
    if(status == 4u)
        return 1u;
    if(status == 5u)
        return 0u;
    if(status == 1u)
        return InitializeWorkingTrackStateFromTdiPayload();
    return 0u;
}

/* RAM:fc45e3 */
u8 PLEXTOR_NEAR_FUNCTION ProbeWorkingTrackEndAndUpdateState(void)
{
    s32 lower_frame = (s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39) + 75ul);
    s32 upper_frame = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD41);
    u8 status;
    fw_addr_t packet_address;
#define PLEXTOR_BINDING_SECTION 101
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 91
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if((s32)((u32)upper_frame - (u32)lower_frame) > 75l)
    {
        status = ProbeRawQAtTargetAndCopyPacket((s32)((u32)upper_frame - 75ul), packet_address);
        if(status == 3u)
            lower_frame = RefineRawQFrameBoundaryAcrossStepSizes(lower_frame, upper_frame);
        else if(status == 4u)
            return 0u;
        else if(status == 2u)
            lower_frame = (s32)((u32)upper_frame - 75ul);
    }

    status = ProbeRawQAtThreeDecrementingFrameOffsets(lower_frame, upper_frame);
    if(status == 4u || status == 7u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 5u);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD51, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0F41));
        return 1u;
    }
    if(status == 5u)
        return 0u;
    if(status == 6u)
    {
        s32 observed_frame = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0F41);

        if(observed_frame < upper_frame)
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD45, (u32)observed_frame);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
        return 1u;
    }
    if(status == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
        return 1u;
    }

    /* The native body returns an uninitialized stack byte for other statuses. */
    return 0u;
}

/* RAM:fc40f0 */
u8 PLEXTOR_NEAR_FUNCTION InitializeWorkingTrackTypeFromRawQProbe(void)
{
    u8 status = ProbeRawQAcrossSixFramesFromTarget((s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D) - 90ul));

    if(status == 2u || status == 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, 15u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD37, 15u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 7u);
        return 1u;
    }
    if(status == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
        return 1u;
    }
    if(status == 4u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 5u);
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD51, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39));
        }
        return 1u;
    }
    if(status == 5u)
        return 0u;
    if(status != 1u)
        return 0u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F64) & 0xd0u) != 0x50u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
        DecodeWorkingTrackModeFromTdiRecord(FW_NEAR_UNKNOWN_DD38, FW_NEAR_UNKNOWN_DD37);
        return 1u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 3u);
    if(ValidateWorkingTrackRawQState() != 0u)
        return ProbeWorkingTrackEndAndUpdateState();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 2u);
        return 1u;
    }
    return 0u;
}

/* RAM:fc3c5a */
void PopulateTrackTableFromCurrentSessionRawQRecords(void)
{
    u8 accepted_record_seen = 0u;
    u8 track_number = 1u;
    fw_addr_t raw_q_record_address = FW_HUGE_UNKNOWN_2BB03B;
    u8 selected_track;
    u16 selected_offset;

    while(track_number <= FirmwareReadNear8(FW_NEAR_UNKNOWN_1149))
    {
        s32 record_frame_position;

        raw_q_record_address = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2BB03B, (u16)((u16)track_number * 10u));
        record_frame_position = ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(raw_q_record_address, 7u));
        if(record_frame_position >= (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD7A) && FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 2u)) != 0u)
        {
            if(accepted_record_seen == 0u)
            {
                u8 record_track = FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 2u));

                accepted_record_seen = 1u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, record_track);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, record_track);
            }
            if(FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 3u)) != 0xffu || FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 4u)) != 0xffu
                || FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 5u)) != 0xffu)
            {
                DispatchPopulateTrackTableRecordForCurrentSession(raw_q_record_address);
                DispatchLoadTrackInformationForCurrentSession(track_number);
                if(DispatchInitializeWorkingTrackTypeForCurrentSession() == 0u)
                    return;
                DispatchUpdateTrackTableForCurrentSession();
            }
        }
        track_number++;
    }

    selected_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    selected_offset = (u16)((u16)selected_track * 19u);
    FirmwareWriteNear8((u16)(0xdd87u + selected_offset), 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) > 99u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, FirmwareReadNear8((u16)(0xdd87u + selected_offset)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, selected_track);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD39, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD7A));
    if(selected_track == FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD39, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD7A));
    else
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD39, (u32)ConvertBinaryMsfToAdjustedFramePosition((u16)(0xdd71u + selected_offset)));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39) + 150ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, (u8)((FirmwareReadNear8((u16)(0xdd89u + selected_offset)) >> 1) & 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, FirmwareReadNear8((u16)(0xdd92u + selected_offset)));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u
           && (FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 3u)) != 0xffu || FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 4u)) != 0xffu
               || FirmwareRead8(FirmwareAddressIncrementLow16(raw_q_record_address, 5u)) != 0xffu))
        || DispatchClassifyWorkingTrackForCurrentSession() != 0u)
        DispatchUpdateTrackTableForCurrentSession();
}

/* RAM:fc3e63 */
void DeriveWorkingTrackEndFrameFromPmaOrToc(u8 track_number)
{
    fw_addr_t pma_record = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2BB03B, (u16)((u16)track_number * 10u));
    u16 descriptor_count;
    u16 descriptor_index;
    u8 session_number = 0u;
    u8 last_session_track = 0u;
    s32 session_lead_out = 0;
    fw_addr_t lead_out_copy_address;
#define PLEXTOR_BINDING_SECTION 102
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if(FirmwareRead8(FirmwareAddressIncrementLow16(pma_record, 2u)) == track_number)
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD41, (u32)ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(pma_record, 3u)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
        return;
    }

    descriptor_count = (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_12CA) / 11u);
    for(descriptor_index = 1u; descriptor_index <= descriptor_count; descriptor_index++)
    {
        fw_addr_t descriptor = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2B8000, (u16)(descriptor_index * 11u));

        if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 3u)) == track_number)
        {
            session_number = FirmwareRead8(descriptor);
            break;
        }
    }

#define PLEXTOR_BINDING_SECTION 92
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    for(descriptor_index = 1u; descriptor_index <= descriptor_count; descriptor_index++)
    {
        fw_addr_t descriptor = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2B8000, (u16)(descriptor_index * 11u));

        if(FirmwareRead8(descriptor) != session_number)
            continue;
        if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 3u)) == 0xa1u)
        {
            last_session_track = FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 8u));
        }
        else if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 3u)) == 0xa2u)
        {
            CopyBytesWith8BitCount(lead_out_copy_address, FirmwareAddressIncrementLow16(descriptor, 8u), 4u);
            session_lead_out = ConvertBinaryMsfToAdjustedFramePosition(lead_out_copy_address);
            break;
        }
    }

    if(track_number == last_session_track)
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD41, (u32)session_lead_out);
    else
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD41, (u32)ConvertBinaryMsfToAdjustedFramePosition((u16)(0xf2bfu + (u16)((u16)track_number * 6u))));
}

/* RAM:fb5412 */
/* ACC/B inputs, RET. */
void PLEXTOR_NEAR_FUNCTION UpdateWorkingTrackDataBlockTypeMaskFromRawTrackRecord(u8 track_number, u8 fallback_mask)
{
    u16 record_index = 0u;
    u16 last_record_index = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_B4BB);

    while(record_index <= last_record_index)
    {
        u16 record_offset = (u16)(record_index << 3);

        if(FirmwareReadNear8((u16)(0xb4beu + record_offset)) == track_number)
        {
            u8 record_type = (u8)(FirmwareRead8((u16)(0xb4c0u + record_offset)) & 0xf0u);

            if(record_type == 0x20u)
                fallback_mask = 0x10u;
            else if(record_type == 0x30u)
                fallback_mask = 0x20u;
            else if(record_type == 0x10u)
                fallback_mask = 0x40u;
            break;
        }
        record_index++;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, fallback_mask);
}

/* RAM:fb5370 */
void PLEXTOR_NEAR_FUNCTION PopulateTrackTableFromDecodedTocTrackRange(void)
{
    u8 track_number = FindFirstType1TrackRecordNumberOrOne();

    while(track_number <= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
    {
        u16 decoded_offset = (u16)((u16)track_number * 6u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, track_number);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D, (u32)ConvertBinaryMsfToAdjustedFramePosition((u16)(0xf2b9u + decoded_offset)));
        DeriveWorkingTrackEndFrameFromPmaOrToc(track_number);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, FirmwareReadNear8((u16)(0xf2b7u + decoded_offset)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, 0u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u)
            UpdateWorkingTrackDataBlockTypeMaskFromRawTrackRecord(track_number, (u8)(decoded_offset >> 8));
        UpdateTrackTableFromWorkingTrack();
        track_number++;
    }
}

/* RAM:fbd154 */
void PLEXTOR_NEAR_FUNCTION PopulateTrackTableFromFullTocTrackRange(void)
{
    u8 track_number;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
        return;
    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    while(track_number <= FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
    {
        u16 decoded_offset = (u16)((u16)track_number * 6u);
        u16 descriptor_count;
        u16 descriptor_index;
        fw_addr_t pma_record;

        ClearWorkingTrackInformation();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, track_number);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D, (u32)ConvertBinaryMsfToAdjustedFramePosition((u16)(0xf2b9u + decoded_offset)));
        DeriveWorkingTrackEndFrameFromPmaOrToc(track_number);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, FirmwareReadNear8((u16)(0xf2b7u + decoded_offset)));

        descriptor_count = (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_12CA) / 11u);
        for(descriptor_index = 1u; descriptor_index <= descriptor_count; descriptor_index++)
        {
            fw_addr_t descriptor = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2B8000, (u16)(descriptor_index * 11u));

            if(FirmwareRead8(FirmwareAddressIncrementLow16(descriptor, 3u)) == track_number)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, FirmwareRead8(descriptor));
                break;
            }
        }

        pma_record = FirmwareAddressIncrementLow16(FW_HUGE_UNKNOWN_2BB03B, (u16)((u16)track_number * 10u));
        if(FirmwareRead8(FirmwareAddressIncrementLow16(pma_record, 2u)) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 0u);
        }
        else if(FirmwareRead8(FirmwareAddressIncrementLow16(pma_record, 3u)) == 0xffu && FirmwareRead8(FirmwareAddressIncrementLow16(pma_record, 4u)) == 0xffu
                && FirmwareRead8(FirmwareAddressIncrementLow16(pma_record, 5u)) == 0xffu)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 0u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u && ProbeAndClassifyWorkingTrackBeforeStart() == 0u)
            return;
        UpdateTrackTableFromWorkingTrack();
        track_number++;
    }
}

/* RAM:fbd0c8 */
void PLEXTOR_NEAR_FUNCTION InitializeWorkingTrackFromReadPipelineState(void)
{
    u8 selected_track;
    u16 selected_offset;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, 1u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E));
    selected_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E);
    selected_offset = (u16)((u16)selected_track * 19u);
    FirmwareWriteNear8((u16)(0xdd87u + selected_offset), 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) > 99u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, FirmwareReadNear8((u16)(0xdd87u + selected_offset)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, selected_track);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD39, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD7A));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD39) + 150ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, (u8)((FirmwareReadNear8((u16)(0xdd89u + selected_offset)) >> 1) & 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, FirmwareReadNear8((u16)(0xdd92u + selected_offset)));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u || ClassifyAndInitializeWorkingTrackFromRawQProbe() != 0u)
        UpdateTrackTableFromWorkingTrack();
}

/* RAM:fcce39 */
void ResetType2ControllerRecordPipelineState(void)
{
    u8 slot_index;
    u32 reserved_region_base;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C9F, 0u);
    FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 0u);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_0C96, 0ul);
    ClearType2ControllerRecordWorkingState();
    ClearTwoElevenByteRuntimeStateRecords();

    reserved_region_base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    FillControllerBufferRange(reserved_region_base, FW_HUGE_UNKNOWN_1B0000 - reserved_region_base, 0u);
    for(slot_index = 0u; slot_index < 8u; ++slot_index)
    {
        u16 slot_offset = (u16)((u16)slot_index * 5u);

        WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_FB04 + slot_offset, 0ul);
        WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_FB2C + slot_offset, 0ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB03 + slot_offset, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB2B + slot_offset, 0u);
    }

    DispatchInitializeType2ControllerRecordGeometryForReset();
    InitializeControllerRecordBoundaryTableFromDefaults();
    WriteBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BA59F, 0ul);
    WriteBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BA5AB, 0ul);
    WriteBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BA5A7, 0ul);
    WriteBigEndianU32FarLowWordFirst(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION, 0ul);
    WriteBigEndianU32FarLowWordFirst(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY, 0ul);
    WriteBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BA5A3, 0ul);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_SCAN_POSITION_SECOND, 0ul);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_SCAN_POSITION_FIRST, 0ul);
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0u)
        FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 60u);
    else
        FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 20u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5D8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 0u);
}

/* RAM:fec9a7 */
u8 PrepareWritePipelineControllerRecordState(void)
{
    u32 first_position;
    u32 second_position;

    FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 0u);
    DispatchAcquireRequiredRecordRunsForWritePrepare(3u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x7fu));

    first_position = ReadBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BA589);
    second_position = ReadBigEndianU32FarLowWordFirst(FW_HUGE_UNKNOWN_2BA59F);
    if(second_position < first_position)
    {
        FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 1u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1587, 1u);
        FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 3u);
    }

    DispatchRecalculateType2GeometryForWritePrepare();
    DispatchBuildType2LeadInForWritePrepare(0u, 2u);
    if(DispatchPreparePrimaryType2ForWritePrepare(1u) == 0u)
        return 0u;
    if(DispatchAcquireType2BaseWindowForWritePrepare() == 0u)
        return 0u;
    if(DispatchPrepareSecondaryType2ForWritePrepare(1u) == 0u)
        return 0u;
    if(DispatchServiceFirstType2ForWritePrepare() == 0u)
        return 0u;
    if(DispatchAcquireType2WindowForWritePrepare() == 0u)
        return 0u;
    if(DispatchInitializeRawQForWritePrepare() == 0u)
        return 0u;
    DispatchBuildType2LeadInForWritePrepare(0u, 9u);
    if(DispatchAcquireType2LeadInForWritePrepare() == 0u)
        return 0u;
    if(DispatchAcquireType2LeadOutForWritePrepare() == 0u)
        return 0u;

    DispatchBuildActiveBoundaryForWritePrepare();
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfeu));
    FirmwareWrite8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH, 1u);
    return 1u;
}

/* RAM:fbe692 */
void InitializeVendorF3ReadAcquisitionMeasurementState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_0C8E, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235));
    DispatchWaitForReadAcquisitionForVendorF3Measurement();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xdfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10A2, 0u);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_10A4, 0ul);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_10A8, 0ul);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_10AC, 0u);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_10AE, 0ul);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_10B2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) | 2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 0x80u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfbu) | ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) & 1u) << 2)));
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) >> 2) & 1u) == 1u)
        WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_1099, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0E) + 150ul);
    SetNoSenseData();
}

/* RAM:fbe5e5 */
void InitializeVendorF3AlternateReadAcquisitionState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_0C8E, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235));
    DispatchWaitForReadAcquisitionForVendorF3Measurement();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1082, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1080, 0u);
    WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_1083, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1095, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1093, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1091, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_108F, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_108D, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_108B, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1089, 0u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1087, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1098, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1098) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 0x80u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfdu) | ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) & 1u) << 1)));
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) >> 1) & 1u) == 1u)
        WriteBigEndianU32FarLowWordFirst(FW_NEAR_UNKNOWN_105E, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0E) + 150ul);
    SetNoSenseData();
}

/* RAM:fbe4d7 */
void ResetControllerAndCompleteVendorF3Command(void)
{
    u8 controller_shadow;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
    DispatchWriteControllerParametersE8E9ForVendorF3Reset(0u);
    DispatchWriteControllerParameterEFForVendorF3Reset(0u);
    DispatchSetControllerFlagForVendorF3Reset(0u);
    ClearBit0547Atomic();
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180B9, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_0180BD, 1u);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, controller_shadow);
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetSfrA9Bit2Atomic();
    ClearP3Bit0Atomic();
    ClearP3Bit1Atomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
    SetP3Bit0Atomic();
    ClearP3Bit1Atomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0xcfu));
    SetNoSenseData();
}

/* RAM:fe9e64 */
/* Incoming R15 is made explicit as fallback_class for write types other than
 * zero, one, and two. */
u8 PLEXTOR_NEAR_FUNCTION SelectRawReadSectorClass(u8 prefer_working_track_class, u8 fallback_class)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 0u || write_type == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) < 8u)
            return 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B3) == 0u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) == 1u)
        {
            if(prefer_working_track_class == 1u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) != 0u)
                    return FirmwareReadNear8(FW_NEAR_UNKNOWN_DD32);
                return DispatchSelectCurrentRawReadSectorClass();
            }
        }
        return DispatchSelectCurrentRawReadSectorClass();
    }
    if(write_type == 2u)
        return DispatchFindAdjacentRawReadQRecordClass(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_149B), 0u);
    return fallback_class;
}

/* RAM:fcafe7 */
void HandleScsiOpcodeEC(void)
{
    u8 selector = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1fu);
    u16 allocation_length = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B12);
    u16 response_length;
    u16 transfer_length;

    if(selector == 0u)
        response_length = DispatchBuildVendorEcRecordListResponse((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) & 1u), 0u);
    else if(selector == 1u)
        response_length = DispatchBuildVendorEcRecordListResponse((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) & 1u), 1u);
    else
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    DispatchCopyVendorEcResponseToControllerBuffer(FW_NEAR_UNKNOWN_0B2A, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A), response_length);
    transfer_length = response_length < allocation_length ? response_length : allocation_length;
    if(DispatchExecuteVendorEcDataIn(transfer_length) != 4u)
        SetNoSenseData();
}

/* RAM:fcbdd9 */
u8 PrepareWriteCommandState(u32 start_address)
{
    u8 mode;

    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    mode = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);
    if(mode < 1u || mode > 3u)
        return 0u;
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 3) & 1u) != 1u)
        DispatchInitializeWriteDefaultsForPrepareWriteCommand();
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) != 0u && (s32)start_address < 32l)
        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfeu));
    return 1u;
}

/* RAM:fd3a38 */
s32 MapNegativeLogicalBlockAddressThroughThirtyTwoPlusSevenLayout(s32 logical_block_address)
{
    s32 biased_address = (s32)((u32)logical_block_address + 150ul);
    u32 magnitude;
    u32 quotient;
    u32 remainder;
    u32 adjustment;

    if(FirmwareReadBit000B() == 0u)
    {
        FirmwareWriteBit000C(0u);
        return biased_address;
    }
    if(logical_block_address >= 0l && logical_block_address < 150l)
    {
        FirmwareWriteBit000C(0u);
        return logical_block_address;
    }

    FirmwareWriteBit000C(1u);
    if(biased_address >= -32l)
    {
        adjustment = 0xfffffffeul;
    }
    else
    {
        magnitude = 0ul - (u32)biased_address;
        quotient = DivideU32QuotientWithoutRuntimeLibrary(magnitude, 32ul);
        remainder = magnitude - (quotient * 32ul);
        if(remainder == 0ul)
            adjustment = 0ul - (MultiplyU32Low(7ul, quotient) - 5ul);
        else
            adjustment = 0ul - (MultiplyU32Low(7ul, quotient) + 2ul);
    }
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0F0C, adjustment);
    return (s32)((u32)biased_address + adjustment);
}

/* RAM:fd3ab6 */
s32 MapLogicalBlockAddressThroughAlternateRecordLayout(s32 logical_block_address)
{
    s32 group_quotient;
    s32 block_quotient;
    u16 group_adjustment;
    u32 adjustment;

    if(FirmwareReadBit000B() == 0u)
    {
        FirmwareWriteBit000C(0u);
        return (s32)((u32)logical_block_address + 150ul);
    }

    FirmwareWriteBit000C(1u);
    group_quotient = DivideS32ByU16TruncatingTowardZero(logical_block_address, 0x1100u);
    group_adjustment = (u16)(((u16)((u16)(u8)(group_quotient + 1l) * 8u) + 0x20u) * 0x27u);
    block_quotient = DivideS32ByU16TruncatingTowardZero(logical_block_address, 0x20u);
    adjustment = MultiplyU32Low(7ul, (u32)block_quotient) + (u32)group_adjustment;
    WriteBigEndianU32Far(FW_NEAR_UNKNOWN_0F0C, adjustment);
    return (s32)((u32)logical_block_address + adjustment + 150ul);
}

/* RAM:fe7cf2 */
u8 PLEXTOR_NEAR_FUNCTION FinalizeRawReadTransferBatchDescriptors(fw_addr_t controller_buffer_address, u16 track_record_index, u8 completed_frame_count)
{
    u16 transfer_count = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1463);
    u16 frame_index;
    u16 record_offset = (u16)(track_record_index << 3);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u && (FirmwareReadNear8((u16)(0xb4bdu + record_offset)) & 0x40u) == 0x40u && FirmwareReadNear8((u16)(0xb4bfu + record_offset)) == 0u)
        DispatchPopulateRawSectorHeaderAndTdiPayloadRangeForFinalize(controller_buffer_address, (u32)transfer_count);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) == 0u)
        for(frame_index = completed_frame_count; frame_index < transfer_count; frame_index++)
            DispatchEnqueueRawReadDescriptorAndAdvanceQStateForFinalize(track_record_index);
    else if(DispatchAcquireRawReadFrameBatchWithInterleavedQForFinalize((u32)transfer_count) == 0u)
        return 0u;
    return 1u;
}

/* RAM:fe7d5a */
void PLEXTOR_NEAR_FUNCTION EnqueueRawReadDescriptorAndAdvanceQStateForIndex(u16 descriptor_index)
{
    u8 direction = 1u;
    u16 record_offset;
    u32 target_address;
    u32 main_data_cursor;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        record_offset = (u16)(descriptor_index << 3);
        direction = FirmwareReadNear8((u16)(0xb4bfu + record_offset));
    }

    if(direction == 0u)
    {
        DispatchUpdateQPacketForIndexedDescriptor(1u, 0u);
        DispatchEnqueueCurrentDescriptorForIndexedDescriptor();
        DispatchDecrementMsfForIndexedDescriptor(FW_NEAR_UNKNOWN_1275);
    }
    else
    {
        DispatchUpdateQPacketForIndexedDescriptor(0u, 0u);
        DispatchEnqueueCurrentDescriptorForIndexedDescriptor();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1030) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1033, 1u);
        DispatchIncrementMsfForIndexedDescriptor(FW_NEAR_UNKNOWN_1275);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1033, 0u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1030) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1033, 1u);
    DispatchIncrementMsfForIndexedDescriptor(FW_NEAR_UNKNOWN_1279);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1033, 0u);

    target_address = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, target_address + 1ul);
    main_data_cursor = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_126E);
    DispatchAdvanceRingForIndexedDescriptor(main_data_cursor);
}

/* RAM:f892fd */
u8 RunControllerReadPathReadinessProbe(void)
{
    u8 attempt;
    u8 ready = 0u;

    DispatchWriteControllerReadinessParameters();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u || FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
        DispatchWaitControllerPortReadyForReadiness(FW_CONTROLLER_UNKNOWN_0180D0, 0u);
    SetExternalInterrupt1EnabledAtomic(0u);

    for(attempt = 0u; attempt < 3u; attempt++)
    {
        if(FirmwareReadBit0031() == 0u)
            ready = DispatchWaitForControllerState1302Bit5ForReadiness();
        else
            ready = DispatchProbeControllerReadPathAtCurrentFrame();
        if(ready == 1u)
            break;
        DispatchRestoreControllerReadPathForReadiness(0u);
    }
    if(ready != 1u)
    {
        DispatchRestoreControllerReadPathForReadiness(1u);
        ready = 0u;
    }
    if(ready == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        DispatchSubmitControllerWord0204ForReadiness(1u);
    return ready;
}

/* RAM:f89515 */
void RestoreControllerReadPathConfiguration(u8 refresh_derived_parameters)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    }
    if(FirmwareReadBit0031() != 0u)
    {
        DispatchStrobeControllerPortForReadPathRestore(FW_CONTROLLER_UNKNOWN_0180C0, 0u);
        DispatchStrobeControllerPortForReadPathRestore(FW_CONTROLLER_UNKNOWN_0180CF, FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) == 2u ? 0xc4u : 0xc0u);
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802A, 3u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
    DispatchStrobeControllerPortForReadPathRestore(FW_CONTROLLER_UNKNOWN_018006, 0xc0u);
    DispatchWriteControllerParameterD6ForReadPathRestore();

    if(refresh_derived_parameters == 1u)
    {
        DispatchRefreshControllerParametersForReadPathRestore();
        DispatchConfigureControllerPortsForReadPathRestore();
        DispatchSubmitControllerWord0204ForReadPathRestore(0u);
    }
}

/* RAM:f8949e */
u8 PLEXTOR_NEAR_FUNCTION WaitForControllerState1302Bit5WithTimeout(void)
{
    u8 controller_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD);
    u16 duration_units;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xdfu));
    controller_state |= 2u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_state);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802A, 0xfbu);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x40u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1217, 18u);

    for(;;)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1217) < 0)
        {
            DispatchRestoreControllerReadPathForState1302Wait(0u);
            return 0u;
        }
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 5) & 1u) == 1u)
            break;
        YieldCoroutine1();
    }

    duration_units = (u16)((s16)4000 / (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    DispatchWaitTimer1ScaledDurationForState1302Wait(duration_units, 1u);
    DispatchStrobeControllerPortForState1302Wait(FW_CONTROLLER_UNKNOWN_018006, 0xc8u);
    DispatchWriteControllerParameterD6ForState1302Wait();
    return 1u;
}

/* RAM:fb0ee1 */
void PLEXTOR_NEAR_FUNCTION ConfigureControllerOperatingCode4AndChannel3(void)
{
    u8 disable_operating_path;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 4u);
    DispatchSetP1Bits0And1ForOperatingCode4();

    disable_operating_path = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x10u) != 0u
                                  || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 0x04u) != 0u && ReadBit001cForReadTargetAdjustment() != 1u && ReadBit001dForReadTargetAdjustment() != 1u));
    if(disable_operating_path != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 0u);
        DispatchStrobeControllerPortForOperatingCode4(FW_CONTROLLER_UNKNOWN_018006, 0x80u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1006, 1u);
        DispatchStrobeControllerPortForOperatingCode4(FW_CONTROLLER_UNKNOWN_018006, 0xc0u);
        DispatchConfigureControllerThresholdForOperatingCode4(0x96ul);
    }

    DispatchSetP1Bits0And1ForOperatingCode4();
    DispatchPulseControllerWord0253ForOperatingCode4(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 1u ? 400u : 40u);
    DispatchConfigureControllerPort8006AndChannel3ForOperatingCode4(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006));
}

/* RAM:f86ab6 */
void PLEXTOR_NEAR_FUNCTION AttemptMediaState3QCaptureAndControllerReinitialization(void)
{
    u8 pulse_failure_count;

#define PLEXTOR_BINDING_SECTION 93
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1217, 50u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1217) >= 0)
    {
        DispatchRearmControllerQPacketCaptureForState3();
        pulse_failure_count = 0u;
        while(DispatchProcessCapturedControllerQPacketForState3() == 0xfeu)
        {
            DispatchYieldCoroutine1ForState3QCapture();
            if(DispatchCheckP1Cex0PulseForState3QCapture() == 0u)
                ++pulse_failure_count;
            if(pulse_failure_count >= 100u)
                return;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
                return;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) != 0u)
        {
            DispatchResetDriveControlForState3QCapture();
            if(DispatchInitializeControllerForState3QCapture() != 0u)
                return;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, 7u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            return;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
            return;
    }
}

/* RAM:f87057 */
void PLEXTOR_NEAR_FUNCTION InitializeMediaState5ControllerPath(void)
{
    u8 reduce_write_speed;

    DispatchInitializeMediaState5PositionController();
    DispatchSnapshotMediaState5MeasurementTarget();
    DispatchApplyMediaState5CalibrationOffset();

    if((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 1u) != 0u && (ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 2u) != 0u)
        reduce_write_speed = 1u;
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        reduce_write_speed = 1u;
    else
        reduce_write_speed = (u8)((ReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAD7) & 0x1000u) != 0u);
    (void)DispatchTryReduceMediaState5WriteSpeed(reduce_write_speed);
    DispatchSetMediaState5ControllerEnable(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCD, DispatchRunMediaState5ControllerCommand06(0x10u));
    DispatchRecomputeMediaState5ControllerThreshold();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) & 1u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 0x20u));
}

/* RAM:fc3150 */
void PLEXTOR_NEAR_FUNCTION UpdateModePage1ARuntimeImageForCurrentMediaState(void)
{
    u32 event_descriptor = BuildMediaEventStatusDescriptor();

    if((u8)(event_descriptor >> 16) == 1u)
    {
        s32 scaled_countdown;

        if(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB4) == 0ul)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6A2, 0u);
            scaled_countdown = 0l;
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E6A2, 2u);
            scaled_countdown = (s32)(ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0FB0) * 2ul) / 100l;
            if(scaled_countdown == 0l)
                scaled_countdown = 1l;
        }
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_E6A3, (u32)scaled_countdown);
    }
    else
    {
        CopyBytes8(FW_NEAR_UNKNOWN_E69F, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8B96), 12u);
    }
}

/* RAM:fde3ac */
void LoadReadAcquisitionTargetForTrackNumber(void)
{
    fw_addr_t tuple_address;
    u8 track_number;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u)
    {
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E42);
        tuple_address = FW_NEAR_UNKNOWN_F2B9 + (u16)((u16)track_number * 6u);
    }
    else
    {
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E42);
        tuple_address = FW_HUGE_UNKNOWN_2BB042 + (u16)((u16)track_number * 10u);
    }

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0E3B, (u32)ConvertBinaryMsfToAdjustedFramePosition(tuple_address));
}

/* RAM:fa6bce */
void ApplyControllerOperatingParametersForState2Path(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 1u) == 0u)
        return;

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) >> 1) & 7u) == 5u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) & 8u) == 0u && ReadBit0033Atomic() != 0u)
    {
        u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

        if(operating_code == 4u)
            WaitTimer1ScaledDuration(12u, 0u);
        else if(operating_code == 8u)
            WaitTimer1ScaledDuration(3u, 0u);
        else if(operating_code == 10u)
            WaitTimer1ScaledDuration(2u, 0u);
        else if(operating_code == 2u)
            WaitTimer1ScaledDuration(29u, 0u);
    }

    WriteControllerParametersE8E9ForValue(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));
    WriteControllerParameterEF00AndConditionalD0Value(0u);
    ClearControllerParameterEEShadow();
    WriteControllerParametersECEDFromScaledValue(0u);
}

/* RAM:fa6c47 */
void ApplyControllerOperatingParametersForState1Path(void)
{
    u8 operating_code;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u)
        return;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 6u)
        return;

    if(ReadBit0033Atomic() != 0u)
    {
        operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
        if(operating_code == 1u)
            WaitTimer1ScaledDuration(0x0212u, 0u);
        else if(operating_code == 4u)
            WaitTimer1ScaledDuration(10u, 0u);
        else if(operating_code == 8u)
            WaitTimer1ScaledDuration(2u, 0u);
        else if(operating_code == 12u || operating_code == 16u || operating_code == 20u || operating_code == 24u || operating_code == 32u || operating_code == 40u || operating_code == 52u)
            WaitTimer1ScaledDuration(0u, 0u);
    }

    WriteControllerParameterEF00AndConditionalD0Value(0u);
    ClearControllerParameterEEShadow();
    WriteControllerParametersECEDFromScaledValue(0u);
    WriteControllerParametersE8E9ForValue(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));
}

/* RAM:fb0620 */
void PLEXTOR_NEAR_FUNCTION ClearP1Bits0And1AndSubmitControllerWord0252(void)
{
    ClearP1Bits0And1Atomic();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, 0u);
    SubmitControllerWordPair(0u, 0x0252u);
}

/* RAM:fb0600 */
void PLEXTOR_NEAR_FUNCTION ClearControllerWord0252AndResetChannel2(void)
{
    SelectP1Bit0OnlyAtomic();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, 0u);
    SubmitControllerWordPair(0u, 0x0252u);
    ResetControllerChannel2();
}

/* RAM:fb059a */
void PLEXTOR_NEAR_FUNCTION SubmitControllerWord0252WithP1Bit0Set(u16 controller_value)
{
    SelectP1Bit0OnlyAtomic();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, 0u);
    SubmitControllerWordPair(controller_value, 0x0252u);
}

/* RAM:fb05bc */
void PLEXTOR_NEAR_FUNCTION SubmitSelectedControllerWord0252WithP1Bit0Set(u8 select_value_006f)
{
    SelectP1Bit0OnlyAtomic();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, 0u);
    SubmitControllerWordPair(select_value_006f != 0u ? 0x6f00u : 0x4800u, 0x0252u);
}

/* RAM:fb8017 */
void ResetControllerDataPathRegisters(void)
{
    DispatchClearControllerDataPathBits31And33();
#define PLEXTOR_BINDING_SECTION 94
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:fb7efd */
void InitializeReadMeasurementControllerPhase(void)
{
    u8 controller_shadow;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_154D, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 2u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
    DispatchClearReadMeasurementPhaseBits2BAnd43();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_14DE, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_1235));

    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xefu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_shadow);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) & 0x1fu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018018, controller_shadow);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFF) & 0xefu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, controller_shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, controller_shadow);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB9F, 2u);
}

/* RAM:fbf133 */
void TransferFixedResponse52Bytes(void)
{
    u16 transfer_count = MinU16(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B12), 0x52u);

    if(DispatchCopyFixedResponse52AndExecuteDataIn(FW_NEAR_UNKNOWN_0B2A, (u32)transfer_count) != 4u)
        DispatchSetNoSenseForFixedResponse52();
}

/* RAM:fbb28a */
void PLEXTOR_NEAR_FUNCTION ClearDecodedTocPointSummariesFromLastTrack(void)
{
    ClearDecodedTocPointSummariesFromIndex((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1B) + 1u));
}

/* RAM:fea65a */
u8 ExecutePendingRawReadPreparationRange(u8 five_frame_mode)
{
    s32 frame_count = (s32)ReadBigEndianU32Far(FW_NEAR_UNKNOWN_1430);
    u8 write_type;

    if(frame_count == 0)
        return 1u;
    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type != 0u && write_type != 1u)
        return 1u;

    if(five_frame_mode == 1u)
    {
        if(frame_count < 5l)
            return (u8)(DispatchAcquireRawReadFrameSequenceWithCurrentQForPendingRange(frame_count, 5u) != 0u);
        if(DispatchAcquireFixedRawReadFramePrefixForPendingRange(0u, 5u) == 0u)
            return 0u;
        return (u8)(DispatchAcquireRawReadFrameSequenceWithCurrentQForPendingRange(frame_count - 5l, 5u) != 0u);
    }
    return (u8)(DispatchAcquireRawReadFrameSequenceWithQUpdatesForPendingRange(frame_count, 1u) != 0u);
}

/* RAM:fe1370 */
u8 AcquireRawReadFramesForWorkingTrack(s32 frame_count)
{
    u8 control_flags;

    if(frame_count <= 0l)
        return 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
    control_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, control_flags);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, DispatchBinaryByteToPackedBcdForWorkingTrackAcquisition(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35)));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u)
    {
        if(frame_count >= 5l)
        {
            if(DispatchAcquireFixedRawReadFramePrefixForWorkingTrack(0u, 9u) == 0u)
            {
                DispatchSetSenseDataForWorkingTrackAcquisition(3u, 0x0c07u);
                DispatchSetSenseInformationForWorkingTrackAcquisition(ReadBigEndianU32Far(FW_NEAR_UNKNOWN_1598));
                return 0u;
            }
            frame_count -= 5l;
            if(frame_count == 0l)
                return 1u;
        }
        return (u8)(DispatchAcquireRawReadFrameSequenceWithCurrentQForWorkingTrack(frame_count, 4u) != 0u);
    }

    return (u8)(DispatchAcquireRawReadFrameSequenceWithQUpdatesForWorkingTrack(frame_count, 0u) != 0u);
}

/* RAM:fe0246 */
u8 PLEXTOR_NEAR_FUNCTION CaptureRawSectorUsingQMsfAndPopulateSubheader(fw_addr_t q_msf_address, fw_addr_t controller_buffer_address, u8 source_kind)
{
    if(DispatchWaitForProducerCapacityForQMsfSectorCapture() == 0u)
    {
        DispatchLatchFailureContextForQMsfSectorCapture();
        return 0u;
    }

    DispatchWriteControllerBlock25ForQMsfSectorCapture(q_msf_address);
    if(DispatchCaptureRawSectorForQMsfSectorCapture(controller_buffer_address, source_kind) == 0u)
    {
        DispatchSetSenseDataForQMsfSectorCapture(4u, 0x4400u);
        DispatchSetSenseInformationForQMsfSectorCapture(ReadBigEndianU32Far(FW_NEAR_UNKNOWN_1598));
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1279, FirmwareRead8(q_msf_address));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127A, FirmwareRead8(FirmwareAddressIncrementLow16(q_msf_address, 1u)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127B, FirmwareRead8(FirmwareAddressIncrementLow16(q_msf_address, 2u)));
    DispatchPopulateMode2SubheaderForQMsfSectorCapture(controller_buffer_address, source_kind);
    return 1u;
}

/* RAM:fd7de6 */
u8 HandleIndexedState0C(void)
{
    u8 lead_in_last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
    u8 recognized_layout = (u8)(lead_in_last_track == 0x0fu || lead_in_last_track == 0x10u || lead_in_last_track == 0x18u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 3u && recognized_layout != 0u && FirmwareReadBit000E() == 0u && DispatchProcessIndexedStateGroupForState0C(1u) == 0u)
        return 1u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x20u) == 0u)
        return 0u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xdfu));
    if(recognized_layout != 0u)
        (void)DispatchProcessIndexedStateGroupForState0C(0u);
    return 1u;
}

/* RAM:fd44d8 */
void HandlePauseResumeCommand(void)
{
    if(DispatchValidatePauseResumeCdb(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D37)) == 0u)
    {
        DispatchHandleUnsupportedPauseResumeCommand();
        return;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 4u) == 0u)
    {
        DispatchSetPauseResumeCommandSequenceError();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 4u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B12) & 1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8D, 1u);
        DispatchWaitForPauseReadAcquisition();
    }
    else if(FirmwareReadBit000E() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8D, 0u);
        DispatchClearPauseResumeBit0018();
        DispatchStartPauseResumeFormat2Read((s32)ReadBigEndianU32Far(FW_NEAR_UNKNOWN_0F32));
    }
    else
    {
        DispatchSetPauseResumeNoSense();
    }
}

/* RAM:fcea2d */
void UpdateReadAcquisitionMeasurementIfNeeded(void)
{
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1307) >> 1) & 1u) == 0u && EvaluatePulseThresholdForReadAcquisitionMeasurement() == 1u)
        DispatchUpdateReadAcquisitionMeasurementFrom8Samples();
}

/* RAM:f9b3db */
void HandleReadHeaderCommand(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DBD)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B02, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0C));
    ExecuteReadHeaderSectorAcquisition();
}

/* RAM:fb23a3 */
void HandleSeek6Command(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DF3)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    PrepareAndDispatchReadOrSeekAcquisitionAtLbaForSeek6(ReadBigEndianU32Near(FW_NEAR_HOST_COMMAND_OPCODE) & 0x001ffffful);
}

/* RAM:fb24ff */
void HandleModeSense6Command(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CC2)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    ExecuteModeSense6ResponseFromC((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E));
}

/* RAM:f9b435 */
void HandleModeSense10Command(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CC9)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    ExecuteModeSense10ResponseFromC(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0B11));
}

/* RAM:fb471a */
void ServicePendingHostCommandWhenOperationFlagsPermit(void)
{
#define PLEXTOR_BINDING_SECTION 95
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if(FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) != 1u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) != 0u || ((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) >> 1) & 1u) != 0u))
        ServicePendingHostCommandDuringMediaOperation();
}

/* RAM:fc0758 */
void HandleVerify12Command(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E25)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B02, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B0C));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B06, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0B10));
    PrepareReadVerifyCommandState();
    ExecuteReadVerifyRange();
    FinalizeReadVerifyCommandState();
}

/* RAM:fb8be5 */
void ResetAllPmaQRecordTables(void)
{
    DispatchResetPmaQRecordTablesFromTrackIndex(0u);
}

/* RAM:fbda43 */
void PLEXTOR_NEAR_FUNCTION BuildVendorF2ControllerMeasurementResponse(void)
{
    BuildVendorF2ControllerMeasurementResponseBody();
}

/* RAM:fcd2da */
u8 ProbeControllerRecordAtFrameAndMapStatus(s32 frame_position)
{
    u8 result;

    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, (u32)frame_position);
    result = DispatchClassifyRawSectorPayloadSignatureAtPipelineTarget();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        result = 5u;
    return result;
}

/* RAM:fcdb84 */
/* The native body retries the same selected frame, not adjacent frames. */
u8 PLEXTOR_NEAR_FUNCTION ProbeType2TdiRecordNearTrackStart(u8 use_fixed_lead_in_offset)
{
    s32 probe_offset = use_fixed_lead_in_offset == 1u ? -214l : (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FB04);
    u8 attempt;

    for(attempt = 0u; attempt < 4u; ++attempt)
    {
        s32 track_start = (s32)ReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD3D);
        u8 status = DispatchProbeControllerRecordForType2Tdi((s32)((u32)track_start + (u32)probe_offset));

        if(status == 5u)
            return 0u;
        if(status == 0xfeu || status == 0xffu || status == 0x0fu)
            continue;
        if((FirmwareRead8(FW_HUGE_UNKNOWN_2BB91C) >> 4) != 0u)
            continue;
        if((FirmwareRead8(FW_HUGE_UNKNOWN_2BB91C) & 0x0fu) != 0u)
            continue;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C9F, (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_2BB927) & 0x01u));
        return 1u;
    }
    return 0u;
}

/* RAM:fb094b */
u8 ResetDriveControlAndInitializeControllerCalibration(void)
{
    u8 result;

#define PLEXTOR_BINDING_SECTION 96
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    ResetDriveControlStateToEight();
    result = InitializeControllerOperatingStateAndRunCalibration();
    if(result == 0u)
        ResetDriveControlStateToEight();
    return result != 0u ? 1u : 0u;
}

/* RAM:fa4375 */
u8 AcquireType2ControllerRecordWindowSequence(void)
{
    BuildType2LeadInTocRecordSequence(0u);
    if(PreparePrimaryType2ControllerRecordSlots(1u) == 0u)
        return 0u;
    if(AcquireFortyFrameType2ControllerRecordBaseWindow() == 0u)
        return 0u;
    if(PrepareSecondaryType2ControllerRecordSlots(1u) == 0u)
        return 0u;
    if(ServiceFirstPendingType2ControllerRecord() == 0u)
        return 0u;
    return AcquireFortyFrameType2ControllerRecordWindow();
}

/* RAM:fa7baf */
void ServicePeriodicControllerD8D9DifferenceSampling(void)
{
    u16 sample_counter;

    if(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_F03D) < 15u)
        DispatchCaptureControllerD8D9NormalizedDifferenceSample();
    sample_counter = (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_F03D) + 1u);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_F03D, sample_counter);
    if(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_F03D) >= 150u)
        WriteBigEndianU16Near(FW_NEAR_UNKNOWN_F03D, 0u);
}

/* RAM:fb0222 */
u8 RunDriveControlState9SequenceWrapper(void)
{
    return DISPATCH_RUN_DRIVE_CONTROL_STATE9_SEQUENCE();
}

/* RAM:fdc90c */
u8 RunControllerBufferMemorySelfTest(void)
{
    return DispatchTestControllerBufferMemoryAddressingForSelfTest() != 0u ? 1u : 0u;
}

/* RAM:fbf30b */
u8 TestControllerBufferMemoryAddressing(void)
{
    fw_addr_t base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);

    DispatchWriteControllerBufferByteForAddressingTest(FirmwareAddressIncrement24(base, 0x500ul), 0xabu);
    base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    DispatchWriteControllerBufferByteForAddressingTest(FirmwareAddressIncrement24(base, 0x700ul), 0xcdu);
    base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    if(DispatchReadControllerBufferByteForAddressingTest(FirmwareAddressIncrement24(base, 0x500ul)) != 0xabu)
        return 0u;

    base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    DispatchWriteControllerBufferByteForAddressingTest(base, 0x55u);
    base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    if(DispatchReadControllerBufferByteForAddressingTest(base) != 0x55u)
        return 0u;

    base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    DispatchWriteControllerBufferByteForAddressingTest(FirmwareAddressIncrement24(base, 0x20000ul), 0xaau);
    base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    if(DispatchReadControllerBufferByteForAddressingTest(base) != 0x55u)
        return 0u;
    base = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A);
    if(DispatchReadControllerBufferByteForAddressingTest(FirmwareAddressIncrement24(base, 0x20000ul)) != 0xaau)
        return 0u;
    if(DispatchControllerBufferBankStridePatternForAddressingTest(0x55u) == 0u)
        return 0u;
    return DispatchControllerBufferBankStridePatternForAddressingTest(0xaau) != 0u ? 1u : 0u;
}

/* RAM:f8f773 */
void PublishReadSetupFailureCoroutine1(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 6u);
    SignalDriveStatusPatternForever();
}

/* RAM:f8f07c */
void ClearStartupStateAndSignalFailureCoroutine1(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1309, 0u);
    WriteControllerParameterD6Value7E();
    SignalDriveStatusPatternForever();
}

/* RAM:fd10df */
void HandleSfrE1Bit7Interrupt(void)
{
    ClearBit070FAtomic();
    if(FirmwareReadBit002B() != 0u)
        ProcessSfrDaDbMeasurementSample();
}

/* RAM:fbb3ed */
void PLEXTOR_NEAR_FUNCTION RecalculateRadialStepScaleAndDerivedState(void)
{
    u16 radial_calibration_span = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_0CF9);
    u16 default_radial_step_scale = (u16)(((u16)FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F848FE)) << 8) | FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F848FF)));
    u32 product = (u32)default_radial_step_scale * (u32)radial_calibration_span;
    u16 radial_step_scale = (u16)DivideU32QuotientWithoutRuntimeLibrary(product, 0x0640ul);

    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1000, radial_step_scale);
    RebuildRadialPositionTableAndThresholds();
}

/* RAM:f9826a */
/* The native caller supplies indices one through thirteen.  Preserve both the
 * access order and the second volatile read of the selected word. */
u8 ValidateControllerMeasurementSeries02SelectionIndex(u8 series_index)
{
    u16 byte_offset = (u16)((u16)series_index * 2u);
    u16 selected = ReadBigEndianU16Near((u16)(0xefc5u + byte_offset));
    u16 previous = ReadBigEndianU16Near((u16)(0xefc3u + byte_offset));
    u16 next;

    if(previous >= selected)
        return 0u;

    next = ReadBigEndianU16Near((u16)(0xefc7u + byte_offset));
    selected = ReadBigEndianU16Near((u16)(0xefc5u + byte_offset));
    return (u8)(selected < next);
}

/* RAM:fb4ef2 */
u8 PLEXTOR_NEAR_FUNCTION ValidateCloseSessionTrackTableState(void)
{
    u8 track_index;
    u8 high_index;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u)
    {
        u8 saved_high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC);

        high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
        if(saved_high_index == high_index)
        {
            SetSenseData(5u, 0x2c04u);
            return 0u;
        }
        return 1u;
    }

    track_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E);
    for(;;)
    {
        u8 record_state;

        high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
        if(track_index >= high_index)
            break;
        record_state = FirmwareRead8((u16)(0xdd87u + (u16)track_index * 19u));
        if(record_state == 3u || record_state == 2u)
        {
            SetSenseData(5u, 0x7204u);
            return 0u;
        }
        if(record_state == 5u || record_state == 6u || record_state == 7u)
        {
            SetSenseData(2u, 0x3004u);
            return 0u;
        }
        ++track_index;
    }

    high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    if(high_index != 100u && FirmwareReadNear8((u16)(0xdd87u + (u16)high_index * 19u)) != 0u)
    {
        SetCommandSequenceErrorSense();
        return 0u;
    }

    high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    if(high_index == FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
    {
        SetSenseData(5u, 0x2c04u);
        return 0u;
    }
    return 1u;
}

/* RAM:fbb9c9 */
void PLEXTOR_NEAR_FUNCTION FinalizePostTocDriveState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC21, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
#define PLEXTOR_BINDING_SECTION 97
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        InitializeCoroutine0(FW_ENTRY_RUN_DRIVE_OPERATION_RESULT_COROUTINE0);
    }
}

/* RAM:fd0c88 */
void PLEXTOR_NEAR_FUNCTION ApplyFixedControllerParameterTableForOperatingCodeClass(void)
{
    u8 table_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 4u ? 0u : 5u;

    ApplyFixedControllerParameterTable(table_index);
}

/* RAM:fab8bb */
void RefreshTocStateFromRawReadQRecordTable(void)
{
#define PLEXTOR_BINDING_SECTION 98
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) == 0xc0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) >= 2u)
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_1143, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_14DE));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147, 1u);
        ParseRawReadQRecordTableIntoTocState();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x7fu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x20u));
        }
    }
}

/* RAM:faac4c */
void SubmitControllerBufferCommand40AndWaitCoroutine0(void)
{
    u8 request = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1038) | 2u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1038, request);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807E, request);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806F, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018070, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018071, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018072, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018073, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018074, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018075, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C1, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806F, 0x40u);
    while(FirmwareRead8(FW_HUGE_UNKNOWN_2BA5C1) == 0u)
        YieldCoroutine0();
}

/* RAM:fd9854 */
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForAcquisitionMode0(void)
{
    PrepareReadMeasurementForDescriptorMode();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B9) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
            ResetAndRebuildTocStateFromRawReadQRecords();
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
    {
        RefreshTocStateFromRawReadQRecordTable();
    }
}

/* RAM:fd9978 */
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForAcquisitionMode2(void)
{
    u8 descriptor_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D);

    switch(descriptor_mode)
    {
    case 0u:
        PREPARE_READ_MEASUREMENT_FOR_HIGH_DESCRIPTOR_MODE();
        break;
    case 1u:
        PrepareReadMeasurementForDescriptorMode();
        break;
    case 2u:
        PrepareReadMeasurementForDescriptorMode();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && FirmwareReadBit0032() == 0u)
        {
            AppendFullTocSessionFromRawReadQRecordTable();
            FUN_RAM_fbb2c3();
            FUN_RAM_fbb2c4();
        }
        break;
    case 3u:
        PrepareReadMeasurementForLowDescriptorMode();
        break;
    default:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 6u);
        break;
    }
}

/* RAM:fd9aba */
void PrepareReadMeasurementForLowDescriptorMode(void)
{
    u8 descriptor_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9E);
    u32 target;

    if(descriptor_mode == 2u)
    {
        target = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FBA2) - 4ul;
    }
    else if(descriptor_mode == 1u || descriptor_mode == 3u)
    {
        target = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FBA2) - 2ul;
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 7u);
        return;
    }
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_FBA6, target);
    PrepareReadMeasurementAtAdjustedTarget();
}

/* RAM:fd99bb */
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForAcquisitionMode3(void)
{
    PrepareReadMeasurementForDescriptorMode();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9D) == 2u && FirmwareReadBit0032() == 0u)
    {
        AppendFullTocSessionFromRawReadQRecordTable();
        FUN_RAM_fbb2c3();
        FUN_RAM_fbb2c4();
    }
}

/* RAM:fde056 */
u8 ExecuteControllerDataInAndMapStatus(u32 transfer_length)
{
    if(transfer_length == 0ul)
        return 5u;

    ExecuteControllerDataInTransferFromBufferOffset(transfer_length, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A));
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) == 13u ? 5u : 4u;
}

/* RAM:fde07a */
u8 ExecuteControllerDataOutAndMapStatus(u32 transfer_descriptor)
{
    u32 transfer_length = transfer_descriptor & 0x003ffffful;

    if(transfer_length == 0ul)
        return 5u;

    ExecuteControllerDataOutTransferFromBufferOffset(transfer_length, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_155A));
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) == 13u ? 5u : 4u;
}

/* RAM:fde8ab */
void HandleModeSelect6Command(void)
{
    u8 parameter_list_length;

    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CB0)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    parameter_list_length = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E);
    if(parameter_list_length == 0u)
        SetNoSenseData();
    else
        ProcessModeSelectParameterList(6u, (u16)parameter_list_length);
}

/* RAM:fd413c */
void HandlePlayAudioMsfCommand(void)
{
    if(ValidateScsiCdbFieldMasks(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D5A)) == 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }
    ValidateAndStartPlayAudioMsfRangeFromCdbTuplesForC();
}

/* RAM:fd7a7f */
u8 HandleIndexedState00(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x20u) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xdfu));
    AdvanceReadAcquisitionTrackCursor();
    DispatchLoadReadAcquisitionTargetForTrackNumber();
    return 1u;
}

/* RAM:fd7dbf */
u8 HandleIndexedState0A(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x20u) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xdfu));
    AdvanceReadAcquisitionTrackCursor();
    DispatchLoadReadAcquisitionTargetForTrackNumber();
    return 1u;
}

/* RAM:fd7a53 */
u8 ProcessIndexedStateCompletion(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1309) != 1u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x20u) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xdfu));
    AdvanceReadAcquisitionTrackCursor();
    DispatchLoadReadAcquisitionTargetForTrackNumber();
    return 1u;
}

/* RAM:fc947b */
u8 ApplyPendingReadAcquisitionTargetNudge(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFA) == 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFA) == 2u)
        MovePickupBy3DirectionTrueForPendingNudge();
    else
        MovePickupBy6DirectionTrueForPendingNudge();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFA, 0u);
    return 1u;
}

/* RAM:fc9a2d */
void InitializeControllerCalibrationMeasurementBaseline(void)
{
    WriteControllerParameterE2ValueAndRefreshEE(0x58u);
    WaitTimer1ScaledDuration(2000u, 1u);
    DisableGlobalInterrupts();
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FC05, MeasureAverageSfrDbSamples(32u));
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_FC05, MeasureAverageSfrDbSamples(32u));
    EnableFirmwareInterruptSources();
}

/* RAM:fc99d7 */
u8 CalibrateControllerParameterSet(void)
{
    ClearRxdatBit3Atomic();
    InitializeControllerCalibrationMeasurementBaseline();
    if(DispatchCalibrateControllerParameterForCode(0x04u) != 1u)
        return 0u;
    if(DispatchCalibrateControllerParameterForCode(0x00u) != 1u)
        return 0u;
    if(DispatchCalibrateControllerParameterForCode(0x1cu) != 1u)
        return 0u;
    if(DispatchCalibrateControllerParameterForCode(0x24u) != 1u)
        return 0u;
    if(DispatchCalibrateControllerParameterForCode(0x2cu) != 1u)
        return 0u;
    if(DispatchCalibrateControllerParameterForCode(0x3cu) != 1u)
        return 0u;
    return 1u;
}

/* RAM:fa66cd */
void SetDriveStateFlagsForIndexedMediaState(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 1u) != 1u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) | 0x80u));
}

/* RAM:f92c09 */
void HandleControllerEventBit3Completion(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11D9, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11D9) | 0x04u));
    if(FirmwareReadBit0014() != 0u)
    {
        u8 controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFE) & 0x1fu);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFE, controller_shadow);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_8018, controller_shadow);
        ClearBit0014Atomic();
    }
}

/* RAM:fa82fe */
void ApplyTimedOutputStateFromIndexBits(void)
{
    u8 state = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_F1B7) >> 2) & 3u);

    if(state == 1u)
    {
        ClearP3Bit0Atomic();
        ClearP3Bit1Atomic();
    }
    else if(state == 2u || state == 3u)
    {
        ClearP3Bit0Atomic();
        SetP3Bit1Atomic();
    }
}

/* RAM:fa8328 */
void SetTimedOutputLinesHigh(void)
{
    SetP3Bit0Atomic();
    SetP3Bit1Atomic();
}

/* RAM:faa8ab */
void WriteControllerBufferByte53AtOffsetA5DF(void)
{
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5DF, 0x53u);
}

/* RAM:faa8b9 */
void WriteControllerBufferByte53AtOffsetA62F(void)
{
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA62F, 0x53u);
}

/* RAM:f89467 */
u8 SampleControllerPort80C0Bit3WithHandshake(void)
{
    u8 sample;
    u8 controller_shadow;

    ClearBit0547Atomic();
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8002, controller_shadow);
    sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_80C0);
    controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, controller_shadow);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8002, controller_shadow);
    SetBit0547Atomic();
    SetBit0546Atomic();
    SetSfrA9Bit2Atomic();
    return (u8)((sample & 0x08u) != 0u);
}

/* RAM:faff96 */
void HandleUnsupportedScsiCommand(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B18, 0x70u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B1A, 0x05u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B24, 0x24u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B25, 0u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0B1B, 0ul);
}

/* RAM:fb617b */
void HandleControllerEvent40Completion(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_801F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8020, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8021, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_8022, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DA, 1u);
}

/* RAM:fd1e12 */
void TimerDispatchPhase4(void)
{
    DebounceP0Bit7State();
    DebounceControllerTransactionStatusBit0();
}

/* RAM:feeafe */
void EnableExternalInterrupt0IfPipelineModeNonzero(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) != 0u)
        SetExternalInterrupt0EnableAtomic();
}

/* RAM:feeb08 */
void DisableExternalInterrupt0IfPipelineModeNonzero(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) != 0u)
        ClearExternalInterrupt0EnableAtomic();
}

/* RAM:fee9de */
u8 CheckP1Bit3StableHighForCurrentMediaState(void)
{
    u8 attempt = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    u8 high_count;

    if(attempt == 0u)
        return 1u;
    if(attempt == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1245) & 0x0fu) == 0u)
        return 0u;

    if(ReadP1Bit3Atomic() == 0u)
    {
        attempt = 0u;
        for(;;)
        {
            attempt = (u8)(attempt + 1u);
            if(ReadP1Bit3Atomic() != 0u)
                break;
            if(attempt >= 0x26u)
                return 0u;
        }
    }

    high_count = 0u;
    do
    {
        if(ReadP1Bit3Atomic() == 0u)
            return 0u;
        high_count = (u8)(high_count + 1u);
    } while(high_count < 0x13u);
    return 1u;
}

/* RAM:f92955 */
void InitializeControllerBufferCursorRegistersIfNeeded(void)
{
    u8 controller_shadow;

    if(ReadBit0015Atomic() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F04, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFB) | 0x80u));
        SetBit0015Atomic();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0EFC) & 0xe0u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
            ClearBit0015ForCursorAtomic();
            controller_shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAD) & 0xdfu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAD, controller_shadow);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018003, controller_shadow);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802D, 0x20u);
        }
    }
    if(ReadBit0014Atomic() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFF, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018019, 0u);
        SetBit0014Atomic();
    }
}

/* RAM:fdc9c7 */
void HandleReadAcquisitionRequest0(void)
{
    ClearBit000EForReadRequestAtomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    PublishReadAcquisitionTarget();
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();
}

/* RAM:fdc9ed */
void HandleReadAcquisitionRequest3(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    PublishReadAcquisitionTarget();
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();
}

/* RAM:fdca7b */
void HandleReadAcquisitionRequest4(void)
{
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C8D) == 0u ? 1u : 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
}

/* RAM:fdcab6 */
void HandleReadAcquisitionRequest6(void)
{
    u8 direction_bit;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBE, 5u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C8D) == 0u)
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FC2, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E));
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FC6, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C92));
    }
    else
    {
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FC2, 0x00000096ul);
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FC6, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x10u));
    direction_bit = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C8D) & 1u) << 6);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xbfu) | direction_bit));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
    PublishReadAcquisitionTarget();
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();
}

/* RAM:fdca1e */
void HandleReadAcquisitionRequest2(void)
{
    u32 end_target = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C92);
    u32 pending_target = ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E);

    if(pending_target > end_target)
    {
        CompleteReadAcquisitionRequest(10u);
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C8D) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x08u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBE, 5u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FC2, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C8E));
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_0FC6, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C92));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
    PublishReadAcquisitionTarget();
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();
}

/* RAM:fbe338 */
void StoreVendorF3MeasurementOperands(u8 value, u16 word_value)
{
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_EF85, (u16)value);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_EF87, word_value);
}

/* RAM:fdcb48 */
void ResetMediaProcessingCoroutines(void)
{
    ClearBit000EForReadRequestAtomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
#define PLEXTOR_BINDING_SECTION 99
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
    InitializeCoroutine1AtF85D74ForC();
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();
}

/* RAM:fdcb8e */
void FUN_RAM_fdcb8e(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C4, 1u);
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xefu));
    InitializeCoroutine1AtF84B0BForC();
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2AtF84A00ForC();
    InitializeCoroutine3AtF84B00ForC();
}

/* RAM:fdcbdf */
void FUN_RAM_fdcbdf(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x02u));
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xefu));
    InitializeCoroutine1AtF852EBForC();
}

/* RAM:fef0cc */
void ConsumeRawReadDescriptors(u16 descriptor_count)
{
    u16 consumer_index;
    u16 index_mask;
    u16 queued_count;

    ClearBit0547Atomic();
    consumer_index = (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1294) + descriptor_count);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1294, consumer_index);
    index_mask = ReadBigEndianU16Near(FW_NEAR_UNKNOWN_129A);
    consumer_index = (u16)(index_mask & ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1294));
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1294, consumer_index);
    queued_count = (u16)(ReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) - descriptor_count);
    WriteBigEndianU16Near(FW_NEAR_UNKNOWN_1292, queued_count);
    SetBit0547Atomic();
}

#define PLEXTOR_BINDING_SECTION 103
#include "bindings/platform/leaf_workers_bindings.h"
#undef PLEXTOR_BINDING_SECTION
