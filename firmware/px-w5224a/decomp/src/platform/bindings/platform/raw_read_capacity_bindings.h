#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRestartReadAcquisitionCoroutineForProducerWait(void);
#define RESTART_RAW_READ_ACQUISITION() ObserveRestartReadAcquisitionCoroutineForProducerWait()
#else
extern u8 RestartReadAcquisitionCoroutineForProducerWait(void);
#define RESTART_RAW_READ_ACQUISITION() RestartReadAcquisitionCoroutineForProducerWait()
#endif
#endif
