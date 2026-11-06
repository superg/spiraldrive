#include "floating_result_tails.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/floating_result_tails_bindings.h"
#undef PLEXTOR_BINDING_SECTION

typedef union
{
    double value;
    u8 bytes[8];
} Float64ConstantResult;

/* Reconstruction helper; index zero is the most-significant firmware byte. */
static void WriteFloat64ConstantByte(Float64ConstantResult *result, u8 index, u8 byte)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/floating_result_tails_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:fd9295 */
u32 ReturnFloat32NaN(void)
{
    return 0xfffffffful;
}

/* RAM:fd929a */
u32 ReturnFloat32Zero(void)
{
    return 0ul;
}

/* RAM:fd929d */
/* The native runtime supplies the sign in PSW.F0; the C boundary makes that
 * input explicit. */
u32 ReturnFloat32InfinityFromSignFlag(u8 negative)
{
    return negative != 0u ? 0xff800000ul : 0x7f800000ul;
}

/* RAM:fd9231 */
double ReturnFloat64NaN(void)
{
    Float64ConstantResult result;
    u8 index;

    for(index = 0; index != 8u; ++index)
        WriteFloat64ConstantByte(&result, index, 0xffu);
    return result.value;
}

/* RAM:fd9238 */
double ReturnFloat64Zero(void)
{
    return 0.0;
}

/* RAM:fd923d */
/* The native runtime supplies the sign in PSW.F0; the C boundary makes that
 * input explicit. */
double ReturnFloat64InfinityFromSignFlag(u8 negative)
{
    Float64ConstantResult result;
    u8 index;

    for(index = 0; index != 8u; ++index)
        WriteFloat64ConstantByte(&result, index, 0u);
    WriteFloat64ConstantByte(&result, 0u, negative != 0u ? 0xffu : 0x7fu);
    WriteFloat64ConstantByte(&result, 1u, 0xf0u);
    return result.value;
}

/* RAM:f849f3 */
double CeilFloat64(double value)
{
    value = NEGATE_FLOAT64_FOR_CEIL(value);
    value = FLOOR_FLOAT64_FOR_CEIL(value);
    return NEGATE_FLOAT64_FOR_CEIL(value);
}
