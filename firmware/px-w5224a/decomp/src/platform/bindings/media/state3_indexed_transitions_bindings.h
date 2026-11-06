#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveMediaState3IndexedTransitionOperation(u8 operation, u32 argument);
#define MEDIA_STATE3_INDEXED_CALL(operation, argument) ObserveMediaState3IndexedTransitionOperation((operation), (argument))
#else
#include "media/decoder.h"
#include "media/state3_countdown.h"
#include "platform/coroutine_initialization.h"
#include "platform/leaf_workers.h"
extern void RestartCoroutine1AndYield(u32 entry_point);
#define MEDIA_STATE3_INDEXED_NATIVE_CALL_I(operation, argument) MEDIA_STATE3_INDEXED_NATIVE_CALL_##operation(argument)
#define MEDIA_STATE3_INDEXED_NATIVE_CALL(operation, argument) MEDIA_STATE3_INDEXED_NATIVE_CALL_I(operation, argument)
#define MEDIA_STATE3_INDEXED_CALL(operation, argument) MEDIA_STATE3_INDEXED_NATIVE_CALL(operation, argument)
#define MEDIA_STATE3_INDEXED_NATIVE_CALL_MEDIA_STATE3_INDEXED_DISPATCH(argument) DispatchIndexedState()
#define MEDIA_STATE3_INDEXED_NATIVE_CALL_MEDIA_STATE3_INDEXED_RESTART_COROUTINE1(argument) RestartCoroutine1AndYield((u32)(argument))
#define MEDIA_STATE3_INDEXED_NATIVE_CALL_MEDIA_STATE3_INDEXED_ADVANCE(argument) AdvanceMediaState3IndexedTransitionState()
#define MEDIA_STATE3_INDEXED_NATIVE_CALL_MEDIA_STATE3_INDEXED_SERVICE_COUNTDOWN(argument) ServiceMediaState3CountdownAndTransition()
#define MEDIA_STATE3_INDEXED_NATIVE_CALL_MEDIA_STATE3_INDEXED_SERVICE_CALIBRATION(argument) ServiceMediaState3ControllerCalibrationTransition()
#define MEDIA_STATE3_INDEXED_NATIVE_CALL_MEDIA_STATE3_INDEXED_INITIALIZE_RANGE(argument) InitializeReadAcquisitionRangeFromLastTocTrack()
#define MEDIA_STATE3_INDEXED_NATIVE_CALL_MEDIA_STATE3_INDEXED_INITIALIZE_COROUTINE0(argument) InitializeCoroutine0((u32)(argument))
#endif
#endif
