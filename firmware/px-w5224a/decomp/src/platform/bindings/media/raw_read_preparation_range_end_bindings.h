#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveFrameDistanceToNextType1TrackRecordBoundary(u16 record_index);
#define FRAME_DISTANCE_TO_NEXT_TYPE1(record_index) ObserveFrameDistanceToNextType1TrackRecordBoundary(record_index)
#else
#include "platform/status_mapping.h"
#define FRAME_DISTANCE_TO_NEXT_TYPE1(record_index) ComputeFrameDistanceToNextType1TrackRecordBoundary(record_index)
#endif
#endif
