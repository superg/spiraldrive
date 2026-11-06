#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveRawPayloadSignatureMapNegative(s32 logical_block_address);
extern void ObserveRawPayloadSignatureSetBit001b(u8 value);
extern void ObserveRawPayloadSignatureResetQueue(void);
extern void ObserveRawPayloadSignatureInitializeCoroutine2(fw_addr_t entry_address);
extern void ObserveRawPayloadSignatureYieldCoroutine3(void);
extern u8 ObserveRawPayloadSignaturePollDescriptor(void);
extern u16 ObserveRawPayloadSignatureGetConsumerDescriptor(void);
extern void ObserveRawPayloadSignatureSetHandshakePins(u8 high);
#define MAP_NEGATIVE_PAYLOAD_ADDRESS(value) ObserveRawPayloadSignatureMapNegative(value)
#define SET_PAYLOAD_BIT001B(value) ObserveRawPayloadSignatureSetBit001b(value)
#define RESET_PAYLOAD_QUEUE() ObserveRawPayloadSignatureResetQueue()
#define INITIALIZE_PAYLOAD_COROUTINE2(entry) ObserveRawPayloadSignatureInitializeCoroutine2(entry)
#define YIELD_PAYLOAD_COROUTINE3() ObserveRawPayloadSignatureYieldCoroutine3()
#define POLL_PAYLOAD_DESCRIPTOR() ObserveRawPayloadSignaturePollDescriptor()
#define GET_PAYLOAD_CONSUMER_DESCRIPTOR() ObserveRawPayloadSignatureGetConsumerDescriptor()
#define SET_PAYLOAD_HANDSHAKE_PINS(high) ObserveRawPayloadSignatureSetHandshakePins(high)
#else
extern s32 MapNegativeLogicalBlockAddressThroughThirtyTwoPlusSevenLayout(s32 logical_block_address);
extern void PLEXTOR_NEAR_FUNCTION ClearBit001bForLbaMapping(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit001bForLbaMapping(void);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(fw_addr_t entry_address);
extern void YieldCoroutine3(void);
extern u8 PLEXTOR_NEAR_FUNCTION ClassifyLatestQueuedRawSectorDescriptor(void);
extern u16 GetRawReadConsumerDescriptorAddressOrZero(void);
extern void SetReadVerifyHandshakePinsLowAtomic(void);
extern void SetReadVerifyHandshakePinsHighAtomic(void);
#define MAP_NEGATIVE_PAYLOAD_ADDRESS(value) MapNegativeLogicalBlockAddressThroughThirtyTwoPlusSevenLayout(value)
#define SET_PAYLOAD_BIT001B(value) ((value) != 0u ? SetBit001bForLbaMapping() : ClearBit001bForLbaMapping())
#define RESET_PAYLOAD_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_PAYLOAD_COROUTINE2(entry) InitializeCoroutine2(entry)
#define YIELD_PAYLOAD_COROUTINE3() YieldCoroutine3()
#define POLL_PAYLOAD_DESCRIPTOR() ClassifyLatestQueuedRawSectorDescriptor()
#define GET_PAYLOAD_CONSUMER_DESCRIPTOR() GetRawReadConsumerDescriptorAddressOrZero()
#define SET_PAYLOAD_HANDSHAKE_PINS(high) ((high) != 0u ? SetReadVerifyHandshakePinsHighAtomic() : SetReadVerifyHandshakePinsLowAtomic())
#endif
#endif
