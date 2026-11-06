#include <assert.h>
#include <stdio.h>
#include "platform/floating_split.h"

static void CheckSplit(double value, double expected_fraction, double expected_integral)
{
    double integral = 12345.0;
    double fraction = SplitFloat64FractionAndIntegral(value, &integral);

    assert(integral == expected_integral);
    assert(fraction == expected_fraction);
}

int main(void)
{
    CheckSplit(12.75, 0.75, 12.0);
    CheckSplit(-12.75, -0.75, -12.0);
    CheckSplit(0.5, 0.5, 0.0);
    CheckSplit(-0.5, -0.5, 0.0);
    CheckSplit(1024.0, 0.0, 1024.0);

    puts("floating_split_tests: ok");
    return 0;
}
