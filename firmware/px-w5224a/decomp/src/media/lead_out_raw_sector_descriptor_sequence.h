#ifndef PLEXTOR_LEAD_OUT_RAW_SECTOR_DESCRIPTOR_SEQUENCE_H
#define PLEXTOR_LEAD_OUT_RAW_SECTOR_DESCRIPTOR_SEQUENCE_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AcquireLeadOutRawSectorDescriptorSequence(u8 restart_after_failure);

#endif
