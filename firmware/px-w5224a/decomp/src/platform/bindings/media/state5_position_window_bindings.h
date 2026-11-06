#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define DIVIDE_STATE5_POSITION(dividend, divisor) ((dividend) / (divisor))
#else
extern s32 DivideS32Quotient(s32 dividend, s32 divisor);
#define DIVIDE_STATE5_POSITION(dividend, divisor) DivideS32Quotient((dividend), (divisor))
#endif
#endif
