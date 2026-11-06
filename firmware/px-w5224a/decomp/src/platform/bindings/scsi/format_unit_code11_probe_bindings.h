#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFormat11ProbeResetRawRead(void);
extern s32 ObserveFormat11ProbeMap(s32 logical_position);
extern void ObserveFormat11ProbeInitializeCoroutine2(u32 entry);
extern void ObserveFormat11ProbeYield(void);
extern void ObserveFormat11ProbeReceiveHostPacket(void);
extern u8 ObserveFormat11ProbePoll(void);
extern void ObserveFormat11ProbeResetForTarget(s32 target, u8 mode);
extern void ObserveFormat11ProbeUpdateRecordPage(s32 position, u8 count);
#define PROBE_RESET_RAW_READ() ObserveFormat11ProbeResetRawRead()
#define PROBE_MAP(position) ObserveFormat11ProbeMap(position)
#define PROBE_INITIALIZE(entry) ObserveFormat11ProbeInitializeCoroutine2(entry)
#define PROBE_YIELD() ObserveFormat11ProbeYield()
#define PROBE_RECEIVE() ObserveFormat11ProbeReceiveHostPacket()
#define PROBE_POLL() ObserveFormat11ProbePoll()
#define PROBE_RESET_TARGET(target, mode) ObserveFormat11ProbeResetForTarget((target), (mode))
#define PROBE_UPDATE_PAGE(position, count) ObserveFormat11ProbeUpdateRecordPage((position), (count))
#define PROBE_WRITE_P3_RX(value) FirmwareWriteBit0580(value)
#define PROBE_WRITE_P3_TX(value) FirmwareWriteBit0581(value)
#else
#include "platform/controller_record_page_update.h"
#include "platform/coroutine_initialization.h"
#include "platform/leaf_workers.h"
#include "scsi/raw_sector_host_transfer_poll.h"
extern void YieldCoroutine0(void);
extern void ReceiveAndDispatchHostPacketDuringActiveOperation(void);
extern void ResetRawReadQueueAndTransferState(void);
extern s32 MapLogicalBlockAddressToControllerFramePosition(s32 logical_position);
sbit Format11ProbeP3Rx = 0xb0;
sbit Format11ProbeP3Tx = 0xb1;
#define PROBE_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define PROBE_MAP(position) MapLogicalBlockAddressToControllerFramePosition(position)
#define PROBE_INITIALIZE(entry) InitializeCoroutine2(entry)
#define PROBE_YIELD() YieldCoroutine0()
#define PROBE_RECEIVE() ReceiveAndDispatchHostPacketDuringActiveOperation()
#define PROBE_POLL() PollRawReadTransferBatchStatus()
#define PROBE_RESET_TARGET(target, mode) ResetRawReadPipelineForOptionalTarget((target), (mode))
#define PROBE_UPDATE_PAGE(position, count) UpdateControllerBufferRecordPageForFramePosition((position), (count))
#define PROBE_WRITE_P3_RX(value) (Format11ProbeP3Rx = (value))
#define PROBE_WRITE_P3_TX(value) (Format11ProbeP3Tx = (value))
#endif
#endif
