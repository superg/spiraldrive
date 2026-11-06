#ifndef PLEXTOR_LEAD_IN_RAW_SECTOR_DESCRIPTOR_SEQUENCE_H
#define PLEXTOR_LEAD_IN_RAW_SECTOR_DESCRIPTOR_SEQUENCE_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AcquireLeadInRawSectorDescriptorSequence(u8 use_atip_lead_in_start, u8 resume_at_failure_position);

#endif
