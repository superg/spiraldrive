#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveState6PositionWindowArm(void);
extern void ObserveState6PositionWindowWriteE8E9(u8 value);
extern u8 ObserveState6PositionWindowPoll(void);
extern u8 ObserveState6PositionWindowOperatingWindow(u8 wide);
extern void ObserveState6PositionWindowRestore(u8 refresh);
extern u8 ObserveState6PositionWindowReadiness(void);
extern void ObserveState6PositionWindowYield(void);
extern void ObserveState6PositionWindowApplyParameters(void);
#define WINDOW_ARM() ObserveState6PositionWindowArm()
#define WINDOW_WRITE_E8E9(value) ObserveState6PositionWindowWriteE8E9(value)
#define WINDOW_POLL() ObserveState6PositionWindowPoll()
#define WINDOW_OPERATING(wide) ObserveState6PositionWindowOperatingWindow(wide)
#define WINDOW_RESTORE(refresh) ObserveState6PositionWindowRestore(refresh)
#define WINDOW_READINESS() ObserveState6PositionWindowReadiness()
#define WINDOW_YIELD() ObserveState6PositionWindowYield()
#define WINDOW_APPLY_PARAMETERS() ObserveState6PositionWindowApplyParameters()
#else
extern void YieldCoroutine1(void);
#define WINDOW_ARM() ArmControllerThreeBytePacketCapture()
#define WINDOW_WRITE_E8E9(value) WriteControllerParametersE8E9ForValue(value)
#define WINDOW_POLL() NormalizeControllerMeasurementPollStatus()
#define WINDOW_OPERATING(wide) IsControllerReadPathWithinOperatingWindow(wide)
#define WINDOW_RESTORE(refresh) RestoreControllerReadPathConfiguration(refresh)
#define WINDOW_READINESS() RunControllerReadPathReadinessProbe()
#define WINDOW_YIELD() YieldCoroutine1()
#define WINDOW_APPLY_PARAMETERS() ApplyControllerOperatingParametersForState1Path()
#endif
#endif
