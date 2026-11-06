#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveF4CalibrationConfigure(u8 selector_code);
extern void ObserveF4CalibrationApplyInitialDownAdjustment(void);
extern void ObserveF4CalibrationWait(u16 duration_units, u8 yield_while_waiting);
extern void ObserveF4CalibrationWriteParameter(u16 parameter_and_value);
extern u8 ObserveF4CalibrationMeasurement(void);
extern void ObserveF4CalibrationRestore(void);
#define CONFIGURE_CALIBRATION(code) ObserveF4CalibrationConfigure(code)
#define APPLY_INITIAL_F4() ObserveF4CalibrationApplyInitialDownAdjustment()
#define WAIT_CALIBRATION(duration, yield) ObserveF4CalibrationWait((duration), (yield))
#define WRITE_PARAMETER(value) ObserveF4CalibrationWriteParameter(value)
#define MEASURE_CALIBRATION() ObserveF4CalibrationMeasurement()
#define RESTORE_PARAMETERS() ObserveF4CalibrationRestore()
#else
extern void WriteControllerParametersD4AndE2ForCode(u8 selector_code);
extern void WriteControllerParameterF4WithDownAdjustment(void);
extern void WaitTimer1ScaledDuration(u16 duration_units, u8 yield_while_waiting);
extern void WriteControllerParameterWord(u16 parameter_and_value);
extern u8 MeasureAveragedSfrD8MinusD9WithOffsets(void);
extern void WriteControllerParametersD412AndE2Value58(void);
#define CONFIGURE_CALIBRATION(code) WriteControllerParametersD4AndE2ForCode(code)
#define APPLY_INITIAL_F4() WriteControllerParameterF4WithDownAdjustment()
#define WAIT_CALIBRATION(duration, yield) WaitTimer1ScaledDuration((duration), (yield))
#define WRITE_PARAMETER(value) WriteControllerParameterWord(value)
#define MEASURE_CALIBRATION() MeasureAveragedSfrD8MinusD9WithOffsets()
#define RESTORE_PARAMETERS() WriteControllerParametersD412AndE2Value58()
#endif
#endif
