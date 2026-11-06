#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawReadPatternClearBit(u8 bit_address);
extern void ObserveRawReadPatternResetQueue(void);
extern void ObserveRawReadPatternInitializeCoroutine2(u32 entry_point);
extern u32 ObserveRawReadPatternResetTransfer(void);
extern void ObserveRawReadPatternYield(void);
extern u8 ObserveRawReadPatternPollBatch(void);
extern u8 ObserveRawReadPatternValidateBatch(u16 descriptor_count);
extern void ObserveRawReadPatternDispatchRequest5(void);
#define CLEAR_PATTERN_BIT(bit) ObserveRawReadPatternClearBit(bit)
#define RESET_PATTERN_QUEUE() ObserveRawReadPatternResetQueue()
#define INITIALIZE_PATTERN_COROUTINE(entry) ObserveRawReadPatternInitializeCoroutine2(entry)
#define RESET_PATTERN_TRANSFER() ObserveRawReadPatternResetTransfer()
#define YIELD_PATTERN_COROUTINE() ObserveRawReadPatternYield()
#define POLL_PATTERN_BATCH() ObserveRawReadPatternPollBatch()
#define VALIDATE_PATTERN_BATCH(count) ObserveRawReadPatternValidateBatch(count)
#define DISPATCH_PATTERN_REQUEST5() ObserveRawReadPatternDispatchRequest5()
#else
u8 bdata RawReadPatternBitByte21 _at_ 0x21;
u8 bdata RawReadPatternBitByte22 _at_ 0x22;
u8 bdata RawReadPatternBitByte23 _at_ 0x23;
sbit RawReadPatternBit000C = RawReadPatternBitByte21 ^ 4;
sbit RawReadPatternBit000D = RawReadPatternBitByte21 ^ 5;
sbit RawReadPatternBit0013 = RawReadPatternBitByte22 ^ 3;
sbit RawReadPatternBit001C = RawReadPatternBitByte23 ^ 4;
sbit RawReadPatternBit001D = RawReadPatternBitByte23 ^ 5;
#define CLEAR_PATTERN_BIT_000C() (RawReadPatternBit000C = 0)
#define CLEAR_PATTERN_BIT_000D() (RawReadPatternBit000D = 0)
#define CLEAR_PATTERN_BIT_0013() (RawReadPatternBit0013 = 0)
#define CLEAR_PATTERN_BIT_001C() (RawReadPatternBit001C = 0)
#define CLEAR_PATTERN_BIT_001D() (RawReadPatternBit001D = 0)
#define RESET_PATTERN_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_PATTERN_COROUTINE(entry) InitializeCoroutine2(entry)
#define RESET_PATTERN_TRANSFER() ResetRawReadTransferState()
#define YIELD_PATTERN_COROUTINE() YieldCoroutine0()
#define POLL_PATTERN_BATCH() CheckQueuedRawReadDescriptorBatchStatus()
#define VALIDATE_PATTERN_BATCH(count) ValidateRawReadDescriptorPayloadPatternBatch(count)
#define DISPATCH_PATTERN_REQUEST5() DispatchReadAcquisitionRequest5ForPipelineTarget()
extern void YieldCoroutine0(void);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define CLEAR_PATTERN_BIT_000C() CLEAR_PATTERN_BIT(0x0cu)
#define CLEAR_PATTERN_BIT_000D() CLEAR_PATTERN_BIT(0x0du)
#define CLEAR_PATTERN_BIT_0013() CLEAR_PATTERN_BIT(0x13u)
#define CLEAR_PATTERN_BIT_001C() CLEAR_PATTERN_BIT(0x1cu)
#define CLEAR_PATTERN_BIT_001D() CLEAR_PATTERN_BIT(0x1du)
#endif
#endif
