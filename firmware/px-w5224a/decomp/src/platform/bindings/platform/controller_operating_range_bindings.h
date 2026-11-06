#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void RunControllerMeasurementPass0(void);
extern void RunControllerMeasurementPass1(void);
#define RUN_CONTROLLER_MEASUREMENT_PASS0 RunControllerMeasurementPass0
#define RUN_CONTROLLER_MEASUREMENT_PASS1 RunControllerMeasurementPass1
#else
#define RUN_CONTROLLER_MEASUREMENT_PASS0 InitializeControllerMeasurementSeries00And01FromProfile
#define RUN_CONTROLLER_MEASUREMENT_PASS1 InitializeControllerMeasurementSeries00And01FromCenteredSlopes
#endif
#endif
