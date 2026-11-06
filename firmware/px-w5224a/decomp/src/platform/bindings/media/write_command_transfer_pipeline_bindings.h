#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveWriteCommandPipelineOperation(u8 operation, u32 argument0, u32 argument1, u32 argument2);
#define PIPELINE_OP(op, a, b, c) ObserveWriteCommandPipelineOperation((op), (a), (b), (c))
#define CLEAR_BIT30() ((void)PIPELINE_OP(WRITE_PIPELINE_CLEAR_BIT30, 0ul, 0ul, 0ul))
#define CLEAR_CONTROLLER_REGIONS() ((void)PIPELINE_OP(WRITE_PIPELINE_CLEAR_CONTROLLER_REGIONS, 0ul, 0ul, 0ul))
#define RESET_RAW_READ() ((void)PIPELINE_OP(WRITE_PIPELINE_RESET_RAW_READ, 0ul, 0ul, 0ul))
#define INITIALIZE_HOST_RANGE() PIPELINE_OP(WRITE_PIPELINE_INITIALIZE_HOST_RANGE, 0ul, 0ul, 0ul)
#define INITIALIZE_RAW_READ() ((void)PIPELINE_OP(WRITE_PIPELINE_INITIALIZE_RAW_READ, 0ul, 0ul, 0ul))
#define FILL_CONTROLLER_RANGE(address, count, value) ((void)PIPELINE_OP(WRITE_PIPELINE_FILL_CONTROLLER_RANGE, (address), (count), (value)))
#define INITIALIZE_LAYOUT() ((void)PIPELINE_OP(WRITE_PIPELINE_INITIALIZE_LAYOUT, 0ul, 0ul, 0ul))
#define START_TRANSFER(count, address) ((void)PIPELINE_OP(WRITE_PIPELINE_START_TRANSFER, (count), (address), 0ul))
#define PIPELINE_YIELD() ((void)PIPELINE_OP(WRITE_PIPELINE_YIELD, 0ul, 0ul, 0ul))
#define TRANSFER_SECTORS(source, destination, count) ((void)PIPELINE_OP(WRITE_PIPELINE_TRANSFER_SECTORS, (source), (destination), (count)))
#define FAIL_PREPARATION() ((u8)PIPELINE_OP(WRITE_PIPELINE_FAIL_PREPARATION, 0ul, 0ul, 0ul))
#define POLL_BATCH() ((u8)PIPELINE_OP(WRITE_PIPELINE_POLL_BATCH, 0ul, 0ul, 0ul))
#define EXECUTE_RANGE() ((u8)PIPELINE_OP(WRITE_PIPELINE_EXECUTE_RANGE, 0ul, 0ul, 0ul))
#define PREPARE_PREFIX(omit, wait) ((u8)PIPELINE_OP(WRITE_PIPELINE_PREPARE_PREFIX, (omit), (wait), 0ul))
#define REPORT_FAILURE(code) ((void)PIPELINE_OP(WRITE_PIPELINE_REPORT_FAILURE, (code), 0ul, 0ul))
#define BEGIN_PIPELINE() ((u8)PIPELINE_OP(WRITE_PIPELINE_BEGIN, 0ul, 0ul, 0ul))
#define SET_NO_SENSE() ((void)PIPELINE_OP(WRITE_PIPELINE_SET_NO_SENSE, 0ul, 0ul, 0ul))
#define MULTIPLY_U32(left, right) ((u32)(left) * (u32)(right))
#define DIVIDE_S32(left, right) ((left) / (right))
#else
extern void YieldCoroutine0(void);
extern void ClearBit0030Atomic(void);
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#define CLEAR_BIT30() ClearBit0030Atomic()
#define CLEAR_CONTROLLER_REGIONS() ClearRawReadControllerBufferRegions()
#define RESET_RAW_READ() ResetRawReadPipelineBuffersAndState()
#define INITIALIZE_HOST_RANGE() InitializeHostTransferRange()
#define INITIALIZE_RAW_READ() InitializeWriteCommandRawReadPipelineState()
#define FILL_CONTROLLER_RANGE(address, count, value) FillControllerBufferRange((address), (count), (value))
#define INITIALIZE_LAYOUT() InitializeWriteRawReadTransferLayout()
#define START_TRANSFER(count, address) StartWriteControllerBufferTransfer((count), (address))
#define PIPELINE_YIELD() YieldCoroutine0()
#define TRANSFER_SECTORS(source, destination, count) TransferControllerBufferSectorsAndWait((source), (destination), (count))
#define FAIL_PREPARATION() FailIfReadMeasurementPreparationActive()
#define POLL_BATCH() PollWritePipelineTransferBatchStatus()
#define EXECUTE_RANGE() ExecuteWriteControllerBufferRangeSequence()
#define PREPARE_PREFIX(omit, wait) PrepareRawReadFramePrefixAndOptionallyWait((omit), (wait))
#define REPORT_FAILURE(code) ReportReadPipelineFailure(code)
#define BEGIN_PIPELINE() BeginWriteCommandTransferPipeline()
#define SET_NO_SENSE() SetNoSenseData()
#define MULTIPLY_U32(left, right) MultiplyU32Low((left), (right))
#define DIVIDE_S32(left, right) DivideS32QuotientNative((left), (right))
#endif
#endif
