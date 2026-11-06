#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveIndexedState0FYield(void);
extern void ObserveIndexedState0FInitializeCoroutine1(u32 entry_point);
extern void ObserveIndexedState0FResetTrackRecords(u8 first_index);
extern void ObserveIndexedState0FLoadTrack(u8 track_number);
extern u8 ObserveIndexedState0FExecute(void);
extern u8 ObserveIndexedState0FValidatePayloads(void);
extern void ObserveIndexedState0FTimedLoop(void);
extern void ObserveIndexedState0FYieldLoop(void);
#define INDEXED_YIELD() ObserveIndexedState0FYield()
#define INDEXED_INIT_COROUTINE1(entry) ObserveIndexedState0FInitializeCoroutine1(entry)
#define INDEXED_RESET_TRACKS(index) ObserveIndexedState0FResetTrackRecords(index)
#define INDEXED_LOAD_TRACK(number) ObserveIndexedState0FLoadTrack(number)
#define INDEXED_EXECUTE() ObserveIndexedState0FExecute()
#define INDEXED_VALIDATE() ObserveIndexedState0FValidatePayloads()
#define INDEXED_TIMED_LOOP() ObserveIndexedState0FTimedLoop()
#define INDEXED_YIELD_LOOP() ObserveIndexedState0FYieldLoop()
#define INDEXED_DIVIDE(dividend, divisor) ((dividend) / (divisor))
#define INDEXED_MULTIPLY(left, right) ((left) * (right))
#else
#include "platform/arithmetic.h"
#include "platform/coroutine_initialization.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#include "platform/timed_output_sequence.h"
extern void YieldCoroutine0(void);
extern void LoadTrackInformationByNumber(u8 track_number);
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
#define INDEXED_YIELD() YieldCoroutine0()
#define INDEXED_INIT_COROUTINE1(entry) InitializeCoroutine1(entry)
#define INDEXED_RESET_TRACKS(index) ResetTrackInformationRecordsFromIndex(index)
#define INDEXED_LOAD_TRACK(number) LoadTrackInformationByNumber(number)
#define INDEXED_EXECUTE() ExecuteIndexedState0FMediaOperation()
#define INDEXED_VALIDATE() ValidateIndexedState0FReferencePayloads()
#define INDEXED_TIMED_LOOP()                                                                                                                                                                           \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        for(;;)                                                                                                                                                                                        \
        {                                                                                                                                                                                              \
            ServiceIndexedState0FTimedOutputSequence();                                                                                                                                                \
            YieldCoroutine0();                                                                                                                                                                         \
        }                                                                                                                                                                                              \
    } while(0)
#define INDEXED_YIELD_LOOP()                                                                                                                                                                           \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        for(;;)                                                                                                                                                                                        \
            YieldCoroutine0();                                                                                                                                                                         \
    } while(0)
#define INDEXED_DIVIDE(dividend, divisor) DivideU32Quotient((dividend), (divisor))
#define INDEXED_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#endif
#endif
