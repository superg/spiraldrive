#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveReadTrackInformationFreeDivide(u32 dividend, u32 divisor);
extern u32 ObserveReadTrackInformationFreeMultiply(u32 factor_a, u32 factor_b);
#define READ_TRACK_FREE_DIVIDE(dividend, divisor) ObserveReadTrackInformationFreeDivide((dividend), (divisor))
#define READ_TRACK_FREE_MULTIPLY(factor_a, factor_b) ObserveReadTrackInformationFreeMultiply((factor_a), (factor_b))
#else
#include "platform/arithmetic.h"
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
#define READ_TRACK_FREE_DIVIDE(dividend, divisor) DivideU32Quotient((dividend), (divisor))
#define READ_TRACK_FREE_MULTIPLY(factor_a, factor_b) MultiplyU32Low((factor_a), (factor_b))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u32 value = (u32)FirmwareReadNear8(address) << 24;
value |= (u32)FirmwareReadNear8((u16)(address + 1u)) << 16;
value |= (u32)FirmwareReadNear8((u16)(address + 2u)) << 8;
value |= FirmwareReadNear8((u16)(address + 3u));
return value;
#else
return *((volatile u32 near *)address);
#endif
#endif
