#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRawReadPrefixTuple(s32 frame_position, u8 *tuple);
extern u8 ObserveRawReadPrefixSelectSectorClass(void);
extern u8 ObserveRawReadPrefixFindClass(u16 record_index, u8 scan_forward);
extern u8 ObserveRawReadPrefixDecodeType(u16 record_index);
extern void ObserveRawReadPrefixSequenceError(void);
extern u8 ObserveRawReadPrefixAcquireSequence(s32 frame_count, u8 reverse);
extern u8 ObserveRawReadPrefixAcquireWindow(u8 prefix_mode, u8 capture_source_kind);
extern u8 ObserveRawReadPrefixPoll(u8 poll_count);
extern void ObserveRawReadPrefixFailure(u8 failure_code);
extern void ObserveRawReadPrefixQueueSense(void);
#define PREFIX_BUILD_TUPLE(frame, tuple) ObserveRawReadPrefixTuple((frame), (tuple))
#define PREFIX_SELECT_CLASS() ObserveRawReadPrefixSelectSectorClass()
#define PREFIX_FIND_CLASS(index, forward) ObserveRawReadPrefixFindClass((index), (forward))
#define PREFIX_DECODE_TYPE(index) ObserveRawReadPrefixDecodeType(index)
#define PREFIX_SEQUENCE_ERROR() ObserveRawReadPrefixSequenceError()
#define PREFIX_ACQUIRE_SEQUENCE(count, reverse) ObserveRawReadPrefixAcquireSequence((count), (reverse))
#define PREFIX_ACQUIRE_WINDOW(mode, source) ObserveRawReadPrefixAcquireWindow((mode), (source))
#define PREFIX_POLL(count) ObserveRawReadPrefixPoll(count)
#define PREFIX_FAILURE(code) ObserveRawReadPrefixFailure(code)
#define PREFIX_QUEUE_SENSE() ObserveRawReadPrefixQueueSense()
#else
#define PREFIX_BUILD_TUPLE(frame, tuple)                                                                                                                                                               \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ConvertFramePositionToBinaryMsfTuple((frame), (fw_addr_t)(tuple));                                                                                                                             \
        ConvertThreeBinaryBytesToPackedBcd((u16)(tuple));                                                                                                                                              \
    } while(0)
#define PREFIX_SELECT_CLASS() SelectCurrentRawReadSectorClass()
#define PREFIX_FIND_CLASS(index, forward) FindAdjacentRawReadQRecordClass((index), (forward))
#define PREFIX_DECODE_TYPE(index) DecodeRawReadQRecordTypeCode(index)
#define PREFIX_SEQUENCE_ERROR() SetRawReadPositionCommandSequenceError()
#define PREFIX_ACQUIRE_SEQUENCE(count, reverse) AcquireRawReadFrameSequenceWithQUpdates((count), (reverse))
#define PREFIX_ACQUIRE_WINDOW(mode, source) AcquireFixedRawReadFramePrefixWindow((mode), (source))
#define PREFIX_POLL(count) PollRawReadPipelineProgress(count)
#define PREFIX_FAILURE(code) ReportReadPipelineFailure(code)
#define PREFIX_QUEUE_SENSE() ReportSense030C09FromRawReadQueue()
#endif
#endif
