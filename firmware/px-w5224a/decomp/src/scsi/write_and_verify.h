#ifndef PLEXTOR_SCSI_WRITE_AND_VERIFY_H
#define PLEXTOR_SCSI_WRITE_AND_VERIFY_H

#if defined(PLEXTOR_HOST_TEST)
void HandleWriteAndVerify10CommandForTest(void);
#else
void HandleWriteAndVerify10Command(void);
#endif

#endif
