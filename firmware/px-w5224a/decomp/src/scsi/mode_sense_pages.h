#ifndef PLEXTOR_MODE_SENSE_PAGES_H
#define PLEXTOR_MODE_SENSE_PAGES_H

#include "../platform/mcs251.h"

u32 PLEXTOR_NEAR_FUNCTION AppendModeSensePageToControllerBuffer(u32 response_address, u8 page_code, u8 page_control) PLEXTOR_REENTRANT;

#endif
