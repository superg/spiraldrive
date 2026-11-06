#ifndef PLEXTOR_MEDIA_READ_ACQUISITION_REQUEST_H
#define PLEXTOR_MEDIA_READ_ACQUISITION_REQUEST_H

#include "../platform/mcs251.h"

u8 RequestReadAcquisitionAndWait(s32 base_target, s32 signed_adjustment);
void PrepareAndDispatchReadOrSeekAcquisitionAtLba(s32 requested_lba);

#endif
