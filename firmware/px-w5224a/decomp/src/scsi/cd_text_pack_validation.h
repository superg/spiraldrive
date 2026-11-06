#ifndef PLEXTOR_SCSI_CD_TEXT_PACK_VALIDATION_H
#define PLEXTOR_SCSI_CD_TEXT_PACK_VALIDATION_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION ValidateCdTextPackAndAccumulatePayloadLengths(u32 pack_index);

#endif
