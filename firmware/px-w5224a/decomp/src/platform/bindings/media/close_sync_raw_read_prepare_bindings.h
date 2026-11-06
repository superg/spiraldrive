#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCloseSyncPrepareReset(void);
extern u8 ObserveCloseSyncPrepareSectorClass(void);
extern void ObserveCloseSyncPrepareBuildQ(s32 relative, fw_addr_t destination);
extern void ObserveCloseSyncPrepareSetMsf(s32 relative, s32 absolute);
extern u8 ObserveCloseSyncPrepareAcquire(s32 count);
extern u8 ObserveCloseSyncPreparePrefix(u8 omit, u8 wait);
extern u8 ObserveCloseSyncPrepareSequence(u8 first_record);
extern u8 ObserveCloseSyncPrepareQueue(void);
extern u8 ObserveCloseSyncPrepareOpc(void);
extern u8 ObserveCloseSyncPreparePlan(u8 alternate);
extern void ObserveCloseSyncPrepareSense030c08(void);
extern void ObserveCloseSyncPrepareSense(u8 key, u16 asc_ascq);
extern void ObserveCloseSyncPrepareTuple(s32 frame, u8 tuple[3]);
extern void ObserveCloseSyncPreparePublishTuple(u16 destination, u8 tuple[3]);
#define RESET_PIPELINE() ObserveCloseSyncPrepareReset()
#define SELECT_SECTOR_CLASS() ObserveCloseSyncPrepareSectorClass()
#define BUILD_Q(relative, destination) ObserveCloseSyncPrepareBuildQ((relative), (destination))
#define SET_MSF(relative, absolute) ObserveCloseSyncPrepareSetMsf((relative), (absolute))
#define ACQUIRE_FRAMES(count) ObserveCloseSyncPrepareAcquire(count)
#define PREPARE_PREFIX(omit, wait) ObserveCloseSyncPreparePrefix((omit), (wait))
#define EXECUTE_SEQUENCE(first) ObserveCloseSyncPrepareSequence(first)
#define BUILD_QUEUE() ObserveCloseSyncPrepareQueue()
#define RUN_OPC() ObserveCloseSyncPrepareOpc()
#define PREPARE_PLAN(alternate) ObserveCloseSyncPreparePlan(alternate)
#define REPORT_TARGET_SENSE() ObserveCloseSyncPrepareSense030c08()
#define REPORT_SENSE(key, asc) ObserveCloseSyncPrepareSense((key), (asc))
#define BUILD_TUPLE(frame, tuple) ObserveCloseSyncPrepareTuple((frame), (tuple))
#define PUBLISH_TUPLE(destination, tuple) ObserveCloseSyncPreparePublishTuple((destination), (tuple))
#else
#define RESET_PIPELINE() ResetRawReadPipelineBuffersAndState()
#define SELECT_SECTOR_CLASS() SelectRawReadSectorClass(0u, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48))
#define BUILD_Q(relative, destination) BuildRawReadQPacketForFramePosition((relative), (destination))
#define SET_MSF(relative, absolute) SetRawReadQPacketMsfTuples((relative), (absolute))
#define ACQUIRE_FRAMES(count) AcquireRawReadFramesForWorkingTrack(count)
#define PREPARE_PREFIX(omit, wait) PrepareRawReadFramePrefixAndOptionallyWait((omit), (wait))
#define EXECUTE_SEQUENCE(first) ExecuteRawReadQRecordSequence(first)
#define BUILD_QUEUE() InitializeAndBuildRawReadQRecordQueue()
#define RUN_OPC() RunOpcForCurrentWriteSpeed()
#define PREPARE_PLAN(alternate) PrepareRawReadAcquisitionPlan(alternate)
#define REPORT_TARGET_SENSE() ReportSense030C08AtRawReadTarget()
#define REPORT_SENSE(key, asc) SetSenseData((key), (asc))
#define BUILD_TUPLE(frame, tuple)                                                                                                                                                                      \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        ConvertFramePositionToBinaryMsfTuple((frame), (fw_addr_t)(tuple));                                                                                                                             \
    } while(0)
#define PUBLISH_TUPLE(destination, tuple)                                                                                                                                                              \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        CopyThreeBytes16((destination), (u16)(tuple));                                                                                                                                                 \
        ConvertThreeBinaryBytesToPackedBcd(destination);                                                                                                                                               \
    } while(0)
#endif
#endif
