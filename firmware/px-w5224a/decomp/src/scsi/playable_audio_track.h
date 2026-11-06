#ifndef PLEXTOR_PLAYABLE_AUDIO_TRACK_H
#define PLEXTOR_PLAYABLE_AUDIO_TRACK_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION IsFramePositionOnPlayableAudioTrack(s32 frame_position);
u8 PLEXTOR_NEAR_FUNCTION ValidatePlayAudioFrameRange(s32 start_frame_position, s32 end_frame_position);

#endif
