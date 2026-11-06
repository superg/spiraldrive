#ifndef PLEXTOR_SCSI_READ_HEADER_ACQUISITION_H
#define PLEXTOR_SCSI_READ_HEADER_ACQUISITION_H

#include "../platform/mcs251.h"

#if defined(PLEXTOR_HOST_TEST)
void ExecuteReadHeaderSectorAcquisitionForTest(void);
#else
void ExecuteReadHeaderSectorAcquisition(void);
#endif
u8 PollReadHeaderSectorAndTransferResponse(void);

#endif
