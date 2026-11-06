#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawQTargetClearBit0017(void);
extern u8 ObserveRawQTargetReadBit0017(void);
extern u8 ObserveRawQTargetReadBit0008(void);
extern void ObserveRawQTargetResetQueue(void);
extern void ObserveRawQTargetInitializeCoroutine2(fw_addr_t entry_address);
extern void ObserveRawQTargetYieldCoroutine3(void);
extern u16 ObserveRawQTargetGetLatestDescriptor(void);
extern u8 ObserveRawQTargetReadRingByte(void);
extern void ObserveRawQTargetSetHandshakePins(u8 high);
#define CLEAR_TARGET_BIT0017() ObserveRawQTargetClearBit0017()
#define READ_TARGET_BIT0017() ObserveRawQTargetReadBit0017()
#define READ_TARGET_BIT0008() ObserveRawQTargetReadBit0008()
#define RESET_TARGET_QUEUE() ObserveRawQTargetResetQueue()
#define INITIALIZE_TARGET_COROUTINE2(entry) ObserveRawQTargetInitializeCoroutine2(entry)
#define YIELD_TARGET_COROUTINE3() ObserveRawQTargetYieldCoroutine3()
#define GET_TARGET_LATEST_DESCRIPTOR() ObserveRawQTargetGetLatestDescriptor()
#define READ_TARGET_RING_BYTE() ObserveRawQTargetReadRingByte()
#define SET_TARGET_HANDSHAKE_PINS(high) ObserveRawQTargetSetHandshakePins(high)
#else
extern void PLEXTOR_NEAR_FUNCTION ClearBit0017ForRawQPositionProbeAtomic(void);
extern u8 FirmwareReadBit0017(void);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(fw_addr_t entry_address);
extern void YieldCoroutine3(void);
extern u16 GetLatestQueuedRawSectorDescriptorAddress(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadMainDataRingByteAtCursorPlus16(void);
extern void SetReadVerifyHandshakePinsLowAtomic(void);
extern void SetReadVerifyHandshakePinsHighAtomic(void);
#define CLEAR_TARGET_BIT0017() ClearBit0017ForRawQPositionProbeAtomic()
#define READ_TARGET_BIT0017() FirmwareReadBit0017()
#define READ_TARGET_BIT0008() FirmwareReadBit0008()
#define RESET_TARGET_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_TARGET_COROUTINE2(entry) InitializeCoroutine2(entry)
#define YIELD_TARGET_COROUTINE3() YieldCoroutine3()
#define GET_TARGET_LATEST_DESCRIPTOR() GetLatestQueuedRawSectorDescriptorAddress()
#define READ_TARGET_RING_BYTE() ReadMainDataRingByteAtCursorPlus16()
#define SET_TARGET_HANDSHAKE_PINS(high) ((high) != 0u ? SetReadVerifyHandshakePinsHighAtomic() : SetReadVerifyHandshakePinsLowAtomic())
#endif
#endif
