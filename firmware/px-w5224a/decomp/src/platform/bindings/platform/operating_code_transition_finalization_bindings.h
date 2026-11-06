#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveOperatingCodeTransitionSynchronize(void);
extern void ObserveOperatingCodeTransitionWritePort(fw_addr_t address, u8 value);
extern void ObserveOperatingCodeTransitionApplyProfile(u8 direction);
extern void ObserveOperatingCodeTransitionWait(u8 wait_for_initial_advance, u8 lower_bound_only);
extern u8 ObserveOperatingCodeTransitionMapProfileIndex(u8 operating_code);
#define SYNCHRONIZE_OPERATING_CODE() ObserveOperatingCodeTransitionSynchronize()
#define WRITE_OPERATING_CODE_PORT(address, value) ObserveOperatingCodeTransitionWritePort((address), (value))
#define APPLY_OPERATING_CODE_PROFILE(direction) ObserveOperatingCodeTransitionApplyProfile(direction)
#define WAIT_FOR_OPERATING_CODE_TRANSITION(advance, lower_only) ObserveOperatingCodeTransitionWait((advance), (lower_only))
#define MAP_OPERATING_CODE_PROFILE_INDEX(code) ObserveOperatingCodeTransitionMapProfileIndex(code)
#else
#define SYNCHRONIZE_OPERATING_CODE() SynchronizeLowOperatingCodeState()
#define WRITE_OPERATING_CODE_PORT(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define APPLY_OPERATING_CODE_PROFILE(direction) ApplyControllerMeasurementReferenceAndTransitionProfile()
#define WAIT_FOR_OPERATING_CODE_TRANSITION(advance, lower_only) WaitForControllerMeasurementWithinToleranceOrExpiry((advance), (lower_only))
#define MAP_OPERATING_CODE_PROFILE_INDEX(code) MapOperatingCodeToProfileIndexMinusSeven(code)
#endif
#endif
