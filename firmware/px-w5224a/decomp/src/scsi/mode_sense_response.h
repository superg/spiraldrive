#ifndef PLEXTOR_MODE_SENSE_RESPONSE_H
#define PLEXTOR_MODE_SENSE_RESPONSE_H

#include "../platform/mcs251.h"

void ExecuteModeSense6Or10Response(u8 cdb_length, u16 allocation_length);

#endif
