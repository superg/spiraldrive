#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveCaptureInheritedDR12ForProgressBucket(void);
extern void ObserveSubmitProgressBucketControllerWord(u16 value, u16 selector);
#define CAPTURE_PROGRESS_DR12() ObserveCaptureInheritedDR12ForProgressBucket()
#define SUBMIT_PROGRESS_WORD(value, selector) ObserveSubmitProgressBucketControllerWord((value), (selector))
#else
extern u32 CaptureInheritedDR12ForProgressBucket(void);
/* C251 places the first word argument in WR6 and the second in WR4. */
extern void SubmitControllerWordPair(u16 selector, u16 value);
#define CAPTURE_PROGRESS_DR12() CaptureInheritedDR12ForProgressBucket()
#define SUBMIT_PROGRESS_WORD(value, selector) SubmitControllerWordPair((selector), (value))
#endif
#endif
