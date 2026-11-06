#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureReadTrackInformationInheritedB(void);
extern void ObserveClearRecordableReadTrackInformation(void);
extern u8 ObserveLoadRecordableReadTrackInformation(u8 track_number, u8 inherited_b);
extern u8 ObserveLoadRecordableReadTrackInformationSnapshot(u8 track_number, u8 inherited_b);
extern u32 ObserveComputeReadTrackInformationFreeBlockCount(void);
extern u32 ObserveComputeReadTrackInformationTrackSizeBlocks(void);
#define CAPTURE_READ_TRACK_INHERITED_B() ObserveCaptureReadTrackInformationInheritedB()
#define CLEAR_RECORDABLE_READ_TRACK() ObserveClearRecordableReadTrackInformation()
#define LOAD_RECORDABLE_READ_TRACK(track, inherited_b) ObserveLoadRecordableReadTrackInformation((track), (inherited_b))
#define LOAD_RECORDABLE_READ_TRACK_SNAPSHOT(track, inherited_b) ObserveLoadRecordableReadTrackInformationSnapshot((track), (inherited_b))
#define COMPUTE_READ_TRACK_FREE_BLOCKS() ObserveComputeReadTrackInformationFreeBlockCount()
#define COMPUTE_READ_TRACK_SIZE_BLOCKS() ObserveComputeReadTrackInformationTrackSizeBlocks()
#else
extern u8 CaptureInheritedBForReadTrackInformation(void);
#define CAPTURE_READ_TRACK_INHERITED_B() CaptureInheritedBForReadTrackInformation()
#define CLEAR_RECORDABLE_READ_TRACK() ClearWorkingTrackInformation()
#define LOAD_RECORDABLE_READ_TRACK(track, inherited_b) LoadTrackInformationByNumberFromInheritedB((track), (inherited_b))
#define LOAD_RECORDABLE_READ_TRACK_SNAPSHOT(track, inherited_b) LoadTrackInformationAgainstSnapshotFromInheritedB((track), (inherited_b))
#define COMPUTE_READ_TRACK_FREE_BLOCKS() ComputeReadTrackInformationFreeBlockCount()
#define COMPUTE_READ_TRACK_SIZE_BLOCKS() ComputeReadTrackInformationTrackSizeBlocks()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveCaptureReadTrackInformationOutputAddress(void);
extern void ObserveSetReadTrackInformationSense(u8 sense_key, u16 asc_ascq);
extern u8 ObservePrepareReadTrackInformationDescriptorBatch(void);
extern u16 ObserveGetReadTrackInformationSecondaryDescriptor(void);
extern u8 ObserveValidateReadTrackInformationDescriptor(u16 descriptor_address);
#define CAPTURE_READ_TRACK_OUTPUT_ADDRESS() ObserveCaptureReadTrackInformationOutputAddress()
#define SET_READ_TRACK_SENSE(key, code) ObserveSetReadTrackInformationSense((key), (code))
#define PREPARE_READ_TRACK_DESCRIPTOR_BATCH() ObservePrepareReadTrackInformationDescriptorBatch()
#define GET_READ_TRACK_SECONDARY_DESCRIPTOR() ObserveGetReadTrackInformationSecondaryDescriptor()
#define VALIDATE_READ_TRACK_DESCRIPTOR(address) ObserveValidateReadTrackInformationDescriptor(address)
#else
extern fw_addr_t CaptureInheritedDR0ForReadTrackInformation(void);
extern u8 PrepareRawReadDescriptorBatchForInspection(void);
extern u8 ValidateRawSectorDescriptorStatus(u16 descriptor_address);
#define CAPTURE_READ_TRACK_OUTPUT_ADDRESS() CaptureInheritedDR0ForReadTrackInformation()
#define SET_READ_TRACK_SENSE(key, code) SetSenseData((key), (code))
#define PREPARE_READ_TRACK_DESCRIPTOR_BATCH() PrepareRawReadDescriptorBatchForInspection()
#define GET_READ_TRACK_SECONDARY_DESCRIPTOR() GetRawReadSecondaryCursorDescriptorAddress()
#define VALIDATE_READ_TRACK_DESCRIPTOR(address) ValidateRawSectorDescriptorStatus(address)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern void ClearP3Bit0Atomic(void);
extern void ClearP3Bit1Atomic(void);
extern void SetP3Bit0Atomic(void);
extern void ObserveClearReadTrackInformationRawModeBits(void);
extern s32 ObservePositionReadTrackInformationPipeline(s32 target_position);
extern void ObserveResetReadTrackInformationRawReadQueue(void);
extern void ObserveInitializeReadTrackInformationCoroutine2AtF90000(void);
extern void ObserveInitializeReadTrackInformationCoroutine2AtF84A00(void);
extern void ObserveYieldReadTrackInformationCoroutine0(void);
extern u8 ObservePollReadTrackInformationFlagsFromC(u8 *output_flags);
extern void ObserveDispatchReadTrackInformationRequest5(void);
extern void ObserveUnsupportedFullTocReadTrackInformation(void);
extern u8 ObserveFindFullTocTrackForFramePosition(s32 frame_position);
extern u8 ObserveAcquireFullTocReadTrackFlags(u8 toc_point_index);
extern s32 ObserveComputeFullTocReadTrackSpan(u8 toc_point_index);
extern s32 ObserveCaptureReadTrackNextWritableFallback(void);
extern s32 ObserveComputeReadTrackNextWritableFramePosition(void);
extern s32 ObserveComputeReadTrackPacketProgressFramePosition(u8 compress_packet_gaps);
#define CLEAR_READ_TRACK_PINS()                                                                                                                                                                        \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ClearP3Bit0Atomic();                                                                                                                                                                           \
        ClearP3Bit1Atomic();                                                                                                                                                                           \
    } while(0)
#define COMPLETE_READ_TRACK_PINS()                                                                                                                                                                     \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        SetP3Bit0Atomic();                                                                                                                                                                             \
        ClearP3Bit1Atomic();                                                                                                                                                                           \
    } while(0)
#define CLEAR_READ_TRACK_MODE_BITS() ObserveClearReadTrackInformationRawModeBits()
#define POSITION_READ_TRACK_PIPELINE(target) ((void)ObservePositionReadTrackInformationPipeline(target))
#define RESET_READ_TRACK_QUEUE() ObserveResetReadTrackInformationRawReadQueue()
#define INITIALIZE_READ_TRACK_COROUTINE_F90000() ObserveInitializeReadTrackInformationCoroutine2AtF90000()
#define INITIALIZE_READ_TRACK_COROUTINE_F84A00() ObserveInitializeReadTrackInformationCoroutine2AtF84A00()
#define YIELD_READ_TRACK_COROUTINE0() ObserveYieldReadTrackInformationCoroutine0()
#define POLL_READ_TRACK_FLAGS(output) ObservePollReadTrackInformationFlagsFromC(output)
#define DISPATCH_READ_TRACK_REQUEST5() ObserveDispatchReadTrackInformationRequest5()
#define UNSUPPORTED_FULL_TOC_READ_TRACK() ObserveUnsupportedFullTocReadTrackInformation()
#define FIND_FULL_TOC_TRACK(position) ObserveFindFullTocTrackForFramePosition(position)
#define ACQUIRE_FULL_TOC_READ_TRACK_FLAGS(index) ObserveAcquireFullTocReadTrackFlags(index)
#define COMPUTE_FULL_TOC_READ_TRACK_SPAN(index) ObserveComputeFullTocReadTrackSpan(index)
#define CAPTURE_READ_TRACK_NEXT_WRITABLE_FALLBACK() ObserveCaptureReadTrackNextWritableFallback()
#define COMPUTE_READ_TRACK_NEXT_WRITABLE() ObserveComputeReadTrackNextWritableFramePosition()
#define COMPUTE_READ_TRACK_PACKET_PROGRESS(compress) ObserveComputeReadTrackPacketProgressFramePosition(compress)
#else
extern void ClearP3Bit0Atomic(void);
extern void ClearP3Bit1Atomic(void);
extern void SetP3Bit0Atomic(void);
extern void ClearReadTrackInformationRawModeBitsAtomic(void);
extern void PositionRawReadPipelineAtFramePositionFromC(s32 target_position);
extern void ResetRawReadQueueAndTransferState(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF90000ForC(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF84A00ForC(void);
extern void YieldCoroutine0(void);
extern u8 PollReadTrackInformationRawDescriptorFlagsFromC(fw_addr_t output);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
extern void HandleUnsupportedScsiCommand(void);
extern s32 CaptureInheritedDR12ForReadTrackNextWritable(void);
#define CLEAR_READ_TRACK_PINS()                                                                                                                                                                        \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ClearP3Bit0Atomic();                                                                                                                                                                           \
        ClearP3Bit1Atomic();                                                                                                                                                                           \
    } while(0)
#define COMPLETE_READ_TRACK_PINS()                                                                                                                                                                     \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        SetP3Bit0Atomic();                                                                                                                                                                             \
        ClearP3Bit1Atomic();                                                                                                                                                                           \
    } while(0)
#define CLEAR_READ_TRACK_MODE_BITS() ClearReadTrackInformationRawModeBitsAtomic()
#define POSITION_READ_TRACK_PIPELINE(target) PositionRawReadPipelineAtFramePositionFromC(target)
#define RESET_READ_TRACK_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_READ_TRACK_COROUTINE_F90000() InitializeCoroutine2AtF90000ForC()
#define INITIALIZE_READ_TRACK_COROUTINE_F84A00() InitializeCoroutine2AtF84A00ForC()
#define YIELD_READ_TRACK_COROUTINE0() YieldCoroutine0()
#define POLL_READ_TRACK_FLAGS(output) PollReadTrackInformationRawDescriptorFlagsFromC((fw_addr_t)(output))
#define DISPATCH_READ_TRACK_REQUEST5() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define UNSUPPORTED_FULL_TOC_READ_TRACK() HandleUnsupportedScsiCommand()
#define FIND_FULL_TOC_TRACK(position) FindTrackNumberForFramePositionFromTocTable(position)
#define ACQUIRE_FULL_TOC_READ_TRACK_FLAGS(index) AcquireReadTrackInformationFlagsFromRawDescriptor(index)
#define COMPUTE_FULL_TOC_READ_TRACK_SPAN(index) ComputeFullTocTrackSpanFrames(index)
#define CAPTURE_READ_TRACK_NEXT_WRITABLE_FALLBACK() CaptureInheritedDR12ForReadTrackNextWritable()
#define COMPUTE_READ_TRACK_NEXT_WRITABLE() ComputeWorkingTrackNextWritableFramePosition()
#define COMPUTE_READ_TRACK_PACKET_PROGRESS(compress) ComputeWorkingTrackPacketProgressFramePosition(compress)
#endif
#endif
