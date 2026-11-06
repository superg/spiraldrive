#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveType2GeometryDivideS32(s32 dividend, s32 divisor);
extern u32 ObserveType2GeometryDivideU32Quotient(u32 dividend, u32 divisor);
extern u32 ObserveType2GeometryDivideU32Remainder(u32 dividend, u32 divisor);
extern u32 ObserveType2GeometryMultiplyU32(u32 factor_a, u32 factor_b);
#define DIVIDE_GEOMETRY_S32(a, b) ObserveType2GeometryDivideS32((a), (b))
#define DIVIDE_GEOMETRY_U32(a, b) ObserveType2GeometryDivideU32Quotient((a), (b))
#define REMAINDER_GEOMETRY_U32(a, b) ObserveType2GeometryDivideU32Remainder((a), (b))
#define MULTIPLY_GEOMETRY_U32(a, b) ObserveType2GeometryMultiplyU32((a), (b))
#else
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
extern u32 DivideU32RemainderNative(u32 dividend, u32 divisor);
extern u32 MultiplyU32Low(u32 factor_a, u32 factor_b);
#define DIVIDE_GEOMETRY_S32(a, b) DivideS32QuotientNative((a), (b))
#define DIVIDE_GEOMETRY_U32(a, b) DivideU32QuotientNative((a), (b))
#define REMAINDER_GEOMETRY_U32(a, b) DivideU32RemainderNative((a), (b))
#define MULTIPLY_GEOMETRY_U32(a, b) MultiplyU32Low((a), (b))
#endif
#endif
