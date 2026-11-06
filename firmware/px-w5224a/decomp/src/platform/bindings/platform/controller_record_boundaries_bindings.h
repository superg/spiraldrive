#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
static s32 DivideType2RuntimePosition(s32 dividend, s32 divisor)
{
    return dividend / divisor;
}
#else
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#define DivideType2RuntimePosition(dividend, divisor) DivideS32QuotientNative((dividend), (divisor))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveProbeType2ControllerRecordWindow(s32 frame_position);
#define PROBE_TYPE2_CONTROLLER_RECORD_WINDOW ObserveProbeType2ControllerRecordWindow
#else
#define PROBE_TYPE2_CONTROLLER_RECORD_WINDOW ProbeControllerRecordAtFrameAndMapStatus
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR7ForActiveControllerRecordBoundaries(void);
#define CAPTURE_INHERITED_R7() ObserveCaptureInheritedR7ForActiveControllerRecordBoundaries()
#else
extern u8 CaptureInheritedR7ForActiveControllerRecordBoundaries(void);
#define CAPTURE_INHERITED_R7() CaptureInheritedR7ForActiveControllerRecordBoundaries()
#endif
#endif
