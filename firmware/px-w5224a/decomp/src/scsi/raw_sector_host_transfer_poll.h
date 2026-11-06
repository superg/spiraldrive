#ifndef PLEXTOR_SCSI_RAW_SECTOR_HOST_TRANSFER_POLL_H
#define PLEXTOR_SCSI_RAW_SECTOR_HOST_TRANSFER_POLL_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION PollRawSectorReadHostTransferStatus(void);
u8 PollRawReadTransferBatchStatus(void);
s8 PLEXTOR_NEAR_FUNCTION ServiceReadCommandRawSectorTransferBatch(void);
#if defined(PLEXTOR_HOST_TEST)
void ExecuteReadCommandRawSectorTransferForTest(void);
s8 ExecuteReadCdRawSectorTransferForFormatForTest(u8 raw_read_format_index);
void PositionRawReadPipelineAtFramePositionForTest(s32 target_position);
#else
void ExecuteReadCommandRawSectorTransfer(void);
s8 ExecuteReadCdRawSectorTransferForFormat(u8 raw_read_format_index);
void PositionRawReadPipelineAtFramePosition(s32 target_position);
#endif

#endif
