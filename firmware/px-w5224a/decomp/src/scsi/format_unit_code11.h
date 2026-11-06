#ifndef PLEXTOR_SCSI_FORMAT_UNIT_CODE11_H
#define PLEXTOR_SCSI_FORMAT_UNIT_CODE11_H

#include "../platform/mcs251.h"

u8 ParseFormatUnitCode11ParameterList(void);
void HandleExpiredFormatUnitCode11PrimaryCountdown(void);

#endif
