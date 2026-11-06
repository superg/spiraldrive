#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePreparePremiumRawSectorBatchBits(void);
extern void ObserveResetPremiumRawSectorBatchQueue(void);
extern void ObserveInitializePremiumRawSectorBatchCoroutine2(u32 entry_point);
extern void ObserveYieldPremiumRawSectorBatchCoroutine0(void);
extern u8 ObserveReadPremiumRawSectorBatchProducerActive(void);
extern u16 ObserveGetPremiumRawSectorBatchConsumerDescriptor(void);
extern void ObserveServicePremiumRawSectorBatchHostCommand(void);
extern void ObserveDispatchPremiumRawSectorBatchReadRequest(void);
extern void ObserveResetPremiumPayloadRawReadQueue(void);
extern void ObserveInitializePremiumPayloadCoroutine2(u32 entry_point);
extern void ObserveInitializePremiumPayloadCoroutine3(u32 entry_point);
extern void ObserveYieldPremiumPayloadCoroutine0(void);
extern u32 ObserveAcquirePremiumPayloadRawSectorBatch(s32 start_frame_position, u16 sector_count);
extern void ObserveResetPremiumPayloadMediaCoroutines(void);
extern void ObserveSetPremiumPayloadP3RxLowTxHigh(void);
extern void ObserveClearPremiumPayloadP1TimerPins(void);
extern void ObserveServicePremiumPayloadHostCommand(void);
extern u32 ObserveAcquirePremiumSignatureRawSectorBatch(s32 start_frame_position, u16 sector_count);
extern void ObserveServicePremiumSignatureHostCommand(void);
#define PREPARE_BATCH_BITS() ObservePreparePremiumRawSectorBatchBits()
#define RESET_BATCH_QUEUE() ObserveResetPremiumRawSectorBatchQueue()
#define INITIALIZE_COROUTINE2(entry) ObserveInitializePremiumRawSectorBatchCoroutine2(entry)
#define YIELD_COROUTINE0() ObserveYieldPremiumRawSectorBatchCoroutine0()
#define READ_PRODUCER_ACTIVE() ObserveReadPremiumRawSectorBatchProducerActive()
#define GET_CONSUMER_DESCRIPTOR() ObserveGetPremiumRawSectorBatchConsumerDescriptor()
#define SERVICE_HOST_COMMAND() ObserveServicePremiumRawSectorBatchHostCommand()
#define DISPATCH_READ_REQUEST() ObserveDispatchPremiumRawSectorBatchReadRequest()
#define PAYLOAD_RESET_QUEUE() ObserveResetPremiumPayloadRawReadQueue()
#define PAYLOAD_INITIALIZE_COROUTINE2(entry) ObserveInitializePremiumPayloadCoroutine2(entry)
#define PAYLOAD_INITIALIZE_COROUTINE3(entry) ObserveInitializePremiumPayloadCoroutine3(entry)
#define PAYLOAD_YIELD_COROUTINE0() ObserveYieldPremiumPayloadCoroutine0()
#define PAYLOAD_ACQUIRE_BATCH(start, count) ObserveAcquirePremiumPayloadRawSectorBatch((start), (count))
#define PAYLOAD_RESET_MEDIA_COROUTINES() ObserveResetPremiumPayloadMediaCoroutines()
#define PAYLOAD_SET_P3_RX_LOW_TX_HIGH() ObserveSetPremiumPayloadP3RxLowTxHigh()
#define PAYLOAD_CLEAR_P1_TIMER_PINS() ObserveClearPremiumPayloadP1TimerPins()
#define PAYLOAD_SERVICE_HOST_COMMAND() ObserveServicePremiumPayloadHostCommand()
#define SIGNATURE_ACQUIRE_BATCH(start, count) ObserveAcquirePremiumSignatureRawSectorBatch((start), (count))
#define SIGNATURE_SERVICE_HOST_COMMAND() ObserveServicePremiumSignatureHostCommand()
#else
extern void PreparePremiumRawSectorBatchBitsAtomic(void);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(u32 entry_point);
extern void YieldCoroutine0(void);
extern u16 GetRawReadConsumerDescriptorAddressOrZero(void);
extern void ServicePendingHostCommandIfAny(void);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
extern void InitializeCoroutine3(u32 entry_point);
extern void ResetMediaProcessingCoroutines(void);
extern void SetP3RxLowTxHighForPremiumPayload(void);
extern void ClearP1TimerPinsForPremiumPayload(void);
#define PREPARE_BATCH_BITS() PreparePremiumRawSectorBatchBitsAtomic()
#define RESET_BATCH_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define YIELD_COROUTINE0() YieldCoroutine0()
#define READ_PRODUCER_ACTIVE() FirmwareReadBit0008()
#define GET_CONSUMER_DESCRIPTOR() GetRawReadConsumerDescriptorAddressOrZero()
#define SERVICE_HOST_COMMAND() ServicePendingHostCommandIfAny()
#define DISPATCH_READ_REQUEST() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define PAYLOAD_RESET_QUEUE() ResetRawReadQueueAndTransferState()
#define PAYLOAD_INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define PAYLOAD_INITIALIZE_COROUTINE3(entry) InitializeCoroutine3(entry)
#define PAYLOAD_YIELD_COROUTINE0() YieldCoroutine0()
#define PAYLOAD_ACQUIRE_BATCH(start, count) AcquirePlextorPremiumRawSectorBatch((start), (count))
#define PAYLOAD_RESET_MEDIA_COROUTINES() ResetMediaProcessingCoroutines()
#define PAYLOAD_SET_P3_RX_LOW_TX_HIGH() SetP3RxLowTxHighForPremiumPayload()
#define PAYLOAD_CLEAR_P1_TIMER_PINS() ClearP1TimerPinsForPremiumPayload()
#define PAYLOAD_SERVICE_HOST_COMMAND() ServicePendingHostCommandIfAny()
#define SIGNATURE_ACQUIRE_BATCH(start, count) AcquirePlextorPremiumRawSectorBatch((start), (count))
#define SIGNATURE_SERVICE_HOST_COMMAND() ServicePendingHostCommandIfAny()
#endif
#endif
