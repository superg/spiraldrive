#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "platform/floating_floor.h"

static void TestFiniteValues(void)
{
    static const double values[] = { -9007199254740992.0, -123456.75, -2.0, -1.5, -1.0, -0.75, -0.0, 0.0, 0.25, 1.0, 1.5, 123456.75, 9007199254740992.0 };
    u8 index;

    for(index = 0u; index != sizeof(values) / sizeof(values[0]); ++index)
        assert(FloorFloat64(values[index]) == floor(values[index]));
    assert(signbit(FloorFloat64(-0.0)) != 0);
}

static void TestExceptionalValues(void)
{
    union
    {
        double value;
        uint64_t bits;
    } canonical_nan;
    double positive_infinity = INFINITY;
    double negative_infinity = -INFINITY;
    double not_a_number = NAN;

    canonical_nan.bits = UINT64_MAX;
    assert(FloorFloat64(positive_infinity) == positive_infinity);
    assert(FloorFloat64(negative_infinity) == negative_infinity);
    assert(isnan(FloorFloat64(not_a_number)));
    assert(FloorFloat64(canonical_nan.value) == -1.0);
}

int main(void)
{
    TestFiniteValues();
    TestExceptionalValues();
    puts("floating floor tests passed");
    return 0;
}
