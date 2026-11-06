#ifndef PLEXTOR_SCSI_READ_VERIFY_RANGE_H
#define PLEXTOR_SCSI_READ_VERIFY_RANGE_H

#include "../platform/mcs251.h"

#if defined(PLEXTOR_HOST_TEST)
void ExecuteReadVerifyRangeForTest(void);
#else
void ExecuteReadVerifyRange(void);
#endif
u8 PollReadVerifyRawDescriptorStatus(void);
void RunFullMediaReadVerifyRecoveryCoroutine0(void);

#endif
