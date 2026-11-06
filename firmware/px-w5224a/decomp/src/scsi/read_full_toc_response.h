#ifndef PLEXTOR_SCSI_READ_FULL_TOC_RESPONSE_H
#define PLEXTOR_SCSI_READ_FULL_TOC_RESPONSE_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION ExecuteReadFullTocResponse(u8 use_packed_bcd_encoding);

#endif
