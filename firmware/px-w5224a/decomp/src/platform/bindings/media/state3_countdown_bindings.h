#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveConvertState3CountdownTrackPosition(fw_addr_t tuple_address);
extern void ObserveRestartState3CountdownCoroutine1(void);
#define CONVERT_STATE3_COUNTDOWN_TRACK_POSITION(address) ObserveConvertState3CountdownTrackPosition(address)
#define RESTART_STATE3_COUNTDOWN_COROUTINE1() ObserveRestartState3CountdownCoroutine1()
#else
extern void RestartCoroutine1AtF84B0BAndYield(void);
#define CONVERT_STATE3_COUNTDOWN_TRACK_POSITION(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define RESTART_STATE3_COUNTDOWN_COROUTINE1() RestartCoroutine1AtF84B0BAndYield()
#endif
#endif
