#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR15ForMeasurementEvaluation(void);
extern void ObserveEvaluateControllerMeasurementSeries00And01(void);
extern void ObserveTransformControllerMeasurementSeries04To05(void);
extern void ObserveTransformControllerMeasurementSeries04To05ForMode6(void);
extern void ObserveSmoothAndEvaluateControllerMeasurementSeries02(void);
#define CAPTURE_MEASUREMENT_FALLBACK() ObserveCaptureInheritedR15ForMeasurementEvaluation()
#define EVALUATE_MEASUREMENT_SERIES00_AND01() ObserveEvaluateControllerMeasurementSeries00And01()
#define TRANSFORM_MEASUREMENT_SERIES04_TO05() ObserveTransformControllerMeasurementSeries04To05()
#define TRANSFORM_MEASUREMENT_SERIES04_TO05_MODE6() ObserveTransformControllerMeasurementSeries04To05ForMode6()
#define SMOOTH_MEASUREMENT_SERIES02() ObserveSmoothAndEvaluateControllerMeasurementSeries02()
extern void ObserveArmTargetWindowPacketCapture(void);
extern u8 ObserveParseTargetWindowPacket(void);
extern void ObserveCompleteTargetWindowMeasurement(void);
#define ARM_TARGET_WINDOW_PACKET_CAPTURE() ObserveArmTargetWindowPacketCapture()
#define PARSE_TARGET_WINDOW_PACKET() ObserveParseTargetWindowPacket()
#define COMPLETE_TARGET_WINDOW_MEASUREMENT() ObserveCompleteTargetWindowMeasurement()
#else
extern u8 CaptureInheritedR15ForMeasurementEvaluation(void);
extern void PLEXTOR_NEAR_FUNCTION EvaluateControllerMeasurementSeries00And01(void);
extern void PLEXTOR_NEAR_FUNCTION TransformControllerMeasurementSeries04To05(void);
extern void PLEXTOR_NEAR_FUNCTION TransformControllerMeasurementSeries04To05ForMode6(void);
extern void PLEXTOR_NEAR_FUNCTION SmoothAndEvaluateControllerMeasurementSeries02(void);
#define CAPTURE_MEASUREMENT_FALLBACK() CaptureInheritedR15ForMeasurementEvaluation()
#define EVALUATE_MEASUREMENT_SERIES00_AND01() EvaluateControllerMeasurementSeries00And01()
#define TRANSFORM_MEASUREMENT_SERIES04_TO05() TransformControllerMeasurementSeries04To05()
#define TRANSFORM_MEASUREMENT_SERIES04_TO05_MODE6() TransformControllerMeasurementSeries04To05ForMode6()
#define SMOOTH_MEASUREMENT_SERIES02() SmoothAndEvaluateControllerMeasurementSeries02()
extern void ArmControllerThreeBytePacketCapture(void);
extern u8 ParseControllerThreeBytePacket(void);
extern void CompleteControllerMeasurementSequence(void);
#define ARM_TARGET_WINDOW_PACKET_CAPTURE() ArmControllerThreeBytePacketCapture()
#define PARSE_TARGET_WINDOW_PACKET() ParseControllerThreeBytePacket()
#define COMPLETE_TARGET_WINDOW_MEASUREMENT() CompleteControllerMeasurementSequence()
#endif
#endif
