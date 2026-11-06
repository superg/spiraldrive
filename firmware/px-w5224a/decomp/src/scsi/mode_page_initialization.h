#ifndef PLEXTOR_MODE_PAGE_INITIALIZATION_H
#define PLEXTOR_MODE_PAGE_INITIALIZATION_H

#include "../platform/mcs251.h"

void InitializeModePageStateFromFirmwareDefaults(void);
void PLEXTOR_NEAR_FUNCTION ApplyModePage05RuntimeImageToWriteState(void);

#endif
