#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFormat11ScanFinalizeWrite(void);
extern void ObserveFormat11ScanInitializeReadVerify(void);
extern u8 ObserveFormat11ScanProbe(void);
extern s32 ObserveFormat11ScanMapLogical(s32 logical_position);
extern u32 ObserveFormat11ScanResetTransfer(void);
extern void ObserveFormat11ScanPositionTransfer(s32 frame_position);
extern void ObserveFormat11ScanResetQueue(void);
extern void ObserveFormat11ScanInitializeCoroutine2(u32 entry_point);
extern void ObserveFormat11ScanYield(void);
extern void ObserveFormat11ScanReceiveHostPacket(void);
extern u8 ObserveFormat11ScanPoll(void);
extern void ObserveFormat11ScanResetForTarget(s32 target_position, u8 mode);
extern void ObserveFormat11ScanUpdateRecordPage(s32 frame_position, u8 count);
extern u8 ObserveFormat11ScanAppendTargets(u8 requested_count, u8 initial_output_index);
extern u32 ObserveFormat11ScanMapFixedPacket(u32 logical_block_address);
extern void ObserveFormat11ScanServiceHost(void);
extern u16 ObserveFormat11ScanRecordBanks(u8 force_bank_scan, s32 start_frame_position, u32 frame_count);
extern u8 ObserveFormat11ScanReadBit000A(void);
extern void ObserveFormat11ScanWriteBit(u8 bit_address, u8 value);
#define SCAN_HOST_BIT_000C 0x0cu
#define SCAN_HOST_BIT_000D 0x0du
#define SCAN_HOST_BIT_000E 0x0eu
#define SCAN_HOST_BIT_0013 0x13u
#define SCAN_HOST_BIT_0018 0x18u
#define SCAN_HOST_BIT_001B 0x1bu
#define SCAN_HOST_BIT_001C 0x1cu
#define SCAN_HOST_BIT_001D 0x1du
#define SCAN_HOST_BIT_0580 0x80u
#define SCAN_HOST_BIT_0581 0x81u
#define SCAN_FINALIZE_WRITE() ObserveFormat11ScanFinalizeWrite()
#define SCAN_INITIALIZE_READ_VERIFY() ObserveFormat11ScanInitializeReadVerify()
#define SCAN_PROBE() ObserveFormat11ScanProbe()
#define SCAN_MAP_LOGICAL(position) ObserveFormat11ScanMapLogical(position)
#define SCAN_RESET_TRANSFER() ObserveFormat11ScanResetTransfer()
#define SCAN_POSITION_TRANSFER(position) ObserveFormat11ScanPositionTransfer(position)
#define SCAN_RESET_QUEUE() ObserveFormat11ScanResetQueue()
#define SCAN_INITIALIZE_COROUTINE2(entry) ObserveFormat11ScanInitializeCoroutine2(entry)
#define SCAN_YIELD() ObserveFormat11ScanYield()
#define SCAN_RECEIVE() ObserveFormat11ScanReceiveHostPacket()
#define SCAN_POLL() ObserveFormat11ScanPoll()
#define SCAN_RESET_TARGET(position, mode) ObserveFormat11ScanResetForTarget((position), (mode))
#define SCAN_UPDATE_PAGE(position, count) ObserveFormat11ScanUpdateRecordPage((position), (count))
#define SCAN_APPEND_TARGETS(count, index) ObserveFormat11ScanAppendTargets((count), (index))
#define SCAN_MAP_FIXED(address) ObserveFormat11ScanMapFixedPacket(address)
#define SCAN_SERVICE_HOST() ObserveFormat11ScanServiceHost()
#define SCAN_RECORD_BANKS(force, start, count) ObserveFormat11ScanRecordBanks((force), (start), (count))
#define SCAN_READ_BIT000A() ObserveFormat11ScanReadBit000A()
#define SCAN_CLEAR_MODE_BITS()                                                                                                                                                                         \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_001B, 0u);                                                                                                                                           \
        ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_001D, 0u);                                                                                                                                           \
        ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_001C, 0u);                                                                                                                                           \
        ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_0013, 0u);                                                                                                                                           \
        ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_000D, 0u);                                                                                                                                           \
        ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_000E, 0u);                                                                                                                                           \
        ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_0018, 0u);                                                                                                                                           \
    } while(0)
#define SCAN_CLEAR_BIT000C() ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_000C, 0u)
#define SCAN_WRITE_P3_RX(value) ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_0580, (value))
#define SCAN_WRITE_P3_TX(value) ObserveFormat11ScanWriteBit(SCAN_HOST_BIT_0581, (value))
#else
extern u8 FinalizeActiveWritePipelineAndTrackState(void);
extern void InitializeReadVerifyWorkingTrackState(void);
extern u8 ProbeFormatUnitCode11MediaRanges(void);
extern s32 MapLogicalBlockAddressToControllerFramePosition(s32 logical_position);
extern u32 ResetRawReadTransferState(void);
extern void PositionRawReadPipelineAtFramePosition(s32 frame_position);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(u32 entry_point);
extern void YieldCoroutine0(void);
extern void ReceiveAndDispatchHostPacketDuringActiveOperation(void);
extern u8 PollRawReadTransferBatchStatus(void);
extern void ResetRawReadPipelineForOptionalTarget(s32 target_position, u8 mode);
extern void UpdateControllerBufferRecordPageForFramePosition(s32 frame_position, u8 count);
extern u8 AppendFormatUnitCode11ScanTargetPositions(u8 requested_count, u8 initial_output_index);
extern u32 MapFixedPacketLogicalToPhysicalAddress(u32 logical_block_address);
extern void ServiceCurrentHostCommandDuringActiveOperation(void);
extern u16 ScanControllerRecordBanksForFrameRange(u8 force_bank_scan, s32 start_frame_position, u32 frame_count);
extern u8 ReadBit000AForFormatUnitCode11Scan(void);
extern void ClearFormatUnitCode11ScanModeBitsAtomic(void);
extern void ClearBit000CForFormatUnitCode11Scan(void);
sbit Format11ScanP3Rx = 0xb0;
sbit Format11ScanP3Tx = 0xb1;
#define SCAN_FINALIZE_WRITE() ((void)FinalizeActiveWritePipelineAndTrackState())
#define SCAN_INITIALIZE_READ_VERIFY() InitializeReadVerifyWorkingTrackState()
#define SCAN_PROBE() ProbeFormatUnitCode11MediaRanges()
#define SCAN_MAP_LOGICAL(position) MapLogicalBlockAddressToControllerFramePosition(position)
#define SCAN_RESET_TRANSFER() ResetRawReadTransferState()
#define SCAN_POSITION_TRANSFER(position) PositionRawReadPipelineAtFramePosition(position)
#define SCAN_RESET_QUEUE() ResetRawReadQueueAndTransferState()
#define SCAN_INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define SCAN_YIELD() YieldCoroutine0()
#define SCAN_RECEIVE() ReceiveAndDispatchHostPacketDuringActiveOperation()
#define SCAN_POLL() PollRawReadTransferBatchStatus()
#define SCAN_RESET_TARGET(position, mode) ResetRawReadPipelineForOptionalTarget((position), (mode))
#define SCAN_UPDATE_PAGE(position, count) UpdateControllerBufferRecordPageForFramePosition((position), (count))
#define SCAN_APPEND_TARGETS(count, index) AppendFormatUnitCode11ScanTargetPositions((count), (index))
#define SCAN_MAP_FIXED(address) MapFixedPacketLogicalToPhysicalAddress(address)
#define SCAN_SERVICE_HOST() ServiceCurrentHostCommandDuringActiveOperation()
#define SCAN_RECORD_BANKS(force, start, count) ScanControllerRecordBanksForFrameRange((force), (start), (count))
#define SCAN_READ_BIT000A() ReadBit000AForFormatUnitCode11Scan()
#define SCAN_CLEAR_MODE_BITS() ClearFormatUnitCode11ScanModeBitsAtomic()
#define SCAN_CLEAR_BIT000C() ClearBit000CForFormatUnitCode11Scan()
#define SCAN_WRITE_P3_RX(value) (Format11ScanP3Rx = (value))
#define SCAN_WRITE_P3_TX(value) (Format11ScanP3Tx = (value))
#endif
#endif
