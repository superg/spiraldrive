#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveReadAcquisitionControllerGlobalInterrupt(u8 enabled);
extern void ObserveReadAcquisitionControllerInterruptEnable(void);
extern void ObserveReadAcquisitionControllerSetSfrA9Bit2(void);
#define DISABLE_GLOBAL_INTERRUPT() ObserveReadAcquisitionControllerGlobalInterrupt(0u)
#define ENABLE_GLOBAL_INTERRUPT() ObserveReadAcquisitionControllerGlobalInterrupt(1u)
#define ENABLE_CONTROLLER_INTERRUPT() ObserveReadAcquisitionControllerInterruptEnable()
#define SET_SFR_A9_BIT2() ObserveReadAcquisitionControllerSetSfrA9Bit2()
#else
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
#define DISABLE_GLOBAL_INTERRUPT() ClearBit0547Atomic()
#define ENABLE_GLOBAL_INTERRUPT() SetBit0547Atomic()
#define ENABLE_CONTROLLER_INTERRUPT() SetBit0546Atomic()
#define SET_SFR_A9_BIT2() SetSfrA9Bit2Atomic()
#endif
#endif
