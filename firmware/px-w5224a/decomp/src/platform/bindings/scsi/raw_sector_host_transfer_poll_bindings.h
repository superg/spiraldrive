#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawSectorPollSetSense(u8 sense_key, u16 sense_code);
extern void ObserveRawSectorPollSetSenseInformation(u32 information);
extern u8 ObservePrepareRawReadDescriptorBatchForHostPoll(void);
extern u16 ObserveCountValidatedDescriptorsForHostPoll(void);
extern void ObservePrepareHostTransferBlockSizeForHostPoll(u16 block_size);
extern void ObserveStartBatchedRawReadHostTransferForHostPoll(void);
extern void ObserveYieldCoroutine0ForHostPoll(void);
extern u16 ObserveGetRawReadConsumerDescriptorForHostPoll(void);
extern u8 ObserveValidateRawSectorDescriptorForHostPoll(u16 descriptor_address);
extern u8 ObserveReadRawSectorBatchMappingBit(void);
extern u16 ObserveGetReadRawSectorBatchSecondaryDescriptor(void);
extern void ObserveResetReadRawSectorBatchQueue(void);
extern void ObserveInitializeReadRawSectorBatchCoroutine2(fw_addr_t entry_address);
extern s32 ObserveMapReadRawSectorBatchLogicalBlock(s32 logical_block_address);
extern void ObserveSetReadRawSectorBatchTransferBit(u8 value);
extern u8 ObserveConfigureReadRawSectorBatchTransfer(void);
extern void ObserveInitializeReadCommandRawSectorBits(void);
extern void ObserveClearReadCommandRawSectorValidationBits(void);
extern void ObserveReadCommandRawSectorCompletedCommand(void);
extern u8 ObserveValidateReadCommandRawSectorMediaState(void);
extern u8 ObserveReadCommandRawSectorMappingEnableBit(void);
extern void ObserveClearReadCommandRawSectorMappingBit(void);
extern s32 ObserveMapReadCommandRawSectorLba(s32 logical_block_address);
extern u8 ObserveValidateReadCommandRawSectorRange(void);
extern void ObserveUpdateReadCommandRawSectorProgress(void);
extern u32 ObserveResetReadCommandRawSectorTransferState(void);
extern void ObservePositionReadCommandRawSectorPipeline(s32 target_position);
extern void ObserveResetReadCommandRawSectorQueue(void);
extern void ObserveInitializeReadCommandRawSectorCoroutine2(fw_addr_t entry_address);
extern void ObserveYieldReadCommandRawSectorCoroutine0(void);
extern s8 ObserveServiceReadCommandRawSectorBatch(void);
extern void ObserveDispatchReadCommandRawSectorRequest5(void);
extern void ObserveSetReadCommandRawSectorHandshakePins(u8 high);
extern void ObserveClearReadCommandRawSectorPendingBit(void);
extern void ObserveInitializeReadCdRawSectorBits(void);
extern u8 ObserveReadReadCdRawSectorTerminalBit(void);
extern u16 ObservePositionRawReadGetLatestDescriptor(void);
extern u16 ObservePositionRawReadGetConsumerDescriptor(void);
extern void ObservePositionRawReadLimitBacklog(void);
extern void ObservePositionRawReadSetExternalInterrupt0(u8 enabled);
extern u8 ObservePositionRawReadBit0008(void);
extern u8 ObserveAdvanceRawReadMatchedTargetForBatchPoll(void);
#define SET_SENSE(key, code) ObserveRawSectorPollSetSense((key), (code))
#define SET_INFORMATION(value) ObserveRawSectorPollSetSenseInformation(value)
#define PREPARE_BATCH() ObservePrepareRawReadDescriptorBatchForHostPoll()
#define COUNT_VALIDATED() ObserveCountValidatedDescriptorsForHostPoll()
#define PREPARE_BLOCK_SIZE(value) ObservePrepareHostTransferBlockSizeForHostPoll(value)
#define START_BATCH_TRANSFER() ObserveStartBatchedRawReadHostTransferForHostPoll()
#define YIELD_COROUTINE0() ObserveYieldCoroutine0ForHostPoll()
#define GET_CONSUMER_DESCRIPTOR() ObserveGetRawReadConsumerDescriptorForHostPoll()
#define VALIDATE_DESCRIPTOR(address) ObserveValidateRawSectorDescriptorForHostPoll(address)
#define READ_BATCH_MAPPING_BIT() ObserveReadRawSectorBatchMappingBit()
#define GET_BATCH_SECONDARY_DESCRIPTOR() ObserveGetReadRawSectorBatchSecondaryDescriptor()
#define RESET_BATCH_QUEUE() ObserveResetReadRawSectorBatchQueue()
#define INITIALIZE_BATCH_COROUTINE2(address) ObserveInitializeReadRawSectorBatchCoroutine2(address)
#define MAP_BATCH_LOGICAL_BLOCK(address) ObserveMapReadRawSectorBatchLogicalBlock(address)
#define SET_BATCH_TRANSFER_BIT(value) ObserveSetReadRawSectorBatchTransferBit(value)
#define CONFIGURE_BATCH_TRANSFER() ObserveConfigureReadRawSectorBatchTransfer()
#define INITIALIZE_READ_COMMAND_BITS() ObserveInitializeReadCommandRawSectorBits()
#define CLEAR_READ_COMMAND_VALIDATION_BITS() ObserveClearReadCommandRawSectorValidationBits()
#define HANDLE_COMPLETED_READ_COMMAND() ObserveReadCommandRawSectorCompletedCommand()
#define VALIDATE_READ_COMMAND_MEDIA() ObserveValidateReadCommandRawSectorMediaState()
#define READ_READ_COMMAND_MAPPING_ENABLE() ObserveReadCommandRawSectorMappingEnableBit()
#define CLEAR_READ_COMMAND_MAPPING_BIT() ObserveClearReadCommandRawSectorMappingBit()
#define MAP_READ_COMMAND_LBA(value) ObserveMapReadCommandRawSectorLba(value)
#define VALIDATE_READ_COMMAND_RANGE() ObserveValidateReadCommandRawSectorRange()
#define UPDATE_READ_COMMAND_PROGRESS() ObserveUpdateReadCommandRawSectorProgress()
#define RESET_READ_COMMAND_TRANSFER() ObserveResetReadCommandRawSectorTransferState()
#define POSITION_READ_COMMAND_PIPELINE(target) ObservePositionReadCommandRawSectorPipeline(target)
#define RESET_READ_COMMAND_QUEUE() ObserveResetReadCommandRawSectorQueue()
#define INITIALIZE_READ_COMMAND_COROUTINE2(entry) ObserveInitializeReadCommandRawSectorCoroutine2(entry)
#define YIELD_READ_COMMAND_COROUTINE0() ObserveYieldReadCommandRawSectorCoroutine0()
#define SERVICE_READ_COMMAND_BATCH() ObserveServiceReadCommandRawSectorBatch()
#define DISPATCH_READ_COMMAND_REQUEST5() ObserveDispatchReadCommandRawSectorRequest5()
#define SET_READ_COMMAND_HANDSHAKE_PINS(high) ObserveSetReadCommandRawSectorHandshakePins(high)
#define CLEAR_READ_COMMAND_PENDING_BIT() ObserveClearReadCommandRawSectorPendingBit()
#define ExecuteReadCommandRawSectorTransferFunction ExecuteReadCommandRawSectorTransferForTest
#define INITIALIZE_READ_CD_BITS() ObserveInitializeReadCdRawSectorBits()
#define READ_READ_CD_TERMINAL_BIT() ObserveReadReadCdRawSectorTerminalBit()
#define ExecuteReadCdRawSectorTransferForFormatFunction ExecuteReadCdRawSectorTransferForFormatForTest
#define GET_POSITION_LATEST_DESCRIPTOR() ObservePositionRawReadGetLatestDescriptor()
#define GET_POSITION_CONSUMER_DESCRIPTOR() ObservePositionRawReadGetConsumerDescriptor()
#define LIMIT_POSITION_BACKLOG() ObservePositionRawReadLimitBacklog()
#define SET_POSITION_EXTERNAL_INTERRUPT0(enabled) ObservePositionRawReadSetExternalInterrupt0(enabled)
#define READ_POSITION_BIT0008() ObservePositionRawReadBit0008()
#define ADVANCE_BATCH_MATCHED_TARGET() ObserveAdvanceRawReadMatchedTargetForBatchPoll()
#define PositionRawReadPipelineAtFramePositionFunction PositionRawReadPipelineAtFramePositionForTest
#else
extern void SetSenseData(u8 sense_key, u16 sense_code);
extern void SetSenseInformation(u32 information);
extern u8 PrepareRawReadDescriptorBatchForInspection(void);
extern u16 CountValidatedRawReadDescriptorsForHostTransfer(void);
extern u32 FUN_RAM_fde9cf(u16 block_size);
extern void YieldCoroutine0(void);
extern u16 GetRawReadConsumerDescriptorAddressOrZero(void);
extern u8 ValidateRawSectorDescriptorStatus(u16 descriptor_address);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit000CForHostTransferDescriptorCount(void);
extern u16 GetRawReadSecondaryCursorDescriptorAddress(void);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(fw_addr_t entry_address);
extern s32 MapLogicalBlockAddressToControllerFramePosition(s32 logical_block_address);
extern void PLEXTOR_NEAR_FUNCTION WriteBit003CForReadCommandRawSectorTransferBatch(u8 value);
extern u8 ConfigureAndTransferRawReadSector(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeReadCommandRawSectorBitsAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearReadCommandRawSectorValidationBitsAtomic(void);
extern void DispatchRawSectorTransferSetup(void);
extern u8 PLEXTOR_NEAR_FUNCTION IsReadWindowOutsideAcceptedRange(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit000AForReadCommandRawSector(void);
extern void FirmwareWriteBit000C(u8 value);
extern u8 ValidateCurrentCommandLogicalBlockRange(void);
extern void UpdateRawReadProgressCountersAndPhaseFlags(void);
extern u32 ResetRawReadTransferState(void);
extern void PositionRawReadPipelineAtFramePosition(s32 target_position);
extern void YieldCoroutine0(void);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
extern void SetReadVerifyHandshakePinsLowAtomic(void);
extern void SetReadVerifyHandshakePinsHighAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0001ForReadCommandRawSectorAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeReadCdRawSectorBitsAtomic(void);
extern u8 FirmwareReadBit0011(void);
extern u16 GetLatestQueuedRawSectorDescriptorAddress(void);
extern void LimitRawReadSecondaryCursorBacklog(void);
extern void PLEXTOR_NEAR_FUNCTION SetExternalInterrupt0EnableAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearExternalInterrupt0EnableAtomic(void);
extern u8 AdvanceRawReadMatchedTargetCursorIfAvailable(void);
#define SET_SENSE(key, code) SetSenseData((key), (code))
#define SET_INFORMATION(value) SetSenseInformation(value)
#define PREPARE_BATCH() PrepareRawReadDescriptorBatchForInspection()
#define COUNT_VALIDATED() CountValidatedRawReadDescriptorsForHostTransfer()
#define PREPARE_BLOCK_SIZE(value) ((void)FUN_RAM_fde9cf(value))
#define START_BATCH_TRANSFER() StartBatchedRawReadHostTransferFromSecondaryCursor()
#define YIELD_COROUTINE0() YieldCoroutine0()
#define GET_CONSUMER_DESCRIPTOR() GetRawReadConsumerDescriptorAddressOrZero()
#define VALIDATE_DESCRIPTOR(address) ValidateRawSectorDescriptorStatus(address)
#define READ_BATCH_MAPPING_BIT() ReadBit000CForHostTransferDescriptorCount()
#define GET_BATCH_SECONDARY_DESCRIPTOR() GetRawReadSecondaryCursorDescriptorAddress()
#define RESET_BATCH_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_BATCH_COROUTINE2(address) InitializeCoroutine2(address)
#define MAP_BATCH_LOGICAL_BLOCK(address) MapLogicalBlockAddressToControllerFramePosition(address)
#define SET_BATCH_TRANSFER_BIT(value) WriteBit003CForReadCommandRawSectorTransferBatch(value)
#define CONFIGURE_BATCH_TRANSFER() ConfigureAndTransferRawReadSector()
#define INITIALIZE_READ_COMMAND_BITS() InitializeReadCommandRawSectorBitsAtomic()
#define CLEAR_READ_COMMAND_VALIDATION_BITS() ClearReadCommandRawSectorValidationBitsAtomic()
#define HANDLE_COMPLETED_READ_COMMAND() DispatchRawSectorTransferSetup()
#define VALIDATE_READ_COMMAND_MEDIA() IsReadWindowOutsideAcceptedRange()
#define READ_READ_COMMAND_MAPPING_ENABLE() ReadBit000AForReadCommandRawSector()
#define CLEAR_READ_COMMAND_MAPPING_BIT() FirmwareWriteBit000C(0u)
#define MAP_READ_COMMAND_LBA(value) MapLogicalBlockAddressToControllerFramePosition(value)
#define VALIDATE_READ_COMMAND_RANGE() ValidateCurrentCommandLogicalBlockRange()
#define UPDATE_READ_COMMAND_PROGRESS() UpdateRawReadProgressCountersAndPhaseFlags()
#define RESET_READ_COMMAND_TRANSFER() ResetRawReadTransferState()
#define POSITION_READ_COMMAND_PIPELINE(target) PositionRawReadPipelineAtFramePosition(target)
#define RESET_READ_COMMAND_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_READ_COMMAND_COROUTINE2(entry) InitializeCoroutine2(entry)
#define YIELD_READ_COMMAND_COROUTINE0() YieldCoroutine0()
#define SERVICE_READ_COMMAND_BATCH() ServiceReadCommandRawSectorTransferBatch()
#define DISPATCH_READ_COMMAND_REQUEST5() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define SET_READ_COMMAND_HANDSHAKE_PINS(high) ((high) != 0u ? SetReadVerifyHandshakePinsHighAtomic() : SetReadVerifyHandshakePinsLowAtomic())
#define CLEAR_READ_COMMAND_PENDING_BIT() ClearBit0001ForReadCommandRawSectorAtomic()
#define ExecuteReadCommandRawSectorTransferFunction ExecuteReadCommandRawSectorTransfer
#define INITIALIZE_READ_CD_BITS() InitializeReadCdRawSectorBitsAtomic()
#define READ_READ_CD_TERMINAL_BIT() FirmwareReadBit0011()
#define ExecuteReadCdRawSectorTransferForFormatFunction ExecuteReadCdRawSectorTransferForFormat
#define GET_POSITION_LATEST_DESCRIPTOR() GetLatestQueuedRawSectorDescriptorAddress()
#define GET_POSITION_CONSUMER_DESCRIPTOR() GetRawReadConsumerDescriptorAddressOrZero()
#define LIMIT_POSITION_BACKLOG() LimitRawReadSecondaryCursorBacklog()
#define SET_POSITION_EXTERNAL_INTERRUPT0(enabled) ((enabled) != 0u ? SetExternalInterrupt0EnableAtomic() : ClearExternalInterrupt0EnableAtomic())
#define READ_POSITION_BIT0008() FirmwareReadBit0008()
#define ADVANCE_BATCH_MATCHED_TARGET() AdvanceRawReadMatchedTargetCursorIfAvailable()
#define PositionRawReadPipelineAtFramePositionFunction PositionRawReadPipelineAtFramePosition
#endif
#endif
