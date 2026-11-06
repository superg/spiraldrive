#ifndef PLEXTOR_MEDIA_PREMIUM_RAW_BATCH_H
#define PLEXTOR_MEDIA_PREMIUM_RAW_BATCH_H

#include "../platform/mcs251.h"

u32 AcquirePlextorPremiumRawSectorBatch(s32 start_frame_position, u16 sector_count);
u8 PLEXTOR_NEAR_FUNCTION ValidatePlextorPremiumDiscSignatureSector(void);
u8 PLEXTOR_NEAR_FUNCTION AcquirePlextorPremiumFirmwarePayloadFromDisc(void);

#endif
