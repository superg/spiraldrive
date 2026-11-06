#ifndef PLEXTOR_CONTROLLER_MEASUREMENT_WINDOW_H
#define PLEXTOR_CONTROLLER_MEASUREMENT_WINDOW_H

void BeginReadRangeControllerMeasurement(void);
void ComputeProcessedSfrDbWindowValue(void);
void RecomputeMediaState5ControllerThreshold(void);
void ResetReadMeasurementState(void);

#endif
