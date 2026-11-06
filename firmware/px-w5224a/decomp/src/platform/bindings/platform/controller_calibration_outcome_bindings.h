#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResolveDriveControlForCalibrationOutcome(void);
extern void ObserveRestartCalibrationOutcomeAtF84B0B(void);
extern void ObserveRestartCalibrationOutcomeAtF8F07C(void);
#define RESOLVE_CALIBRATION_OUTCOME_DRIVE_CONTROL() ObserveResolveDriveControlForCalibrationOutcome()
#define RESTART_CALIBRATION_OUTCOME_AT_F84B0B() ObserveRestartCalibrationOutcomeAtF84B0B()
#define RESTART_CALIBRATION_OUTCOME_AT_F8F07C() ObserveRestartCalibrationOutcomeAtF8F07C()
#else
extern void ResolveDriveControlStateToZero(void);
extern void RestartCoroutine1AtF84B0BAndYield(void);
extern void RestartCoroutine1AtF8F07CAndYield(void);
#define RESOLVE_CALIBRATION_OUTCOME_DRIVE_CONTROL() ResolveDriveControlStateToZero()
#define RESTART_CALIBRATION_OUTCOME_AT_F84B0B() RestartCoroutine1AtF84B0BAndYield()
#define RESTART_CALIBRATION_OUTCOME_AT_F8F07C() RestartCoroutine1AtF8F07CAndYield()
#endif
#endif
