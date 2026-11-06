#ifndef PLEXTOR_CONFIGURATION_RANGE_UPDATE_H
#define PLEXTOR_CONFIGURATION_RANGE_UPDATE_H

#include "mcs251.h"

u8 UpdateAndProgramDriveConfigurationRange(fw_addr_t source, u16 destination_offset, u16 byte_count);

#endif
