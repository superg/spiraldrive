#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveState6FinalizationPulse(u8 sample_count);
extern void ObserveState6FinalizationLoadProfile(u8 profile_index);
extern void ObserveState6FinalizationConfigurePort8006(u8 enabled);
extern void ObserveState6FinalizationApplyChannels(void);
extern void ObserveState6FinalizationConfigureRange(void);
extern void ObserveState6FinalizationYield(void);
extern void ObserveState6FinalizationStrobe(fw_addr_t address, u8 value);
extern void ObserveState6FinalizationConfigureP0(u8 enabled);
extern void ObserveState6FinalizationWaitForTolerance(u8 wait_for_initial_advance, u8 lower_bound_only);
extern u8 ObserveState6FinalizationIsIndexed(u8 state);
extern void ObserveState6FinalizationUpdateOutcome(u8 stage);
extern u8 ObserveState6FinalizationReadP0Bit7(void);
#define FINAL_PULSE(count) ObserveState6FinalizationPulse(count)
#define FINAL_LOAD_PROFILE(index) ObserveState6FinalizationLoadProfile(index)
#define FINAL_CONFIGURE_PORT8006(enabled) ObserveState6FinalizationConfigurePort8006(enabled)
#define FINAL_APPLY_CHANNELS() ObserveState6FinalizationApplyChannels()
#define FINAL_CONFIGURE_RANGE() ObserveState6FinalizationConfigureRange()
#define FINAL_YIELD() ObserveState6FinalizationYield()
#define FINAL_STROBE(address, value) ObserveState6FinalizationStrobe((address), (value))
#define FINAL_CONFIGURE_P0(enabled) ObserveState6FinalizationConfigureP0(enabled)
#define FINAL_WAIT_FOR_TOLERANCE(initial, lower) ObserveState6FinalizationWaitForTolerance((initial), (lower))
#define FINAL_IS_INDEXED(state) ObserveState6FinalizationIsIndexed(state)
#define FINAL_UPDATE_OUTCOME(stage) ObserveState6FinalizationUpdateOutcome(stage)
#define FINAL_READ_P0_BIT7() ObserveState6FinalizationReadP0Bit7()
#else
#include "platform/controller_calibration_outcome.h"
#include "platform/controller_parameters.h"
#include "platform/controller_transition_wait.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void YieldCoroutine1(void);
#define FINAL_PULSE(count) TestP1Cex0PulseSuccessThreshold(count)
#define FINAL_LOAD_PROFILE(index) LoadControllerOperatingProfile(index)
#define FINAL_CONFIGURE_PORT8006(enabled) ConfigureControllerPort8006AndChannel3(enabled)
#define FINAL_APPLY_CHANNELS() ApplyCurrentOperatingCodeToControllerChannels0Through3()
#define FINAL_CONFIGURE_RANGE() ConfigureControllerPorts8496Through8499ForOperatingRange()
#define FINAL_YIELD() YieldCoroutine1()
#define FINAL_STROBE(address, value) StrobeControllerPortWriteByteFromC((address), (value))
#define FINAL_CONFIGURE_P0(enabled) ConfigureP0Bit3AndControllerParametersE7E5(enabled)
#define FINAL_WAIT_FOR_TOLERANCE(initial, lower) WaitForControllerMeasurementWithinToleranceOrExpiry((initial), (lower))
#define FINAL_IS_INDEXED(state) IsIndexedDriveStateActive(state)
#define FINAL_UPDATE_OUTCOME(stage) UpdateCalibrationOutcomeStateForStage(stage)
#define FINAL_READ_P0_BIT7() FirmwareReadBit0407()
#endif
#endif
