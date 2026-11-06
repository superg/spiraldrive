#ifndef PLEXTOR_MEDIA_RAW_PAYLOAD_SIGNATURE_H
#define PLEXTOR_MEDIA_RAW_PAYLOAD_SIGNATURE_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION ClassifyLatestQueuedRawSectorDescriptor(void);
u8 ClassifyRawSectorPayloadSignatureAtPipelineTarget(void);

#endif
