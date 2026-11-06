#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObservePositionSampleSelectNearest(void);
extern u8 ObservePositionSampleInteriorSpacing(s16 reference_value);
extern void ObservePositionSampleUpdateHighSpeedFlags(void);
extern u8 ObservePositionSampleIndexedState(u8 state_code);
#define SELECT_NEAREST_SAMPLE() ObservePositionSampleSelectNearest()
#define INTERIOR_SPACING_ACCEPTED(reference_value) ObservePositionSampleInteriorSpacing(reference_value)
#define UPDATE_HIGH_SPEED_FLAGS() ObservePositionSampleUpdateHighSpeedFlags()
#define INDEXED_STATE_ACTIVE(state_code) ObservePositionSampleIndexedState(state_code)
#else
extern s32 DivideS32Quotient(s32 dividend, s32 divisor);
#define SELECT_NEAREST_SAMPLE() SelectNearestControllerPositionSampleIndex()
#define INTERIOR_SPACING_ACCEPTED(reference_value) AreInteriorControllerPositionSamplesAboveReferenceBy30(reference_value)
#define UPDATE_HIGH_SPEED_FLAGS() UpdateHighWriteSpeedFlagsFromCalibrationThresholds()
#define INDEXED_STATE_ACTIVE(state_code) IsIndexedDriveStateActive(state_code)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
return dividend / divisor;
#else
return DivideS32Quotient(dividend, divisor);
#endif
#endif
