#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR15ForD8D9MeasurementSeries(void);
extern void ObserveWaitForD8D9MeasurementSeries(u16 duration_units, u8 yield_while_waiting);
extern void ObserveCaptureD8D9MiddleFourSumsForSeries(u8 series_index);
extern void ObserveSortD8D9MeasurementSeries(fw_addr_t array_address, u16 element_count);
extern void ObserveAdvanceControllerD8D9MeasurementSeriesInvocation(void);
#define CAPTURE_D8D9_SERIES_INHERITED_COUNT() ObserveCaptureInheritedR15ForD8D9MeasurementSeries()
#define WAIT_D8D9_SERIES(duration, yield) ObserveWaitForD8D9MeasurementSeries((duration), (yield))
#define CAPTURE_D8D9_SERIES(index) ObserveCaptureD8D9MiddleFourSumsForSeries(index)
#define SORT_D8D9_SERIES(address, count) ObserveSortD8D9MeasurementSeries((address), (count))
#define OBSERVE_D8D9_SERIES_INVOCATION() ObserveAdvanceControllerD8D9MeasurementSeriesInvocation()
#else
extern u8 CaptureInheritedR15ForD8D9MeasurementSeries(void);
#define CAPTURE_D8D9_SERIES_INHERITED_COUNT() CaptureInheritedR15ForD8D9MeasurementSeries()
#define WAIT_D8D9_SERIES(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define CAPTURE_D8D9_SERIES(index) CaptureControllerD8D9MiddleFourSumsForIndex(index)
#define SORT_D8D9_SERIES(address, count) SortWordArrayAscendingInPlace((address), (count))
#define OBSERVE_D8D9_SERIES_INVOCATION()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if !defined(PLEXTOR_HOST_TEST)
extern u16 DivideU16QuotientNative(u16 dividend, u16 divisor);
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#else
static u16 DivideU16QuotientNative(u16 dividend, u16 divisor)
{
    return divisor == 0u ? 0u : (u16)(dividend / divisor);
}

static s32 DivideS32QuotientNative(s32 dividend, s32 divisor)
{
    return divisor == 0l ? 0l : dividend / divisor;
}
#endif
#endif
