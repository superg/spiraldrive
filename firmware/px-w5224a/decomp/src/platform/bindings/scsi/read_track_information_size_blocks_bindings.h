#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveReadTrackInformationSizeDivide(u32 dividend, u32 divisor);
extern u32 ObserveReadTrackInformationSizeMultiply(u32 factor_a, u32 factor_b);
#define READ_TRACK_SIZE_DIVIDE(dividend, divisor) ObserveReadTrackInformationSizeDivide((dividend), (divisor))
#define READ_TRACK_SIZE_MULTIPLY(factor_a, factor_b) ObserveReadTrackInformationSizeMultiply((factor_a), (factor_b))
#else
#include "platform/arithmetic.h"
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
#define READ_TRACK_SIZE_DIVIDE(dividend, divisor) DivideU32Quotient((dividend), (divisor))
#define READ_TRACK_SIZE_MULTIPLY(factor_a, factor_b) MultiplyU32Low((factor_a), (factor_b))
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
