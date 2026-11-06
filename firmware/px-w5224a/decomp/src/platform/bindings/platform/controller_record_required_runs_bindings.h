#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRequiredRunsAcquire(u16 last_index, u8 operation_mode);
extern u32 ObserveRequiredRunsDivide(u32 dividend, u32 divisor);
extern void ObserveRequiredRunsRecalculate(void);
extern void ObserveRequiredRunsBuildLeadIn(u8 selector);
extern void ObserveRequiredRunsAcquireBaseWindow(void);
#define REQUIRED_ACQUIRE(last, mode) ObserveRequiredRunsAcquire((last), (mode))
#define REQUIRED_DIVIDE(a, b) ObserveRequiredRunsDivide((a), (b))
#define REQUIRED_RECALCULATE() ObserveRequiredRunsRecalculate()
#define REQUIRED_BUILD_LEADIN(selector) ObserveRequiredRunsBuildLeadIn(selector)
#define REQUIRED_ACQUIRE_BASE() ObserveRequiredRunsAcquireBaseWindow()
#else
extern u8 AcquireZeroStateControllerRecordIndexRuns(u16 last_index, u8 operation_mode);
extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
extern void RecalculateType2ControllerRecordGeometryWindow(void);
extern void BuildType2LeadInTocRecordSequence(u8 selector);
extern u8 AcquireFortyFrameType2ControllerRecordBaseWindow(void);
#define REQUIRED_ACQUIRE(last, mode) AcquireZeroStateControllerRecordIndexRuns((last), (mode))
#define REQUIRED_DIVIDE(a, b) DivideU32QuotientNative((a), (b))
#define REQUIRED_RECALCULATE() RecalculateType2ControllerRecordGeometryWindow()
#define REQUIRED_BUILD_LEADIN(selector) BuildType2LeadInTocRecordSequence(selector)
#define REQUIRED_ACQUIRE_BASE() AcquireFortyFrameType2ControllerRecordBaseWindow()
#endif
#endif
