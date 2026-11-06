#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
static s32 DivideS32QuotientNative(s32 dividend, s32 divisor)
{
    return dividend / divisor;
}
#else
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#endif
#endif
