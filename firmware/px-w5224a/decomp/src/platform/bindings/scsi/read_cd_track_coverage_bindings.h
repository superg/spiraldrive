#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 PLEXTOR_NEAR_FUNCTION MatchReadCdFramePositionToTrackRangeFromC(s32 target_frame_position, u32 track_bounds_and_bias);
#else
#include "scsi/read_cd_track_range_match.h"
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u32 packed_arguments = (u32)first_track << 24;

packed_arguments |= (u32)last_track << 16;
packed_arguments |= frame_bias;
return MatchReadCdFramePositionToTrackRangeFromC(frame_position, packed_arguments);
#else
return MatchReadCdFramePositionToTrackRange(frame_position, first_track, last_track, frame_bias);
#endif
#endif
