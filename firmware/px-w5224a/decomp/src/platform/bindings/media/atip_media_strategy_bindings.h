#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveAtipStrategyValidate(void);
extern void ObserveAtipStrategyResetDrive(void);
extern void ObserveAtipStrategyRestart(u32 entry_point);
extern void ObserveAtipStrategyInitialize(void);
#define VALIDATE_ATIP_STRATEGY() ObserveAtipStrategyValidate()
#define RESET_ATIP_STRATEGY_DRIVE() ObserveAtipStrategyResetDrive()
#define RESTART_ATIP_STRATEGY(entry) ObserveAtipStrategyRestart(entry)
#define INITIALIZE_ATIP_STRATEGY() ObserveAtipStrategyInitialize()
#else
extern u8 ValidateAtipMediaClassAgainstCurrentMediaState(void);
extern void ResetDriveControlStateToEight(void);
extern void RestartCoroutine1AndYield(u32 entry_point);
extern void InitializeMediaStrategyProfileAndWriteSpeedSelection(void);
#define VALIDATE_ATIP_STRATEGY() ValidateAtipMediaClassAgainstCurrentMediaState()
#define RESET_ATIP_STRATEGY_DRIVE() ResetDriveControlStateToEight()
#define RESTART_ATIP_STRATEGY(entry) RestartCoroutine1AndYield(entry)
#define INITIALIZE_ATIP_STRATEGY() InitializeMediaStrategyProfileAndWriteSpeedSelection()
#endif
#endif
