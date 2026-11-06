#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveConfiguredSegmentsSelectSectorClass(u8 prefer_working_track_class, u8 fallback_class);
extern fw_addr_t ObserveConfiguredSegmentsResumeRing(void);
extern u8 ObserveConfiguredSegmentsAcquireFrames(s32 frame_count);
extern void ObserveConfiguredSegmentsPopulateRange(fw_addr_t controller_buffer_address, u32 sector_count);
extern u8 ObserveConfiguredSegmentsAcquirePrefix(u8 prefix_mode, u8 capture_source_kind);
extern u8 ObserveConfiguredSegmentsAcquireWithQUpdates(s32 frame_count, u8 reverse_relative_msf);
#define SELECT_SECTOR_CLASS(prefer, fallback) ObserveConfiguredSegmentsSelectSectorClass((prefer), (fallback))
#define RESUME_RING() ObserveConfiguredSegmentsResumeRing()
#define ACQUIRE_FRAMES(count) ObserveConfiguredSegmentsAcquireFrames(count)
#define POPULATE_RANGE(address, count) ObserveConfiguredSegmentsPopulateRange((address), (count))
#define ACQUIRE_PREFIX(mode, source) ObserveConfiguredSegmentsAcquirePrefix((mode), (source))
#define ACQUIRE_WITH_Q_UPDATES(count, reverse) ObserveConfiguredSegmentsAcquireWithQUpdates((count), (reverse))
#else
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#define SELECT_SECTOR_CLASS(prefer, fallback) SelectRawReadSectorClass((prefer), (fallback))
#define RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define ACQUIRE_FRAMES(count) AcquireRawReadFramesAndPopulateTdiPayload(count)
#define POPULATE_RANGE(address, count) PopulateRawSectorHeaderAndTdiPayloadRange((address), (count))
#define ACQUIRE_PREFIX(mode, source) AcquireFixedRawReadFramePrefixWindow((mode), (source))
#define ACQUIRE_WITH_Q_UPDATES(count, reverse) AcquireRawReadFrameSequenceWithQUpdates((count), (reverse))
#endif
#endif
