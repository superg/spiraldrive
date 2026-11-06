#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObservePrepareReadHeaderPollBatch(void);
extern u16 ObserveCountReadHeaderPollDescriptors(void);
extern u16 ObserveGetReadHeaderPollDescriptor(void);
extern u8 ObserveValidateReadHeaderPollDescriptor(u16 descriptor_address);
extern void ObserveConvertReadHeaderPollFrame(s32 frame_position, fw_addr_t destination_address);
extern u16 ObserveMinReadHeaderPollLength(u16 left, u16 right);
extern void ObserveTransferReadHeaderPollResponse(fw_addr_t source_address, u32 byte_count);
extern void ObserveSetNoSenseForReadHeaderPoll(void);
extern void ObserveSetReadHeaderPollErrorSense(u8 sense_key, u16 sense_code);
#define PREPARE_READ_HEADER_POLL_BATCH() ObservePrepareReadHeaderPollBatch()
#define COUNT_READ_HEADER_POLL_DESCRIPTORS() ObserveCountReadHeaderPollDescriptors()
#define GET_READ_HEADER_POLL_DESCRIPTOR() ObserveGetReadHeaderPollDescriptor()
#define VALIDATE_READ_HEADER_POLL_DESCRIPTOR(address) ObserveValidateReadHeaderPollDescriptor(address)
#define CONVERT_READ_HEADER_POLL_FRAME(frame, destination) ObserveConvertReadHeaderPollFrame((frame), (destination))
#define MIN_READ_HEADER_POLL_LENGTH(left, right) ObserveMinReadHeaderPollLength((left), (right))
#define TRANSFER_READ_HEADER_POLL_RESPONSE(source, count) ObserveTransferReadHeaderPollResponse((source), (count))
#define SET_NO_SENSE_FOR_READ_HEADER_POLL() ObserveSetNoSenseForReadHeaderPoll()
#define SET_READ_HEADER_POLL_ERROR_SENSE(key, code) ObserveSetReadHeaderPollErrorSense((key), (code))
#else
extern u8 PrepareRawReadDescriptorBatchForInspection(void);
extern u16 CountValidatedRawReadDescriptorsForHostTransfer(void);
extern u16 GetRawReadSecondaryCursorDescriptorAddress(void);
extern u8 ValidateRawSectorDescriptorStatus(u16 descriptor_address);
extern void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t destination_address);
extern u16 MinU16(u16 left, u16 right);
extern u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source_address, u32 byte_count);
extern void SetNoSenseData(void);
#define PREPARE_READ_HEADER_POLL_BATCH() PrepareRawReadDescriptorBatchForInspection()
#define COUNT_READ_HEADER_POLL_DESCRIPTORS() CountValidatedRawReadDescriptorsForHostTransfer()
#define GET_READ_HEADER_POLL_DESCRIPTOR() GetRawReadSecondaryCursorDescriptorAddress()
#define VALIDATE_READ_HEADER_POLL_DESCRIPTOR(address) ValidateRawSectorDescriptorStatus(address)
#define CONVERT_READ_HEADER_POLL_FRAME(frame, destination) ConvertFramePositionToBinaryMsfTuple((frame), (destination))
#define MIN_READ_HEADER_POLL_LENGTH(left, right) MinU16((left), (right))
#define TRANSFER_READ_HEADER_POLL_RESPONSE(source, count) ((void)CopyResponseToControllerBufferAndExecuteDataIn((source), (count)))
#define SET_NO_SENSE_FOR_READ_HEADER_POLL() SetNoSenseData()
#define SET_READ_HEADER_POLL_ERROR_SENSE(key, code) SetSenseData((key), (code))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePrepareReadHeaderValidationBits(void);
extern void ObserveClearReadHeaderValidationBit(void);
extern void ObserveSetReadHeaderHandshakePins(u8 high);
extern void ObserveReadHeaderSetSense(u8 sense_key, u16 sense_code);
extern void ObserveReadHeaderSetSenseInformation(u32 information);
extern u8 ObserveValidateReadHeaderRange(void);
extern u32 ObserveResetReadHeaderTransferState(void);
extern void ObservePositionReadHeaderPipeline(s32 target_position);
extern void ObserveResetReadHeaderQueue(void);
extern void ObserveInitializeReadHeaderCoroutine2(fw_addr_t entry_address);
extern void ObserveYieldReadHeaderCoroutine0(void);
extern u8 ObservePollReadHeaderResponse(void);
extern void ObserveDispatchReadHeaderAcquisitionRequest5(void);
#define PREPARE_VALIDATION_BITS() ObservePrepareReadHeaderValidationBits()
#define CLEAR_VALIDATION_BIT() ObserveClearReadHeaderValidationBit()
#define SET_HANDSHAKE_PINS(high) ObserveSetReadHeaderHandshakePins(high)
#define SET_SENSE(key, code) ObserveReadHeaderSetSense((key), (code))
#define SET_INFORMATION(value) ObserveReadHeaderSetSenseInformation(value)
#define VALIDATE_RANGE() ObserveValidateReadHeaderRange()
#define RESET_TRANSFER_STATE() ObserveResetReadHeaderTransferState()
#define POSITION_PIPELINE(target) ObservePositionReadHeaderPipeline(target)
#define RESET_QUEUE() ObserveResetReadHeaderQueue()
#define INITIALIZE_COROUTINE2(address) ObserveInitializeReadHeaderCoroutine2(address)
#define YIELD_COROUTINE0() ObserveYieldReadHeaderCoroutine0()
#define POLL_RESPONSE() ObservePollReadHeaderResponse()
#define DISPATCH_REQUEST5() ObserveDispatchReadHeaderAcquisitionRequest5()
#define ReadHeaderAcquisitionFunction ExecuteReadHeaderSectorAcquisitionForTest
#else
extern void PrepareReadHeaderRangeValidationBitsAtomic(void);
extern void ClearBit000DForReadHeaderAtomic(void);
extern void SetReadVerifyHandshakePinsLowAtomic(void);
extern void SetReadVerifyHandshakePinsHighAtomic(void);
extern void SetSenseData(u8 sense_key, u16 sense_code);
extern void SetSenseInformation(u32 information);
extern u8 ValidateCurrentCommandLogicalBlockRange(void);
extern u32 ResetRawReadTransferState(void);
extern void PositionRawReadPipelineAtFramePosition(s32 target_position);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(fw_addr_t entry_address);
extern void YieldCoroutine0(void);
extern u8 PollReadHeaderSectorAndTransferResponse(void);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
#define PREPARE_VALIDATION_BITS() PrepareReadHeaderRangeValidationBitsAtomic()
#define CLEAR_VALIDATION_BIT() ClearBit000DForReadHeaderAtomic()
#define SET_HANDSHAKE_PINS(high) ((high) != 0u ? SetReadVerifyHandshakePinsHighAtomic() : SetReadVerifyHandshakePinsLowAtomic())
#define SET_SENSE(key, code) SetSenseData((key), (code))
#define SET_INFORMATION(value) SetSenseInformation(value)
#define VALIDATE_RANGE() ValidateCurrentCommandLogicalBlockRange()
#define RESET_TRANSFER_STATE() ResetRawReadTransferState()
#define POSITION_PIPELINE(target) PositionRawReadPipelineAtFramePosition(target)
#define RESET_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_COROUTINE2(address) InitializeCoroutine2(address)
#define YIELD_COROUTINE0() YieldCoroutine0()
#define POLL_RESPONSE() PollReadHeaderSectorAndTransferResponse()
#define DISPATCH_REQUEST5() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define ReadHeaderAcquisitionFunction ExecuteReadHeaderSectorAcquisition
#endif
#endif
