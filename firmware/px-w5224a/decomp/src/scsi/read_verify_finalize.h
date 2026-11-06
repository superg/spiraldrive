#ifndef PLEXTOR_SCSI_READ_VERIFY_FINALIZE_H
#define PLEXTOR_SCSI_READ_VERIFY_FINALIZE_H

#if defined(PLEXTOR_HOST_TEST)
void FinalizeReadVerifyCommandStateForTest(void);
#else
void FinalizeReadVerifyCommandState(void);
#endif

#endif
