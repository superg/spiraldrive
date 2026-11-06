#ifndef PLEXTOR_FLOAT64_RUNTIME_HELPERS_H
#define PLEXTOR_FLOAT64_RUNTIME_HELPERS_H

#include "mcs251.h"

double ScaleFloat64ByPowerOfTwo(double value, s16 scale);
double DecomposeFloat64MantissaAndExponent(double value, s16 *exponent);
u8 CompareFloat64(double left_value, double right_value);
double SquareRootFloat64(double value);
double AddFloat64(double left_value, double right_value);
double SubtractFloat64(double left_value, double right_value);
double MultiplyFloat64(double left_value, double right_value);
double DivideFloat64(double left_value, double right_value);

#endif
