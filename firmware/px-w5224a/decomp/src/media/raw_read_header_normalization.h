#ifndef PLEXTOR_RAW_READ_HEADER_NORMALIZATION_H
#define PLEXTOR_RAW_READ_HEADER_NORMALIZATION_H

#include "../platform/mcs251.h"

void CopyAndNormalizeRawReadHeaderToLinkedBuffer(u32 descriptor_handle, u8 header_type);

#endif
