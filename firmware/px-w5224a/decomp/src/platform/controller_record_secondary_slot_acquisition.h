#ifndef PLEXTOR_CONTROLLER_RECORD_SECONDARY_SLOT_ACQUISITION_H
#define PLEXTOR_CONTROLLER_RECORD_SECONDARY_SLOT_ACQUISITION_H

#include "mcs251.h"

u8 AcquireSecondaryType2ControllerRecordSlot(u8 acquisition_mode, u8 record_slot_index, s32 start_frame_position);

#endif
