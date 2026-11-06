#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveControllerPositionSampleArm(void);
extern u8 ObserveControllerPositionSampleParse(void);
extern void ObserveControllerPositionSampleYield(void);
extern u8 ObserveControllerPositionSampleEvaluate(s16 interior_reference);
#define POSITION_SAMPLE_ARM() ObserveControllerPositionSampleArm()
#define POSITION_SAMPLE_PARSE() ObserveControllerPositionSampleParse()
#define POSITION_SAMPLE_YIELD() ObserveControllerPositionSampleYield()
#define POSITION_SAMPLE_EVALUATE(reference) ObserveControllerPositionSampleEvaluate(reference)
#else
extern void YieldCoroutine1(void);
#define POSITION_SAMPLE_ARM() ArmControllerThreeBytePacketCapture()
#define POSITION_SAMPLE_PARSE() ParseControllerThreeBytePacket()
#define POSITION_SAMPLE_YIELD() YieldCoroutine1()
#define POSITION_SAMPLE_EVALUATE(reference) EvaluateControllerPositionSamplesAndUpdateCalibration(reference)
#endif
#endif
