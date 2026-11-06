#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveMediaCoroutineState6ReadCompletionBit(void);
extern void ObserveMediaCoroutineState6WriteCompletionBit(u8 value);
extern void ObserveMediaCoroutineState6WriteP3Bit0(u8 value);
extern void ObserveMediaCoroutineState6WriteP3Bit1(u8 value);
extern u8 ObserveMediaCoroutineState6ReduceSpeed(u8 enabled);
extern void ObserveMediaCoroutineState6MeasurementRetry(void);
extern u8 ObserveMediaCoroutineState6Calibration(void);
extern void ObserveMediaCoroutineState6ClearFlags(void);
extern u8 ObserveMediaCoroutineState6AcquirePosition(void);
extern void ObserveMediaCoroutineState6ApplyOffset(void);
extern void ObserveMediaCoroutineState6Yield(void);
#define STATE6_READ_COMPLETION_BIT() ObserveMediaCoroutineState6ReadCompletionBit()
#define STATE6_WRITE_COMPLETION_BIT(value) ObserveMediaCoroutineState6WriteCompletionBit(value)
#define STATE6_WRITE_P3_BIT0(value) ObserveMediaCoroutineState6WriteP3Bit0(value)
#define STATE6_WRITE_P3_BIT1(value) ObserveMediaCoroutineState6WriteP3Bit1(value)
#define STATE6_REDUCE_SPEED(value) ObserveMediaCoroutineState6ReduceSpeed(value)
#define STATE6_MEASUREMENT_RETRY() ObserveMediaCoroutineState6MeasurementRetry()
#define STATE6_CALIBRATE() ObserveMediaCoroutineState6Calibration()
#define STATE6_CLEAR_FLAGS() ObserveMediaCoroutineState6ClearFlags()
#define STATE6_ACQUIRE_POSITION() ObserveMediaCoroutineState6AcquirePosition()
#define STATE6_APPLY_OFFSET() ObserveMediaCoroutineState6ApplyOffset()
#define STATE6_YIELD() ObserveMediaCoroutineState6Yield()
#else
#include "platform/controller_measurement_retry_sequence.h"
#include "platform/leaf_workers.h"
#include "platform/media_state6_controller_calibration_sequence.h"
#include "platform/status_mapping.h"
#include "platform/write_speed_reduction.h"
extern void YieldCoroutine1(void);
u8 bdata MediaCoroutineState6BitByte27 _at_ 0x27;
sbit MediaCoroutineState6CompletionBit = MediaCoroutineState6BitByte27 ^ 1;
sbit MediaCoroutineState6P3Bit0 = 0xb0 ^ 0;
sbit MediaCoroutineState6P3Bit1 = 0xb0 ^ 1;
#define STATE6_READ_COMPLETION_BIT() MediaCoroutineState6CompletionBit
#define STATE6_WRITE_COMPLETION_BIT(value) (MediaCoroutineState6CompletionBit = (value))
#define STATE6_WRITE_P3_BIT0(value) (MediaCoroutineState6P3Bit0 = (value))
#define STATE6_WRITE_P3_BIT1(value) (MediaCoroutineState6P3Bit1 = (value))
#define STATE6_REDUCE_SPEED(value) TryReduceWriteSpeedForCurrentMediaState(value)
#define STATE6_MEASUREMENT_RETRY() RunMediaState6ControllerMeasurementRetrySequence()
#define STATE6_CALIBRATE() ExecuteMediaState6ControllerCalibrationSequence()
#define STATE6_CLEAR_FLAGS() ClearMediaStateFlags7Through12()
#define STATE6_ACQUIRE_POSITION() AcquireDiscReadPositionWithControllerReinitializationRetry()
#define STATE6_APPLY_OFFSET() ApplyMediaCalibrationOffsetForCurrentWriteSpeed()
#define STATE6_YIELD() YieldCoroutine1()
#endif
#endif
