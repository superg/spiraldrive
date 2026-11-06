#ifndef PLEXTOR_FLOATING_WIDTH_CONVERSIONS_H
#define PLEXTOR_FLOATING_WIDTH_CONVERSIONS_H

#include "mcs251.h"

double ConvertFloat32ToFloat64(float value);
float ConvertFloat64ToFloat32(double value);
double NegateFloat64(double value);
s32 ConvertFloat64ToInteger32(double value);
double ConvertInteger32ToFloat64(s32 value);
double ConvertInteger16ToFloat64(s16 value);
double ConvertInteger8ToFloat64(s8 value);

#endif
