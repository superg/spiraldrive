#ifndef PLEXTOR_CONTROLLER_RECORD_PAGE_RECONCILIATION_H
#define PLEXTOR_CONTROLLER_RECORD_PAGE_RECONCILIATION_H

#include "mcs251.h"

void ReconcileControllerBufferRecordsBackward(u8 group_index, u8 page_index);
void ReconcileControllerBufferRecordsForward(u8 group_index, u8 page_index);
void RebuildControllerBufferRecordPageHeader(u8 group_index, u8 page_index);
void UpdateControllerBufferRecordPageForFramePosition(s32 frame_position, u8 preserved_record_group_count);

#endif
