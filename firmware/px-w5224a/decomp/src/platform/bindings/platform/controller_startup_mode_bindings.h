#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveControllerStartupModeDelay(void);
extern void ObserveControllerStartupModeHandshake(void);
extern void ObserveControllerStartupModeThreeTrap(void);
#define STARTUP_MODE_DELAY() ObserveControllerStartupModeDelay()
#define STARTUP_MODE_HANDSHAKE() ObserveControllerStartupModeHandshake()
#define STARTUP_MODE_THREE_TRAP() ObserveControllerStartupModeThreeTrap()
#else
extern void DelayTimerUnits59(void);
extern void PLEXTOR_NEAR_FUNCTION RunControllerStartupStatusHandshake(void);
#define STARTUP_MODE_DELAY() DelayTimerUnits59()
#define STARTUP_MODE_HANDSHAKE() RunControllerStartupStatusHandshake()
#define STARTUP_MODE_THREE_TRAP()                                                                                                                                                                      \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
    } while(1)
#endif
#endif
