#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveStrategyProfileMultiply(u32 factor_a, u32 factor_b);
extern void ObserveStrategyProfileWriteSpeed(u8 write_speed_x);
#define STRATEGY_MULTIPLY(left, right) ObserveStrategyProfileMultiply((left), (right))
#define STRATEGY_APPLY_SPEED(speed) ObserveStrategyProfileWriteSpeed(speed)
#else
#define STRATEGY_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define STRATEGY_APPLY_SPEED(speed) ReplaceWriteSpeedSelectionFlagForValue(speed)
#endif
#endif
