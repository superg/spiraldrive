#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObservePrimaryRawReadPipelineBit(u8 bit_address);
extern void ObserveWritePrimaryRawReadPipelineBit(u8 bit_address, u8 value);
extern u8 ObserveCaptureInheritedR15ForPrimaryRawReadPipelineStep(void);
#define READ_PIPELINE_BIT(address) ObservePrimaryRawReadPipelineBit(address)
#define WRITE_PIPELINE_BIT(address, value) ObserveWritePrimaryRawReadPipelineBit((address), (value))
#define CAPTURE_PIPELINE_RETRY() ObserveCaptureInheritedR15ForPrimaryRawReadPipelineStep()
#else
u8 bdata PrimaryRawReadPipelineBitByte20 _at_ 0x20;
u8 bdata PrimaryRawReadPipelineBitByte21 _at_ 0x21;
u8 bdata PrimaryRawReadPipelineBitByte22 _at_ 0x22;
u8 bdata PrimaryRawReadPipelineBitByte23 _at_ 0x23;
u8 bdata PrimaryRawReadPipelineBitByte24 _at_ 0x24;
sbit PrimaryRawReadPipelineBit0007 = PrimaryRawReadPipelineBitByte20 ^ 7;
sbit PrimaryRawReadPipelineBit0009 = PrimaryRawReadPipelineBitByte21 ^ 1;
sbit PrimaryRawReadPipelineBit000C = PrimaryRawReadPipelineBitByte21 ^ 4;
sbit PrimaryRawReadPipelineBit000F = PrimaryRawReadPipelineBitByte21 ^ 7;
sbit PrimaryRawReadPipelineBit0013 = PrimaryRawReadPipelineBitByte22 ^ 3;
sbit PrimaryRawReadPipelineBit001B = PrimaryRawReadPipelineBitByte23 ^ 3;
sbit PrimaryRawReadPipelineBit001C = PrimaryRawReadPipelineBitByte23 ^ 4;
sbit PrimaryRawReadPipelineBit001D = PrimaryRawReadPipelineBitByte23 ^ 5;
sbit PrimaryRawReadPipelineBit001F = PrimaryRawReadPipelineBitByte23 ^ 7;
sbit PrimaryRawReadPipelineBit0020 = PrimaryRawReadPipelineBitByte24 ^ 0;
sbit PrimaryRawReadPipelineBit0021 = PrimaryRawReadPipelineBitByte24 ^ 1;
sbit PrimaryRawReadPipelineBit0025 = PrimaryRawReadPipelineBitByte24 ^ 5;
extern u8 PLEXTOR_NEAR_FUNCTION CaptureInheritedR15ForPrimaryRawReadPipelineStep(void);
#define READ_PIPELINE_BIT_0007() ((u8)PrimaryRawReadPipelineBit0007)
#define READ_PIPELINE_BIT_0009() ((u8)PrimaryRawReadPipelineBit0009)
#define READ_PIPELINE_BIT_000C() ((u8)PrimaryRawReadPipelineBit000C)
#define READ_PIPELINE_BIT_000F() ((u8)PrimaryRawReadPipelineBit000F)
#define READ_PIPELINE_BIT_0013() ((u8)PrimaryRawReadPipelineBit0013)
#define READ_PIPELINE_BIT_001B() ((u8)PrimaryRawReadPipelineBit001B)
#define READ_PIPELINE_BIT_001C() ((u8)PrimaryRawReadPipelineBit001C)
#define READ_PIPELINE_BIT_001D() ((u8)PrimaryRawReadPipelineBit001D)
#define READ_PIPELINE_BIT_001F() ((u8)PrimaryRawReadPipelineBit001F)
#define READ_PIPELINE_BIT_0020() ((u8)PrimaryRawReadPipelineBit0020)
#define READ_PIPELINE_BIT_0021() ((u8)PrimaryRawReadPipelineBit0021)
#define READ_PIPELINE_BIT_0025() ((u8)PrimaryRawReadPipelineBit0025)
#define WRITE_PIPELINE_BIT_0007(value) (PrimaryRawReadPipelineBit0007 = (value))
#define WRITE_PIPELINE_BIT_0009(value) (PrimaryRawReadPipelineBit0009 = (value))
#define WRITE_PIPELINE_BIT_001F(value) (PrimaryRawReadPipelineBit001F = (value))
#define WRITE_PIPELINE_BIT_0020(value) (PrimaryRawReadPipelineBit0020 = (value))
#define WRITE_PIPELINE_BIT_0025(value) (PrimaryRawReadPipelineBit0025 = (value))
#define CAPTURE_PIPELINE_RETRY() CaptureInheritedR15ForPrimaryRawReadPipelineStep()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
#define READ_PIPELINE_BIT_0007() READ_PIPELINE_BIT(0x07u)
#define READ_PIPELINE_BIT_0009() READ_PIPELINE_BIT(0x09u)
#define READ_PIPELINE_BIT_000C() READ_PIPELINE_BIT(0x0cu)
#define READ_PIPELINE_BIT_000F() READ_PIPELINE_BIT(0x0fu)
#define READ_PIPELINE_BIT_0013() READ_PIPELINE_BIT(0x13u)
#define READ_PIPELINE_BIT_001B() READ_PIPELINE_BIT(0x1bu)
#define READ_PIPELINE_BIT_001C() READ_PIPELINE_BIT(0x1cu)
#define READ_PIPELINE_BIT_001D() READ_PIPELINE_BIT(0x1du)
#define READ_PIPELINE_BIT_001F() READ_PIPELINE_BIT(0x1fu)
#define READ_PIPELINE_BIT_0020() READ_PIPELINE_BIT(0x20u)
#define READ_PIPELINE_BIT_0021() READ_PIPELINE_BIT(0x21u)
#define READ_PIPELINE_BIT_0025() READ_PIPELINE_BIT(0x25u)
#define WRITE_PIPELINE_BIT_0007(value) WRITE_PIPELINE_BIT(0x07u, value)
#define WRITE_PIPELINE_BIT_0009(value) WRITE_PIPELINE_BIT(0x09u, value)
#define WRITE_PIPELINE_BIT_001F(value) WRITE_PIPELINE_BIT(0x1fu, value)
#define WRITE_PIPELINE_BIT_0020(value) WRITE_PIPELINE_BIT(0x20u, value)
#define WRITE_PIPELINE_BIT_0025(value) WRITE_PIPELINE_BIT(0x25u, value)
#endif
#endif
