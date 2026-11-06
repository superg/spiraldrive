#ifndef PLEXTOR_SCSI_VENDOR_MEASUREMENT_RESPONSE_H
#define PLEXTOR_SCSI_VENDOR_MEASUREMENT_RESPONSE_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION BuildVendorF2ShortControllerMeasurementResponse(void);
void PLEXTOR_NEAR_FUNCTION BuildVendorF2ControllerMeasurementResponseBody(void);
void PLEXTOR_NEAR_FUNCTION BuildVendorF2Mode2Or6MeasurementResponse(void);

#endif
