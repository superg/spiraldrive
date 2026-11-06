#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveFiveFramePreparePlan(u8 alternate_plan);
extern fw_addr_t ObserveFiveFrameResumeRing(void);
extern u8 ObserveFiveFrameWaitForCapacity(void);
extern u8 ObserveFiveFrameCapture(fw_addr_t address, u8 preserve_main, u8 clear_subchannel);
extern void ObserveFiveFramePopulateSubheader(fw_addr_t address, u8 source_kind);
extern void ObserveFiveFrameUpdateQ(u8 skip_identification, u8 decrement_relative);
extern void ObserveFiveFrameEnqueue(void);
extern void ObserveFiveFrameAdvanceMsf(u8 reverse_relative);
extern fw_addr_t ObserveFiveFrameAdvanceRing(fw_addr_t cursor);
extern u8 ObserveFiveFramePreparePrefix(u8 omit_final_prefix, u8 wait_for_completion);
extern void ObserveFiveFrameReportFailure(u8 failure_code);
#define FIVE_FRAME_PREPARE_PLAN(alternate) ObserveFiveFramePreparePlan(alternate)
#define FIVE_FRAME_RESUME_RING() ObserveFiveFrameResumeRing()
#define FIVE_FRAME_WAIT_CAPACITY() ObserveFiveFrameWaitForCapacity()
#define FIVE_FRAME_CAPTURE(address, preserve, clear) ObserveFiveFrameCapture((address), (preserve), (clear))
#define FIVE_FRAME_POPULATE_SUBHEADER(address, kind) ObserveFiveFramePopulateSubheader((address), (kind))
#define FIVE_FRAME_UPDATE_Q(skip, decrement) ObserveFiveFrameUpdateQ((skip), (decrement))
#define FIVE_FRAME_ENQUEUE() ObserveFiveFrameEnqueue()
#define FIVE_FRAME_ADVANCE_MSF(reverse) ObserveFiveFrameAdvanceMsf(reverse)
#define FIVE_FRAME_ADVANCE_RING(cursor) ObserveFiveFrameAdvanceRing(cursor)
#define FIVE_FRAME_PREPARE_PREFIX(omit, wait) ObserveFiveFramePreparePrefix((omit), (wait))
#define FIVE_FRAME_REPORT_FAILURE(code) ObserveFiveFrameReportFailure(code)
#else
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void PopulateConfiguredMode2SubheaderWhenRequired(fw_addr_t raw_sector_address, u8 raw_sector_source_kind);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern void ReportReadPipelineFailure(u8 failure_code);
#define FIVE_FRAME_PREPARE_PLAN(alternate) PrepareRawReadAcquisitionPlan(alternate)
#define FIVE_FRAME_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define FIVE_FRAME_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define FIVE_FRAME_CAPTURE(address, preserve, clear) CompleteRawSectorCaptureAndClearBuffers((address), (preserve), (clear))
#define FIVE_FRAME_POPULATE_SUBHEADER(address, kind) PopulateConfiguredMode2SubheaderWhenRequired((address), (kind))
#define FIVE_FRAME_UPDATE_Q(skip, decrement) UpdateRawReadQPacketControlAndIdentification((skip), (decrement))
#define FIVE_FRAME_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define FIVE_FRAME_ADVANCE_MSF(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define FIVE_FRAME_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define FIVE_FRAME_PREPARE_PREFIX(omit, wait) PrepareRawReadFramePrefixAndOptionallyWait((omit), (wait))
#define FIVE_FRAME_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
#endif
#endif
