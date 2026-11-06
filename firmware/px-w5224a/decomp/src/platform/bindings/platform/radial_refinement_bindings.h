#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRadialRefinementAcquire(void);
extern u8 ObserveRadialRefinementReset(void);
extern void ObserveRadialRefinementOperatingEnable(u8 enabled);
extern u8 ObserveRadialRefinementTarget(s32 target);
extern u16 ObserveRadialRefinementMeasure(void);
extern void ObserveRadialRefinementSetMeasurement(u16 measurement);
extern void ObserveRadialRefinementRebuild(void);
extern void ObserveRadialRefinementYield(void);
#define RefinementAcquire() ObserveRadialRefinementAcquire()
#define RefinementReset() ObserveRadialRefinementReset()
#define RefinementOperatingEnable(value) ObserveRadialRefinementOperatingEnable(value)
#define RefinementTarget(value) ObserveRadialRefinementTarget(value)
#define RefinementMeasure() ObserveRadialRefinementMeasure()
#define RefinementSetMeasurement(value) ObserveRadialRefinementSetMeasurement(value)
#define RefinementRebuild() ObserveRadialRefinementRebuild()
#define RefinementYield() ObserveRadialRefinementYield()
static u32 DivideU32QuotientNative(u32 dividend, u32 divisor)
{
    return divisor == 0ul ? 0ul : dividend / divisor;
}
#else
extern u8 AcquireDiscReadPosition(void);
extern void YieldCoroutine1(void);
extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
#define RefinementAcquire() AcquireDiscReadPosition()
#define RefinementReset() ResetDriveControlAndInitializeControllerCalibration()
#define RefinementOperatingEnable(value) SetControllerOperatingEnableState(value)
#define RefinementTarget(value) IsDecodedFramePositionWithinTwoFramesOfTarget(value)
#define RefinementMeasure() MeasureAverageControllerPositionWord()
#define RefinementSetMeasurement(value) SetRadialModelMeasurementAndScale(value)
#define RefinementRebuild() RebuildRadialPositionTableAndThresholds()
#define RefinementYield() YieldCoroutine1()
#endif
#endif
