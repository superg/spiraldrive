#include "floating_split.h"

/* RAM:f9ff54 */
/* The original compiler helper truncates through a signed 32-bit integer. */
double SplitFloat64FractionAndIntegral(double value, double PLEXTOR_FLOATING_SPLIT_HUGE *integral_output) PLEXTOR_REENTRANT
{
    double integral_value = (double)(s32)value;

    *integral_output = integral_value;
    return value - integral_value;
}
