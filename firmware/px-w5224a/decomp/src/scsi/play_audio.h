#ifndef PLEXTOR_PLAY_AUDIO_H
#define PLEXTOR_PLAY_AUDIO_H

#include "../platform/mcs251.h"

void StorePlaybackRangeStartFramePosition(s32 frame_position);

#if defined(PLEXTOR_HOST_TEST)
void ValidateAndStartPlayAudioMsfRangeFromCdbTuplesForTest(fw_addr_t end_tuple_address, fw_addr_t start_tuple_address, u8 playback_mode);
void ValidateAndStartPlayAudioFrameRangeForTest(s32 start_frame_position, s32 frame_count, u8 playback_mode);
#else
void ValidateAndStartPlayAudioMsfRangeFromCdbTuples(fw_addr_t end_tuple_address, fw_addr_t start_tuple_address, u8 playback_mode);
void ValidateAndStartPlayAudioFrameRange(s32 start_frame_position, s32 frame_count, u8 playback_mode);
#endif

#endif
