#ifndef PLEXTOR_MODE_PAGE2A_H
#define PLEXTOR_MODE_PAGE2A_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION UpdateModePage2ACapabilitiesRuntimeImage(void);
void PLEXTOR_NEAR_FUNCTION PopulateModePage2AWriteSpeedPerformanceDescriptors(u8 maximum_speed_x, u8 count_offset);

#endif
