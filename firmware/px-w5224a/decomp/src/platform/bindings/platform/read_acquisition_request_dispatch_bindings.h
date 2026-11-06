#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
#define SET_READ_ACQUISITION_P3_RX_HIGH_TX_LOW()                                                                                                                                                       \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareWriteBit0580(1u);                                                                                                                                                                      \
        FirmwareWriteBit0581(0u);                                                                                                                                                                      \
    } while(0)
#else
extern void PLEXTOR_NEAR_FUNCTION SetP3RxHighTxLowForReadAcquisitionDispatch(void);
#define SET_READ_ACQUISITION_P3_RX_HIGH_TX_LOW() SetP3RxHighTxLowForReadAcquisitionDispatch()
#endif
#endif
