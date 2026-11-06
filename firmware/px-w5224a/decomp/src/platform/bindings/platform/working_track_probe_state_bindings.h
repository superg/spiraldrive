#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR15ForWorkingTrackProbeState(void);
extern s32 ObserveRefineRawQFrameBoundaryForWorkingTrackProbeState(s32 initial_frame, s32 upper_frame);
extern u8 ObserveProbeRawQOffsetsForWorkingTrackProbeState(s32 initial_frame, s32 reference_frame);
#define CAPTURE_WORKING_TRACK_PROBE_R15() ObserveCaptureInheritedR15ForWorkingTrackProbeState()
#define REFINE_WORKING_TRACK_PROBE_BOUNDARY(initial, upper) ObserveRefineRawQFrameBoundaryForWorkingTrackProbeState((initial), (upper))
#define PROBE_WORKING_TRACK_OFFSETS(initial, reference) ObserveProbeRawQOffsetsForWorkingTrackProbeState((initial), (reference))
#else
extern u8 PLEXTOR_NEAR_FUNCTION CaptureInheritedR15ForWorkingTrackProbeState(void);
extern s32 PLEXTOR_NEAR_FUNCTION RefineRawQFrameBoundaryAcrossStepSizes(s32 initial_frame, s32 upper_frame);
extern u8 PLEXTOR_NEAR_FUNCTION ProbeRawQAtThreeDecrementingFrameOffsets(s32 initial_frame, s32 reference_frame);
#define CAPTURE_WORKING_TRACK_PROBE_R15() CaptureInheritedR15ForWorkingTrackProbeState()
#define REFINE_WORKING_TRACK_PROBE_BOUNDARY(initial, upper) RefineRawQFrameBoundaryAcrossStepSizes((initial), (upper))
#define PROBE_WORKING_TRACK_OFFSETS(initial, reference) ProbeRawQAtThreeDecrementingFrameOffsets((initial), (reference))
#endif
#endif
