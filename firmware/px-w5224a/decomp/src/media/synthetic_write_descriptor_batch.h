#ifndef PLEXTOR_SYNTHETIC_WRITE_DESCRIPTOR_BATCH_H
#define PLEXTOR_SYNTHETIC_WRITE_DESCRIPTOR_BATCH_H

#include "../platform/mcs251.h"

void EnqueueThirtyTwoSyntheticWriteDescriptorsForStartAddress(s32 start_address);
void EnqueueThirtyTwoWritePreservationDescriptors(void);

#endif
