#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadVerifyMappingModeBit(void);
extern void ObserveClearReadVerifyMappingModeBit(void);
extern void ObserveClearReadVerifyRawModeBits(void);
extern void ObserveSetReadVerifyHandshakePins(u8 high);
extern void ObserveReadVerifySetSenseData(u8 sense_key, u16 sense_code);
extern void ObserveReadVerifySetSenseInformation(u32 information);
extern s32 ObserveMapReadVerifyLogicalBlockAddress(s32 logical_block_address);
extern u8 ObserveValidateReadVerifyLogicalBlockRange(void);
extern void ObserveResetReadVerifyRawReadQueue(void);
extern void ObserveInitializeReadVerifyCoroutine2(fw_addr_t entry_address);
extern u32 ObserveResetReadVerifyTransferState(void);
extern u8 ObservePollReadVerifyRawDescriptorStatus(void);
extern void ObserveYieldReadVerifyCoroutine0(void);
extern void ObserveSetReadVerifyNoSense(void);
extern void ObserveDispatchReadVerifyAcquisitionRequest5(void);
extern void ObserveFullReadVerifyRecoveryInitializeCoroutine1(fw_addr_t entry_address);
extern void ObserveFullReadVerifyRecoverySetNoSense(void);
extern void ObserveFullReadVerifyRecoveryExecuteRange(void);
extern void ObserveFullReadVerifyRecoveryYieldCoroutine0(void);
extern void ObserveReadVerifyPollSetSense(u8 sense_key, u16 sense_code);
extern void ObserveReadVerifyPollSetSenseInformation(u32 information);
extern u16 ObserveReadVerifyPollGetConsumerDescriptor(void);
extern u8 ObserveReadVerifyPollValidateDescriptor(u16 descriptor_address);
extern u8 ObserveReadVerifyPollMappingModeBit(void);
extern void ObserveReadVerifyPollConsumeDescriptors(u16 descriptor_count);
extern void ObserveReadVerifyPollResetRawReadQueue(void);
extern void ObserveReadVerifyPollInitializeCoroutine2(fw_addr_t entry_address);
extern s32 ObserveReadVerifyPollMapLogicalBlockAddress(s32 logical_block_address);
#define READ_MAPPING_MODE_BIT() ObserveReadVerifyMappingModeBit()
#define CLEAR_MAPPING_MODE_BIT() ObserveClearReadVerifyMappingModeBit()
#define CLEAR_RAW_MODE_BITS() ObserveClearReadVerifyRawModeBits()
#define SET_HANDSHAKE_PINS(high) ObserveSetReadVerifyHandshakePins(high)
#define SET_SENSE(key, code) ObserveReadVerifySetSenseData((key), (code))
#define SET_SENSE_INFORMATION(value) ObserveReadVerifySetSenseInformation(value)
#define MAP_LOGICAL_BLOCK(value) ObserveMapReadVerifyLogicalBlockAddress(value)
#define VALIDATE_RANGE() ObserveValidateReadVerifyLogicalBlockRange()
#define RESET_RAW_READ_QUEUE() ObserveResetReadVerifyRawReadQueue()
#define INITIALIZE_COROUTINE2(address) ObserveInitializeReadVerifyCoroutine2(address)
#define RESET_TRANSFER_STATE() ObserveResetReadVerifyTransferState()
#define POLL_DESCRIPTOR_STATUS() ObservePollReadVerifyRawDescriptorStatus()
#define YIELD_COROUTINE0() ObserveYieldReadVerifyCoroutine0()
#define SET_NO_SENSE() ObserveSetReadVerifyNoSense()
#define DISPATCH_REQUEST5() ObserveDispatchReadVerifyAcquisitionRequest5()
#define RECOVERY_INITIALIZE_COROUTINE1(address) ObserveFullReadVerifyRecoveryInitializeCoroutine1(address)
#define RECOVERY_SET_NO_SENSE() ObserveFullReadVerifyRecoverySetNoSense()
#define RECOVERY_EXECUTE_RANGE() ObserveFullReadVerifyRecoveryExecuteRange()
#define RECOVERY_YIELD_COROUTINE0() ObserveFullReadVerifyRecoveryYieldCoroutine0()
#define POLL_SET_SENSE(key, code) ObserveReadVerifyPollSetSense((key), (code))
#define POLL_SET_SENSE_INFORMATION(value) ObserveReadVerifyPollSetSenseInformation(value)
#define POLL_GET_CONSUMER_DESCRIPTOR() ObserveReadVerifyPollGetConsumerDescriptor()
#define POLL_VALIDATE_DESCRIPTOR(address) ObserveReadVerifyPollValidateDescriptor(address)
#define POLL_MAPPING_MODE_BIT() ObserveReadVerifyPollMappingModeBit()
#define POLL_CONSUME_DESCRIPTORS(count) ObserveReadVerifyPollConsumeDescriptors(count)
#define POLL_RESET_RAW_READ_QUEUE() ObserveReadVerifyPollResetRawReadQueue()
#define POLL_INITIALIZE_COROUTINE2(address) ObserveReadVerifyPollInitializeCoroutine2(address)
#define POLL_MAP_LOGICAL_BLOCK_ADDRESS(value) ObserveReadVerifyPollMapLogicalBlockAddress(value)
#define ReadVerifyFunction ExecuteReadVerifyRangeForTest
#else
extern u8 ReadBit000AForReadVerifyRange(void);
extern void ClearBit000CForReadVerifyRange(void);
extern void ClearReadVerifyRawModeBitsAtomic(void);
extern void SetReadVerifyHandshakePinsLowAtomic(void);
extern void SetReadVerifyHandshakePinsHighAtomic(void);
extern void SetSenseData(u8 sense_key, u16 sense_code);
extern void SetSenseInformation(u32 information);
extern s32 MapLogicalBlockAddressToControllerFramePosition(s32 logical_block_address);
extern u8 ValidateCurrentCommandLogicalBlockRange(void);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(fw_addr_t entry_address);
extern u32 ResetRawReadTransferState(void);
extern u8 PollReadVerifyRawDescriptorStatus(void);
extern void YieldCoroutine0(void);
extern void SetNoSenseData(void);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
extern void InitializeCoroutine1(fw_addr_t entry_address);
extern u16 GetRawReadConsumerDescriptorAddressOrZero(void);
extern u8 ValidateRawSectorDescriptorStatus(u16 descriptor_address);
extern u8 ReadBit000CForReadVerifyDescriptorPoll(void);
extern void ConsumeRawReadDescriptors(u16 descriptor_count);
#define READ_MAPPING_MODE_BIT() ReadBit000AForReadVerifyRange()
#define CLEAR_MAPPING_MODE_BIT() ClearBit000CForReadVerifyRange()
#define CLEAR_RAW_MODE_BITS() ClearReadVerifyRawModeBitsAtomic()
#define SET_HANDSHAKE_PINS(high) ((high) != 0u ? SetReadVerifyHandshakePinsHighAtomic() : SetReadVerifyHandshakePinsLowAtomic())
#define SET_SENSE(key, code) SetSenseData((key), (code))
#define SET_SENSE_INFORMATION(value) SetSenseInformation(value)
#define MAP_LOGICAL_BLOCK(value) MapLogicalBlockAddressToControllerFramePosition(value)
#define VALIDATE_RANGE() ValidateCurrentCommandLogicalBlockRange()
#define RESET_RAW_READ_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_COROUTINE2(address) InitializeCoroutine2(address)
#define RESET_TRANSFER_STATE() ResetRawReadTransferState()
#define POLL_DESCRIPTOR_STATUS() PollReadVerifyRawDescriptorStatus()
#define YIELD_COROUTINE0() YieldCoroutine0()
#define SET_NO_SENSE() SetNoSenseData()
#define DISPATCH_REQUEST5() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define RECOVERY_INITIALIZE_COROUTINE1(address) InitializeCoroutine1(address)
#define RECOVERY_SET_NO_SENSE() SetNoSenseData()
#define RECOVERY_EXECUTE_RANGE() ExecuteReadVerifyRange()
#define RECOVERY_YIELD_COROUTINE0() YieldCoroutine0()
#define POLL_SET_SENSE(key, code) SetSenseData((key), (code))
#define POLL_SET_SENSE_INFORMATION(value) SetSenseInformation(value)
#define POLL_GET_CONSUMER_DESCRIPTOR() GetRawReadConsumerDescriptorAddressOrZero()
#define POLL_VALIDATE_DESCRIPTOR(address) ValidateRawSectorDescriptorStatus(address)
#define POLL_MAPPING_MODE_BIT() ReadBit000CForReadVerifyDescriptorPoll()
#define POLL_CONSUME_DESCRIPTORS(count) ConsumeRawReadDescriptors(count)
#define POLL_RESET_RAW_READ_QUEUE() ResetRawReadQueueAndTransferState()
#define POLL_INITIALIZE_COROUTINE2(address) InitializeCoroutine2(address)
#define POLL_MAP_LOGICAL_BLOCK_ADDRESS(value) MapLogicalBlockAddressToControllerFramePosition(value)
#define ReadVerifyFunction ExecuteReadVerifyRange
#endif
#endif
