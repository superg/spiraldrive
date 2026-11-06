#ifndef PLEXTOR_RAW_READ_PATTERN_VALIDATION_H
#define PLEXTOR_RAW_READ_PATTERN_VALIDATION_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION ValidateRawReadDescriptorPayloadPatternBatch(u16 descriptor_count);

#endif
