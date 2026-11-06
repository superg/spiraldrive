#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveReserveTrackRawReset(void);
extern void ObserveReserveTrackRawFill(fw_addr_t address, u32 count, u8 value);
extern u8 ObserveReserveTrackRawInitializeQueue(void);
extern u8 ObserveReserveTrackRawRunOpc(void);
extern void ObserveReserveTrackRawSetSense(u8 key, u16 code);
extern void ObserveReserveTrackRawConvertBcd(u16 tuple_address);
extern s32 ObserveReserveTrackRawConvertMsf(fw_addr_t tuple_address);
extern void ObserveReserveTrackRawSequenceError(void);
extern u8 ObserveReserveTrackRawPreparePlan(u8 alternate);
extern u8 ObserveReserveTrackRawPoll(u8 count);
extern void ObserveReserveTrackRawQueueSense(void);
extern void ObserveReserveTrackRawFailure(u8 code);
#define RESERVE_RAW_RESET() ObserveReserveTrackRawReset()
#define RESERVE_RAW_FILL(a, c, v) ObserveReserveTrackRawFill((a), (c), (v))
#define RESERVE_RAW_INIT_QUEUE() ObserveReserveTrackRawInitializeQueue()
#define RESERVE_RAW_RUN_OPC() ObserveReserveTrackRawRunOpc()
#define RESERVE_RAW_SET_SENSE(k, c) ObserveReserveTrackRawSetSense((k), (c))
#define RESERVE_RAW_CONVERT_BCD(a) ObserveReserveTrackRawConvertBcd(a)
#define RESERVE_RAW_CONVERT_MSF(a) ObserveReserveTrackRawConvertMsf(a)
#define RESERVE_RAW_SEQUENCE_ERROR() ObserveReserveTrackRawSequenceError()
#define RESERVE_RAW_PREPARE_PLAN(a) ObserveReserveTrackRawPreparePlan(a)
#define RESERVE_RAW_POLL(c) ObserveReserveTrackRawPoll(c)
#define RESERVE_RAW_QUEUE_SENSE() ObserveReserveTrackRawQueueSense()
#define RESERVE_RAW_FAILURE(c) ObserveReserveTrackRawFailure(c)
#else
extern void SetSenseData(u8 sense_key, u16 sense_code);
#define RESERVE_RAW_RESET() ResetRawReadPipelineBuffersAndState()
#define RESERVE_RAW_FILL(a, c, v) FillControllerBufferRange((a), (c), (v))
#define RESERVE_RAW_INIT_QUEUE() InitializeAndBuildRawReadQRecordQueue()
#define RESERVE_RAW_RUN_OPC() RunOpcForCurrentWriteSpeed()
#define RESERVE_RAW_SET_SENSE(k, c) SetSenseData((k), (c))
#define RESERVE_RAW_CONVERT_BCD(a) ConvertThreePackedBcdBytesToBinary(a)
#define RESERVE_RAW_CONVERT_MSF(a) ConvertBinaryMsfToAdjustedFramePosition(a)
#define RESERVE_RAW_SEQUENCE_ERROR() SetRawReadPositionCommandSequenceError()
#define RESERVE_RAW_PREPARE_PLAN(a) PrepareRawReadAcquisitionPlan(a)
#define RESERVE_RAW_POLL(c) PollRawReadPipelineProgress(c)
#define RESERVE_RAW_QUEUE_SENSE() ReportSense030C09FromRawReadQueue()
#define RESERVE_RAW_FAILURE(c) ReportReadPipelineFailure(c)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t tuple_address = FW_NEAR_UNKNOWN_7000;
#else
u8 tuple[3];
fw_addr_t tuple_address = (fw_addr_t)tuple;
#endif
#endif
