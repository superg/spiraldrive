#ifndef PLEXTOR_SCSI_VENDOR_F3_STATE_H
#define PLEXTOR_SCSI_VENDOR_F3_STATE_H

#include "../platform/mcs251.h"

void SetVendorF3StateWordsAndFlag(u16 value);
void PLEXTOR_NEAR_FUNCTION ExecuteVendorF3IndexedDriveCycleSequence(u8 maximum_cycle_index);

#endif
