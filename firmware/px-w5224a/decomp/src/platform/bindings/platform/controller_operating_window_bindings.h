#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s16 ObserveCaptureControllerReadPathOperatingSampleAtomic(void);
#define CAPTURE_OPERATING_SAMPLE() ObserveCaptureControllerReadPathOperatingSampleAtomic()
#else
extern s16 PLEXTOR_NEAR_FUNCTION CaptureControllerReadPathOperatingSampleAtomic(void);
#define CAPTURE_OPERATING_SAMPLE() CaptureControllerReadPathOperatingSampleAtomic()
#endif
#endif
