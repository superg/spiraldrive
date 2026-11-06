#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWords02040319Yield(void);
extern void ObserveWords02040319Configure(void);
extern void ObserveWords02040319WriteParameter(u16 value);
extern void ObserveWords02040319Submit(u16 value, u16 selector);
extern u8 ObserveWords02040319Pulse(u8 sample_count);
extern u8 ObserveWords02040319IndexedState(u8 state);
extern void ObserveWords02040319LowerF4(void);
extern u16 ObserveWords02040319Command0E(void);
extern void ObserveWords02040319ApplyAndSettle(void);
extern void ObserveWords02040319Outcome(u8 stage);
extern u8 ObserveWords02040319Calibrate0319(void);
#define CAL_YIELD() ObserveWords02040319Yield()
#define CONFIGURE_CONTROLLER() ObserveWords02040319Configure()
#define WRITE_PARAMETER(value) ObserveWords02040319WriteParameter(value)
#define SUBMIT_WORD(value, selector) ObserveWords02040319Submit((value), (selector))
#define TEST_PULSE(count) ObserveWords02040319Pulse(count)
#define IS_INDEXED_STATE(state) ObserveWords02040319IndexedState(state)
#define LOWER_PARAMETER_F4() ObserveWords02040319LowerF4()
#define RUN_COMMAND_0E() ObserveWords02040319Command0E()
#define APPLY_WORD_0204_AND_SETTLE() ObserveWords02040319ApplyAndSettle()
#define UPDATE_OUTCOME(stage) ObserveWords02040319Outcome(stage)
#define CALIBRATE_WORD_0319() ObserveWords02040319Calibrate0319()
#else
extern void YieldCoroutine1(void);
extern s16 DivideS16Quotient(s16 dividend, s16 divisor);
extern s32 DivideS32Quotient(s32 dividend, s32 divisor);
#define CAL_YIELD() YieldCoroutine1()
#define CONFIGURE_CONTROLLER() ConfigureControllerOperatingStateAndCalibratePort8097()
#define WRITE_PARAMETER(value) WriteControllerParameterWord(value)
#define SUBMIT_WORD(value, selector) SubmitControllerWordPair((value), (selector))
#define TEST_PULSE(count) TestP1Cex0PulseSuccessThreshold(count)
#define IS_INDEXED_STATE(state) IsIndexedDriveStateActive(state)
#define LOWER_PARAMETER_F4() WriteControllerParameterF4WithDownAdjustment()
#define RUN_COMMAND_0E() RunControllerCommand0ETransaction()
#define APPLY_WORD_0204_AND_SETTLE() ApplyControllerWord0204CalibrationAndSettle()
#define UPDATE_OUTCOME(stage) UpdateCalibrationOutcomeStateForStage(stage)
#define CALIBRATE_WORD_0319() CalibrateControllerWord0319Offset()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
samples[index + FEEDBACK_COUNT] = (u16)(sum / 3u);
#else
samples[index + FEEDBACK_COUNT] = (u16)DivideS32Quotient((s32)(u32)sum, 3l);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
return (s16)(sum / 2);
#else
return DivideS16Quotient(sum, 2);
#endif
#endif
