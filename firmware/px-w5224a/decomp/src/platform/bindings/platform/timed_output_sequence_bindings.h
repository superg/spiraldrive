#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadCountdownForTimedOutputSequence(void);
extern void ObserveSetP3Bits0And1HighForTimedOutputSequence(void);
extern void ObserveApplyTimedOutputStateFromIndexBits(void);
extern void ObserveSetTimedOutputLinesHigh(void);
#define READ_TIMED_OUTPUT_COUNTDOWN() ObserveReadCountdownForTimedOutputSequence()
#define SET_TIMED_OUTPUT_LINES_DIRECT_HIGH() ObserveSetP3Bits0And1HighForTimedOutputSequence()
#define APPLY_TIMED_OUTPUT_STATE() ObserveApplyTimedOutputStateFromIndexBits()
#define SET_TIMED_OUTPUT_LINES_HIGH() ObserveSetTimedOutputLinesHigh()
#else
extern void SetP3Bits0And1HighForTimedOutputSequence(void);
extern void ApplyTimedOutputStateFromIndexBits(void);
extern void SetTimedOutputLinesHigh(void);
#define READ_TIMED_OUTPUT_COUNTDOWN() FirmwareReadNear8(FW_NEAR_UNKNOWN_121A)
#define SET_TIMED_OUTPUT_LINES_DIRECT_HIGH() SetP3Bits0And1HighForTimedOutputSequence()
#define APPLY_TIMED_OUTPUT_STATE() ApplyTimedOutputStateFromIndexBits()
#define SET_TIMED_OUTPUT_LINES_HIGH() SetTimedOutputLinesHigh()
#endif
#endif
