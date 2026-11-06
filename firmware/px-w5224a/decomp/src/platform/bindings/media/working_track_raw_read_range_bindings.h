#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWorkingTrackRangeReset(void);
extern void ObserveWorkingTrackRangeFill(fw_addr_t address, u32 count, u8 value);
extern u8 ObserveWorkingTrackRangeSectorClass(u8 prefer, u8 fallback);
extern void ObserveWorkingTrackRangeSetMsf(s32 relative, s32 absolute);
extern void ObserveWorkingTrackRangeSequenceError(void);
extern u8 ObserveWorkingTrackRangePreparePlan(u8 alternate);
extern void ObserveWorkingTrackRangeReportFailure(void);
extern u8 ObserveWorkingTrackRangeAcquire(s32 count);
extern u8 ObserveWorkingTrackRangePrefix(u8 omit_final, u8 wait);
#define RANGE_RESET() ObserveWorkingTrackRangeReset()
#define RANGE_FILL(a, c, v) ObserveWorkingTrackRangeFill((a), (c), (v))
#define RANGE_SECTOR_CLASS(p, f) ObserveWorkingTrackRangeSectorClass((p), (f))
#define RANGE_SET_MSF(r, a) ObserveWorkingTrackRangeSetMsf((r), (a))
#define RANGE_SEQUENCE_ERROR() ObserveWorkingTrackRangeSequenceError()
#define RANGE_PREPARE_PLAN(a) ObserveWorkingTrackRangePreparePlan(a)
#define RANGE_REPORT_FAILURE() ObserveWorkingTrackRangeReportFailure()
#define RANGE_ACQUIRE(c) ObserveWorkingTrackRangeAcquire(c)
#define RANGE_PREFIX(o, w) ObserveWorkingTrackRangePrefix((o), (w))
#else
#define RANGE_RESET() ResetRawReadPipelineBuffersAndState()
#define RANGE_FILL(a, c, v) FillControllerBufferRange((a), (c), (v))
#define RANGE_SECTOR_CLASS(p, f) SelectRawReadSectorClass((p), (f))
#define RANGE_SET_MSF(r, a) SetRawReadQPacketMsfTuples((r), (a))
#define RANGE_SEQUENCE_ERROR() SetRawReadPositionCommandSequenceError()
#define RANGE_PREPARE_PLAN(a) PrepareRawReadAcquisitionPlan(a)
#define RANGE_REPORT_FAILURE() ReportSense030C08AtRawReadTarget()
#define RANGE_ACQUIRE(c) AcquireRawReadFramesForWorkingTrack(c)
#define RANGE_PREFIX(o, w) PrepareRawReadFramePrefixAndOptionallyWait((o), (w))
#endif
#endif
