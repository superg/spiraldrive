#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObservePrimaryRawReadCoroutineBit(u8 bit_address);
extern void ObserveWritePrimaryRawReadCoroutineBit(u8 bit_address, u8 value);
extern u8 ObserveCallPrimaryRawReadPipelineStep(void);
extern void ObservePrimaryRawReadCoroutineYield(void);
extern u8 ObservePrimaryRawReadCoroutineCheckP1(void);
extern void ObservePrimaryRawReadCoroutineFinalize(void);
#define READ_PRIMARY_COROUTINE_BIT(address) ObservePrimaryRawReadCoroutineBit(address)
#define WRITE_PRIMARY_COROUTINE_BIT(address, value) ObserveWritePrimaryRawReadCoroutineBit((address), (value))
#define RUN_PRIMARY_PIPELINE_STEP() ObserveCallPrimaryRawReadPipelineStep()
#define YIELD_PRIMARY_COROUTINE2() ObservePrimaryRawReadCoroutineYield()
#define CHECK_PRIMARY_COROUTINE_P1() ObservePrimaryRawReadCoroutineCheckP1()
#define FINALIZE_PRIMARY_COROUTINE2() ObservePrimaryRawReadCoroutineFinalize()
#else
u8 bdata PrimaryRawReadCoroutineBitByte20 _at_ 0x20;
u8 bdata PrimaryRawReadCoroutineBitByte21 _at_ 0x21;
u8 bdata PrimaryRawReadCoroutineBitByte23 _at_ 0x23;
u8 bdata PrimaryRawReadCoroutineBitByte24 _at_ 0x24;
sbit PrimaryRawReadCoroutineBit0004 = PrimaryRawReadCoroutineBitByte20 ^ 4;
sbit PrimaryRawReadCoroutineBit0005 = PrimaryRawReadCoroutineBitByte20 ^ 5;
sbit PrimaryRawReadCoroutineBit0007 = PrimaryRawReadCoroutineBitByte20 ^ 7;
sbit PrimaryRawReadCoroutineBit0008 = PrimaryRawReadCoroutineBitByte21 ^ 0;
sbit PrimaryRawReadCoroutineBit0009 = PrimaryRawReadCoroutineBitByte21 ^ 1;
sbit PrimaryRawReadCoroutineBit000C = PrimaryRawReadCoroutineBitByte21 ^ 4;
sbit PrimaryRawReadCoroutineBit000E = PrimaryRawReadCoroutineBitByte21 ^ 6;
sbit PrimaryRawReadCoroutineBit0018 = PrimaryRawReadCoroutineBitByte23 ^ 0;
sbit PrimaryRawReadCoroutineBit001F = PrimaryRawReadCoroutineBitByte23 ^ 7;
sbit PrimaryRawReadCoroutineBit0020 = PrimaryRawReadCoroutineBitByte24 ^ 0;
sbit PrimaryRawReadCoroutineBit0024 = PrimaryRawReadCoroutineBitByte24 ^ 4;
sbit PrimaryRawReadCoroutineBit0025 = PrimaryRawReadCoroutineBitByte24 ^ 5;
sbit PrimaryRawReadCoroutineBit0026 = PrimaryRawReadCoroutineBitByte24 ^ 6;
extern u8 PLEXTOR_NEAR_FUNCTION CallPrimaryRawReadPipelineStepWithDescriptorPointer(void);
#define READ_PRIMARY_COROUTINE_BIT_0005() ((u8)PrimaryRawReadCoroutineBit0005)
#define READ_PRIMARY_COROUTINE_BIT_0009() ((u8)PrimaryRawReadCoroutineBit0009)
#define READ_PRIMARY_COROUTINE_BIT_000C() ((u8)PrimaryRawReadCoroutineBit000C)
#define READ_PRIMARY_COROUTINE_BIT_0020() ((u8)PrimaryRawReadCoroutineBit0020)
#define WRITE_PRIMARY_COROUTINE_BIT_0004(value) (PrimaryRawReadCoroutineBit0004 = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_0005(value) (PrimaryRawReadCoroutineBit0005 = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_0007(value) (PrimaryRawReadCoroutineBit0007 = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_0008(value) (PrimaryRawReadCoroutineBit0008 = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_0009(value) (PrimaryRawReadCoroutineBit0009 = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_000E(value) (PrimaryRawReadCoroutineBit000E = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_0018(value) (PrimaryRawReadCoroutineBit0018 = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_001F(value) (PrimaryRawReadCoroutineBit001F = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_0024(value) (PrimaryRawReadCoroutineBit0024 = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_0025(value) (PrimaryRawReadCoroutineBit0025 = (value))
#define WRITE_PRIMARY_COROUTINE_BIT_0026(value) (PrimaryRawReadCoroutineBit0026 = (value))
#define RUN_PRIMARY_PIPELINE_STEP() CallPrimaryRawReadPipelineStepWithDescriptorPointer()
#define YIELD_PRIMARY_COROUTINE2() YieldCoroutine2()
#define CHECK_PRIMARY_COROUTINE_P1() CheckP1Bit3StableHighForCurrentMediaState()
#define FINALIZE_PRIMARY_COROUTINE2() FinalizeRawReadPipelineAndIdleCoroutine2()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define READ_PRIMARY_COROUTINE_BIT_0005() READ_PRIMARY_COROUTINE_BIT(0x05u)
#define READ_PRIMARY_COROUTINE_BIT_0009() READ_PRIMARY_COROUTINE_BIT(0x09u)
#define READ_PRIMARY_COROUTINE_BIT_000C() READ_PRIMARY_COROUTINE_BIT(0x0cu)
#define READ_PRIMARY_COROUTINE_BIT_0020() READ_PRIMARY_COROUTINE_BIT(0x20u)
#define WRITE_PRIMARY_COROUTINE_BIT_0004(value) WRITE_PRIMARY_COROUTINE_BIT(0x04u, value)
#define WRITE_PRIMARY_COROUTINE_BIT_0005(value) WRITE_PRIMARY_COROUTINE_BIT(0x05u, value)
#define WRITE_PRIMARY_COROUTINE_BIT_0007(value) WRITE_PRIMARY_COROUTINE_BIT(0x07u, value)
#define WRITE_PRIMARY_COROUTINE_BIT_0008(value) WRITE_PRIMARY_COROUTINE_BIT(0x08u, value)
#define WRITE_PRIMARY_COROUTINE_BIT_0009(value) WRITE_PRIMARY_COROUTINE_BIT(0x09u, value)
#define WRITE_PRIMARY_COROUTINE_BIT_000E(value) WRITE_PRIMARY_COROUTINE_BIT(0x0eu, value)
#define WRITE_PRIMARY_COROUTINE_BIT_0018(value) WRITE_PRIMARY_COROUTINE_BIT(0x18u, value)
#define WRITE_PRIMARY_COROUTINE_BIT_001F(value) WRITE_PRIMARY_COROUTINE_BIT(0x1fu, value)
#define WRITE_PRIMARY_COROUTINE_BIT_0024(value) WRITE_PRIMARY_COROUTINE_BIT(0x24u, value)
#define WRITE_PRIMARY_COROUTINE_BIT_0025(value) WRITE_PRIMARY_COROUTINE_BIT(0x25u, value)
#define WRITE_PRIMARY_COROUTINE_BIT_0026(value) WRITE_PRIMARY_COROUTINE_BIT(0x26u, value)
#endif
#endif
