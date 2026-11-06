#ifndef PLEXTOR_FLOATING_RESULT_TAILS_H
#define PLEXTOR_FLOATING_RESULT_TAILS_H

#include "mcs251.h"

u32 ReturnFloat32NaN(void);
u32 ReturnFloat32Zero(void);
u32 ReturnFloat32InfinityFromSignFlag(u8 negative);
double ReturnFloat64NaN(void);
double ReturnFloat64Zero(void);
double ReturnFloat64InfinityFromSignFlag(u8 negative);
double CeilFloat64(double value);

#endif
