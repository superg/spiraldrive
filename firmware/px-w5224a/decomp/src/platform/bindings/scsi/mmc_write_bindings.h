#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveWriteStartPacketProgress(u8 compress_packet_gaps);
extern u8 ObserveWriteStartFinalizeActivePipeline(void);
extern u8 ObserveWriteStartValidateTrackTable(s32 frame_position);
extern void ObserveWriteStartUnsupportedCommand(void);
extern void ObserveWriteStartYield(void);
#define WRITE_START_PACKET_PROGRESS(compress_packet_gaps) ObserveWriteStartPacketProgress(compress_packet_gaps)
#define WRITE_START_FINALIZE_PIPELINE() ObserveWriteStartFinalizeActivePipeline()
#define WRITE_START_VALIDATE_TRACK_TABLE(frame_position) ObserveWriteStartValidateTrackTable(frame_position)
#define WRITE_START_UNSUPPORTED_COMMAND() ObserveWriteStartUnsupportedCommand()
#define WRITE_START_YIELD() ObserveWriteStartYield()
#else
#define WRITE_START_PACKET_PROGRESS(compress_packet_gaps) ComputeWorkingTrackPacketProgressFramePosition(compress_packet_gaps)
#define WRITE_START_FINALIZE_PIPELINE() FinalizeActiveWritePipelineAndTrackState()
#define WRITE_START_VALIDATE_TRACK_TABLE(frame_position) ValidateWriteFramePositionAgainstTrackTable(frame_position)
#define WRITE_START_UNSUPPORTED_COMMAND() HandleUnsupportedScsiCommand()
#define WRITE_START_YIELD() YieldCoroutine0()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
static u16 FirmwareReadBigEndianU16Near(u16 address)
{
    return FirmwareReadNearU16(address);
}
#else
#define FirmwareReadBigEndianU16Near(address) (*((volatile u16 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
static u32 FirmwareReadBigEndianU32Near(u16 address)
{
    return FirmwareReadNearU32(address);
}
#else
#define FirmwareReadBigEndianU32Near(address) (*((volatile u32 huge *)(fw_addr_t)(u16)(address)))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 4
#if defined(PLEXTOR_HOST_TEST)
static void FirmwareWriteBigEndianU32Near(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}
#else
#define FirmwareWriteBigEndianU32Near(address, value) (*((volatile u32 huge *)(fw_addr_t)(u16)(address)) = (u32)(value))
#endif
#endif
