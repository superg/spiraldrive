#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveDivideReadAcquisitionFramePositionBy75(s32 value);
extern u16 ObserveMapReadAcquisitionFrameSeconds(u16 frame_seconds);
extern u16 ObserveMapReadAcquisitionLeadInSeconds(u16 seconds_magnitude);

#define DivideReadAcquisitionFramePositionBy75(value) ObserveDivideReadAcquisitionFramePositionBy75(value)
#define MapReadAcquisitionFrameSeconds(value) ObserveMapReadAcquisitionFrameSeconds(value)
#define MapReadAcquisitionLeadInSeconds(value) ObserveMapReadAcquisitionLeadInSeconds(value)
#else
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
extern u16 MapFrameSecondsToRadialModelValue(u16 frame_seconds);
extern u16 MapLeadInSecondsMagnitudeToRadialModelValue(u16 lead_in_seconds_magnitude);

#define DivideReadAcquisitionFramePositionBy75(value) DivideS32QuotientNative((value), 75l)
#define MapReadAcquisitionFrameSeconds(value) MapFrameSecondsToRadialModelValue(value)
#define MapReadAcquisitionLeadInSeconds(value) MapLeadInSecondsMagnitudeToRadialModelValue(value)
#endif
#endif
