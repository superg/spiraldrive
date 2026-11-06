#ifndef PLEXTOR_RAW_READ_FRAME_PREFIX_WAIT_H
#define PLEXTOR_RAW_READ_FRAME_PREFIX_WAIT_H

#include "../platform/mcs251.h"

u8 PrepareRawReadFramePrefixAndOptionallyWait(u8 omit_final_prefix_frame, u8 wait_for_completion);

#endif
