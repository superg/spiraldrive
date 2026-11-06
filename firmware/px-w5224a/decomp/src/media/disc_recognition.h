#ifndef PLEXTOR_MEDIA_DISC_RECOGNITION_H
#define PLEXTOR_MEDIA_DISC_RECOGNITION_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AccumulateCurrentQPacketIntoInitialToc(void);
u8 PLEXTOR_NEAR_FUNCTION AcquireInitialDiscTocState(void);
u8 PLEXTOR_NEAR_FUNCTION BuildFullTocForAvailableSessions(void);
u8 PLEXTOR_NEAR_FUNCTION AcquireNextSessionLeadInToc(void);
u8 PLEXTOR_NEAR_FUNCTION AccumulateCurrentQPacketIntoSessionToc(void);
void PLEXTOR_NEAR_FUNCTION AppendFullTocSessionFromDecodedTocState(void);
void PLEXTOR_NEAR_FUNCTION ProbeOuterDiscAndDetermineReadSpeedLimit(void);
u8 RunDiscRecognitionAndTocInitialization(void);

#endif
