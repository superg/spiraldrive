#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveRadialStepScaleDivideS32(s32 dividend, s32 divisor);
extern u16 ObserveRadialStepScaleMapFrameSeconds(u16 frame_seconds);
extern u32 ObserveRadialStepScaleMultiplyU32(u32 factor_a, u32 factor_b);
extern u32 ObserveRadialStepScaleDivideU32(u32 dividend, u32 divisor);
#define RADIAL_SCALE_DIVIDE_S32(dividend, divisor) ObserveRadialStepScaleDivideS32((dividend), (divisor))
#define RADIAL_SCALE_MAP_SECONDS(seconds) ObserveRadialStepScaleMapFrameSeconds(seconds)
#define RADIAL_SCALE_MULTIPLY(a, b) ObserveRadialStepScaleMultiplyU32((a), (b))
#define RADIAL_SCALE_DIVIDE_U32(dividend, divisor) ObserveRadialStepScaleDivideU32((dividend), (divisor))
#else
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
extern u16 MapFrameSecondsToRadialModelValue(u16 frame_seconds);
#define RADIAL_SCALE_DIVIDE_S32(dividend, divisor) DivideS32QuotientNative((dividend), (divisor))
#define RADIAL_SCALE_MAP_SECONDS(seconds) MapFrameSecondsToRadialModelValue(seconds)
#define RADIAL_SCALE_MULTIPLY(a, b) MultiplyU32Low((a), (b))
#define RADIAL_SCALE_DIVIDE_U32(dividend, divisor) DivideU32QuotientNative((dividend), (divisor))
#endif
#endif
