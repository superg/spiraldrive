#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveMipControllerRecordSnapshotDivide(s32 dividend, s32 divisor);
#define DIVIDE_SNAPSHOT_POSITION(dividend, divisor) ObserveMipControllerRecordSnapshotDivide((dividend), (divisor))
#else
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#define DIVIDE_SNAPSHOT_POSITION(dividend, divisor) DivideS32QuotientNative((dividend), (divisor))
#endif
#endif
