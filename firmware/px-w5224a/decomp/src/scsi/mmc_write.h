#ifndef PLEXTOR_SCSI_MMC_WRITE_H
#define PLEXTOR_SCSI_MMC_WRITE_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION ValidateAndReconcileWriteCommandStartLba(s32 requested_start_lba);


u8 CheckWriteRawReadPipelineContinuation(void);


u8 PLEXTOR_NEAR_FUNCTION AdvanceRawReadPacketCursorAfterTransferBatch(void);


u8 PLEXTOR_NEAR_FUNCTION PrepareRawReadPlanAtFixedPacketBoundary(void);


void ExecuteVendorF3DriveCycleCommand(void);


u8 PLEXTOR_NEAR_FUNCTION AcquireLeadInRawReadDescriptorsWithRetry(void);


void CompleteCloseTrackSessionAfterOpc(void);

#endif
