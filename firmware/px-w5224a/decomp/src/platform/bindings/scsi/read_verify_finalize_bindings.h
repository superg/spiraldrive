#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveInvalidReadVerifyFinalizeMode(void);
#define FAIL_STOP_INVALID_MODE() ObserveInvalidReadVerifyFinalizeMode()
#define ReadVerifyFinalizeFunction FinalizeReadVerifyCommandStateForTest
#else
#define FAIL_STOP_INVALID_MODE()                                                                                                                                                                       \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
    } while(1)
#define ReadVerifyFinalizeFunction FinalizeReadVerifyCommandState
#endif
#endif
