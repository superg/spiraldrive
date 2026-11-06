#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveTryReduceWriteSpeedForState6Calibration(u8 enabled);
extern u8 ObserveSetMediaCalibrationRetryFlag(void);
extern void ObserveInitializeMediaState6CalibrationRetryState(void);
extern void ObserveAdjustMediaCalibrationOffsetForRetry(void);
#define TRY_REDUCE_STATE6_CALIBRATION(enabled) ObserveTryReduceWriteSpeedForState6Calibration(enabled)
#define SET_STATE6_CALIBRATION_RETRY_FLAG() ObserveSetMediaCalibrationRetryFlag()
#define INITIALIZE_STATE6_CALIBRATION_RETRY() ObserveInitializeMediaState6CalibrationRetryState()
#define ADJUST_STATE6_CALIBRATION_OFFSET() ObserveAdjustMediaCalibrationOffsetForRetry()
#else
#define TRY_REDUCE_STATE6_CALIBRATION(enabled) TryReduceWriteSpeedForCurrentMediaState(enabled)
#define SET_STATE6_CALIBRATION_RETRY_FLAG() SetMediaCalibrationRetryFlag()
#define INITIALIZE_STATE6_CALIBRATION_RETRY() InitializeMediaState6CalibrationRetryState()
#define ADJUST_STATE6_CALIBRATION_OFFSET() AdjustEncodedMediaCalibrationOffsetForCurrentWriteSpeed()
#endif
#endif
