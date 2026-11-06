#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void SetMeasurementRetryP3RxHighTxLow(void);
extern void SetMeasurementRetryP3RxLowTxHigh(void);
#else
extern void PLEXTOR_NEAR_FUNCTION SetP3RxHighTxLowForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION SetP3RxLowTxHighForReadAcquisitionCoroutine3(void);
#define SetMeasurementRetryP3RxHighTxLow SetP3RxHighTxLowForReadAcquisitionCoroutine3
#define SetMeasurementRetryP3RxLowTxHigh SetP3RxLowTxHighForReadAcquisitionCoroutine3
#endif
#endif
