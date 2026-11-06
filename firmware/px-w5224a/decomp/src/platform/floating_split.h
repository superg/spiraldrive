#ifndef PLEXTOR_FLOATING_SPLIT_H
#define PLEXTOR_FLOATING_SPLIT_H

#include "mcs251.h"

#if defined(PLEXTOR_HOST_TEST)
#define PLEXTOR_FLOATING_SPLIT_HUGE
#else
#define PLEXTOR_FLOATING_SPLIT_HUGE huge
#endif

double SplitFloat64FractionAndIntegral(double value, double PLEXTOR_FLOATING_SPLIT_HUGE *integral_output) PLEXTOR_REENTRANT;

#endif
