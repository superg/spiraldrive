#ifndef PLEXTOR_SCSI_READ_MASTER_CUE_STATUS_H
#define PLEXTOR_SCSI_READ_MASTER_CUE_STATUS_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION PollReadMasterCueRawReadStatus(void);
u8 PLEXTOR_NEAR_FUNCTION IsReadWindowOutsideAcceptedRange(void);

#endif
