#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveWritePipelineFinalizeIsrcAvailable(void);
extern void ObserveWritePipelineFinalizeLoadIsrc(void);
extern void ObserveWritePipelineFinalizeConvertFrame(s32 frame_position, u8 output[3]);
extern void ObserveWritePipelineFinalizeCopyTuple(u8 destination[3], const u8 source[3]);
extern u8 ObserveWritePipelineFinalizeCompareTuples(const u8 left[3], const u8 right[3]);
extern u8 ObserveWritePipelineFinalizeAcquireLeadOut(u8 restart_after_failure);
extern void ObserveWritePipelineFinalizeReportFailure(u8 failure_code);
extern void ObserveWritePipelineFinalizeReportSense030C09(void);
extern void ObserveWritePipelineFinalizeSetNoSense(void);
#define FINALIZE_ISRC_AVAILABLE() ObserveWritePipelineFinalizeIsrcAvailable()
#define FINALIZE_LOAD_ISRC() ObserveWritePipelineFinalizeLoadIsrc()
#define FINALIZE_CONVERT_FRAME(frame, output) ObserveWritePipelineFinalizeConvertFrame((frame), (output))
#define FINALIZE_COPY_TUPLE(destination, source) ObserveWritePipelineFinalizeCopyTuple((destination), (source))
#define FINALIZE_COMPARE_TUPLES(left, right) ObserveWritePipelineFinalizeCompareTuples((left), (right))
#define FINALIZE_ACQUIRE_LEAD_OUT(restart) ObserveWritePipelineFinalizeAcquireLeadOut(restart)
#define FINALIZE_REPORT_FAILURE(code) ObserveWritePipelineFinalizeReportFailure(code)
#define FINALIZE_REPORT_SENSE_030C09() ObserveWritePipelineFinalizeReportSense030C09()
#define FINALIZE_SET_NO_SENSE() ObserveWritePipelineFinalizeSetNoSense()
#else
#define FINALIZE_ISRC_AVAILABLE() IsIsrcAvailableForCurrentTrack()
#define FINALIZE_LOAD_ISRC() LoadIsrcCharactersForCurrentTrack()
#define FINALIZE_CONVERT_FRAME(frame, output) ConvertFramePositionToBinaryMsfTuple((frame), (fw_addr_t)(output))
#define FINALIZE_COPY_TUPLE(destination, source) CopyThreeBytes16((u16)(destination), (u16)(source))
#define FINALIZE_COMPARE_TUPLES(left, right) CompareBinaryMsfTuples((u16)(left), (u16)(right))
#define FINALIZE_ACQUIRE_LEAD_OUT(restart) AcquireLeadOutRawSectorDescriptorSequence(restart)
#define FINALIZE_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
#define FINALIZE_REPORT_SENSE_030C09() ReportSense030C09FromRawReadQueue()
#define FINALIZE_SET_NO_SENSE() SetNoSenseData()
#endif
#endif
