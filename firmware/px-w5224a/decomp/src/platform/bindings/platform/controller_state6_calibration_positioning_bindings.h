#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveState6PositioningAcquire(void);
extern void ObserveState6PositioningReset(void);
extern u8 ObserveState6PositioningInitialize(void);
extern void ObserveState6PositioningEnable(u8 enabled);
extern u8 ObserveState6PositioningTolerance(u8 restart_timeout);
#define POSITION_ACQUIRE() ObserveState6PositioningAcquire()
#define POSITION_RESET() ObserveState6PositioningReset()
#define POSITION_INITIALIZE() ObserveState6PositioningInitialize()
#define POSITION_ENABLE(enabled) ObserveState6PositioningEnable(enabled)
#define POSITION_TOLERANCE(restart) ObserveState6PositioningTolerance(restart)
#else
#define POSITION_ACQUIRE() AcquireDiscReadPosition()
#define POSITION_RESET() ResetDriveControlStateToEight()
#define POSITION_INITIALIZE() InitializeControllerOperatingStateAndRunCalibration()
#define POSITION_ENABLE(enabled) SetControllerOperatingEnableState(enabled)
#define POSITION_TOLERANCE(restart) CheckControllerMeasurementToleranceOrTimeout(restart)
#endif
#endif
