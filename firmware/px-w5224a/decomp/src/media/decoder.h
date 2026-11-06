#ifndef PLEXTOR_MEDIA_DECODER_H
#define PLEXTOR_MEDIA_DECODER_H

#include "../platform/mcs251.h"


void UpdateControllerPacketThirtyFramePhaseMatchFlag(u8 packet_phase_class);
u8 ParseControllerThreeBytePacket(void);
u8 AcquireReadPositionPacketWithChannelSetup(void);
void CaptureControllerD8D9WindowIntoMeasurementSeries(void);
void InitializeReadAcquisitionRangeFromLastTocTrack(void);
void AdvanceMediaState3IndexedTransitionState(void);
void ConfigureReadAcquisitionControllerForOperatingCode(void);

#endif
