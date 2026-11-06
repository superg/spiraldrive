#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveAcquireRangeWriteInitialParameter(u8 value);
extern void ObserveAcquireRangeArmPacketCapture(void);
extern u8 ObserveAcquireRangeParsePacket(void);
extern void ObserveAcquireRangeYieldCoroutine1(void);
extern void ObserveAcquireRangeEndMeasurement(void);
extern void ObserveAcquireRangeBeginMeasurement(void);
extern void ObserveAcquireRangeAdvanceOperatingStage(void);
#define ACQUIRE_RANGE_WRITE_INITIAL(value) ObserveAcquireRangeWriteInitialParameter(value)
#define ACQUIRE_RANGE_ARM() ObserveAcquireRangeArmPacketCapture()
#define ACQUIRE_RANGE_PARSE() ObserveAcquireRangeParsePacket()
#define ACQUIRE_RANGE_YIELD() ObserveAcquireRangeYieldCoroutine1()
#define ACQUIRE_RANGE_END() ObserveAcquireRangeEndMeasurement()
#define ACQUIRE_RANGE_BEGIN() ObserveAcquireRangeBeginMeasurement()
#define ACQUIRE_RANGE_ADVANCE() ObserveAcquireRangeAdvanceOperatingStage()
#else
extern void WriteControllerParameterEF00AndConditionalD0Value(u8 value);
extern void ArmControllerThreeBytePacketCapture(void);
extern u8 ParseControllerThreeBytePacket(void);
extern void YieldCoroutine1(void);
extern void EndReadRangeControllerMeasurement(void);
extern void BeginReadRangeControllerMeasurement(void);
extern void AdvanceOperatingUpdateStageAtFrameBoundaries(void);
#define ACQUIRE_RANGE_WRITE_INITIAL(value) WriteControllerParameterEF00AndConditionalD0Value(value)
#define ACQUIRE_RANGE_ARM() ArmControllerThreeBytePacketCapture()
#define ACQUIRE_RANGE_PARSE() ParseControllerThreeBytePacket()
#define ACQUIRE_RANGE_YIELD() YieldCoroutine1()
#define ACQUIRE_RANGE_END() EndReadRangeControllerMeasurement()
#define ACQUIRE_RANGE_BEGIN() BeginReadRangeControllerMeasurement()
#define ACQUIRE_RANGE_ADVANCE() AdvanceOperatingUpdateStageAtFrameBoundaries()
#endif
#endif
