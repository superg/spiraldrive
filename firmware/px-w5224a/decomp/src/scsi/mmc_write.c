#include "mmc_write.h"
#include "../media/active_write_pipeline.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "../platform/track_state.h"

/* Native boundaries retained in assembly.  The worker has no C-visible
 * arguments or return value; this C caller only preserves their order. */
extern void ServicePendingHostCommandWhenOperationFlagsPermit(void);
extern void YieldCoroutine0(void);
extern void HandleUnsupportedScsiCommand(void);
/* These adapters pin the independently assigned input registers of the two
 * native workers called by RAM:fe796a. */
extern u8 PrepareRawReadFramePrefixOmittingFinalFrameNoWait(void);
extern u8 AcquireFixedRawReadFramePrefixWindowMode2Source4(void);
extern u8 PrepareRawReadAcquisitionPlanPrimary(void);
extern u8 PLEXTOR_NEAR_FUNCTION DiscardRawReadDescriptorsFromQueueTail(u16 descriptor_count);
extern void ExecuteVendorF3IndexedDriveCycleSequenceFromCdb(void);
extern u8 AcquireLeadInRawSectorDescriptorSequenceInitialAtip(void);
extern u8 AcquireLeadInRawSectorDescriptorSequenceInitialNonAtip(void);
extern u8 AcquireLeadInRawSectorDescriptorSequenceRetry(void);
extern u8 EnsureOpcForSelectedWriteSpeedForCloseTrack(void);
extern u8 AcquireLeadInAndLeadOutRawSectorDescriptorSequencesForCloseTrack(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/mmc_write_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/mmc_write_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/scsi/mmc_write_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra function address. */
#define PLEXTOR_BINDING_SECTION 4
#include "../platform/bindings/scsi/mmc_write_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fbc576 */
u8 PLEXTOR_NEAR_FUNCTION ValidateAndReconcileWriteCommandStartLba(s32 requested_start_lba)
{
    s32 target_frame_position = (s32)((u32)requested_start_lba + 150ul);
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 1u)
    {
        if(requested_start_lba != 0l && target_frame_position != (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D))
        {
            WRITE_START_UNSUPPORTED_COMMAND();
            return 0u;
        }
    }
    else if(write_type == 3u)
    {
        if(requested_start_lba != 0l && target_frame_position != (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD65))
        {
            WRITE_START_UNSUPPORTED_COMMAND();
            return 0u;
        }
    }
    else if(write_type == 0u && requested_start_lba != 0l)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
        {
            s32 write_cursor = (s32)FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_DD4D);

            if(target_frame_position != write_cursor && target_frame_position != (s32)((u32)write_cursor + 7ul))
            {
                WRITE_START_UNSUPPORTED_COMMAND();
                return 0u;
            }
        }
        else if(target_frame_position != WRITE_START_PACKET_PROGRESS(1u))
        {
            if(WRITE_START_FINALIZE_PIPELINE() == 0u)
                return 0u;
            if(WRITE_START_VALIDATE_TRACK_TABLE(requested_start_lba) == 0u)
            {
                WRITE_START_UNSUPPORTED_COMMAND();
                return 0u;
            }
            while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
                WRITE_START_YIELD();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x10u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
        }
    }
    return 1u;
}

/* RAM:fe79c8 */
/* The native routine returns ACC 1 after clearing the two continuation flags,
 * or ACC 0 after a pending host operation completes.  A queued count of 30
 * or more takes the clear-and-continue path. */
u8 CheckWriteRawReadPipelineContinuation(void)
{
    u8 write_option_flags;
    u16 queued_descriptor_count;

    if(FirmwareReadBit0030() != 0u)
        goto clear_continuation_flags;

    write_option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46);
    if(((write_option_flags >> 1) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
        goto clear_continuation_flags;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 1u)
        goto clear_continuation_flags;

    queued_descriptor_count = (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_1292);
    if(queued_descriptor_count >= 30u)
        goto clear_continuation_flags;

    do
    {
        ServicePendingHostCommandWhenOperationFlagsPermit();
        YieldCoroutine0();
    } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 1u);
    return 0u;

clear_continuation_flags:
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1284, 0u);
    return 1u;
}

/* RAM:fe796a */
/* Called after one completed raw-read transfer batch.  The native worker
 * adds the just-selected u16 batch count to the 32-bit packet cursor.  Only
 * exact equality with the fixed-packet size starts the next prefix sequence;
 * an overshoot deliberately takes the ordinary completion path. */
u8 PLEXTOR_NEAR_FUNCTION AdvanceRawReadPacketCursorAfterTransferBatch(void)
{
    u32 packet_cursor = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_14B8) + (u32)FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1463);

    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_14B8, packet_cursor);
    if(packet_cursor == FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_0C4B))
    {
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_14B8, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B2, 1u);
        if(PrepareRawReadFramePrefixOmittingFinalFrameNoWait() == 0u)
            return 0u;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B3, 1u);
        if(AcquireFixedRawReadFramePrefixWindowMode2Source4() == 0u)
            return 0u;

        packet_cursor = FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1598) - 1u;
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1598, packet_cursor);
        FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_8000, packet_cursor);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    return 1u;
}

/* RAM:fe791c */
/* The one-descriptor discard deliberately has no failure branch: the native
 * caller ignores its Boolean result before checking the completed-count
 * snapshot and preparing the primary raw-read acquisition plan. */
u8 PLEXTOR_NEAR_FUNCTION PrepareRawReadPlanAtFixedPacketBoundary(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B2) == 1u)
    {
        if(FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_1292) < 0x20u)
        {
            while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
            {
                ServicePendingHostCommandWhenOperationFlagsPermit();
                YieldCoroutine0();
            }
        }
        (void)DiscardRawReadDescriptorsFromQueueTail(1u);
        if(FirmwareReadBigEndianU32Near(FW_NEAR_UNKNOWN_1459) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 1u);
        if(PrepareRawReadAcquisitionPlanPrimary() == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B2, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 0u);
    }
    return 1u;
}

/* RAM:fbf161 */
void ExecuteVendorF3DriveCycleCommand(void)
{
    u16 configuration_word = FirmwareReadBigEndianU16Near(FW_NEAR_UNKNOWN_FAAC);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAC, (u8)(configuration_word >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAD, (u8)(configuration_word | 0x0008u));
    InitializeCoroutine1(FW_ENTRY_RESET_CONTROLLER_AND_IDLE_COROUTINE1);
    YieldCoroutine0();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) & 1u) != 0u)
    {
        ExecuteVendorF3IndexedDriveCycleSequenceFromCdb();
        return;
    }
    ResetControllerChannelsAndApplyDriveCode5(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1208, 0x0fu);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1208) >= 0)
        YieldCoroutine0();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0u);
}

/* RAM:fe7a5e */
u8 PLEXTOR_NEAR_FUNCTION AcquireLeadInRawReadDescriptorsWithRetry(void)
{
    u8 attempt = 0u;
    u8 success;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
        success = AcquireLeadInRawSectorDescriptorSequenceInitialAtip();
    else
        success = AcquireLeadInRawSectorDescriptorSequenceInitialNonAtip();
    while(success == 0u && attempt <= 8u && FirmwareReadNear8(FW_NEAR_UNKNOWN_88BA) != 0u)
    {
        attempt = (u8)(attempt + 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        success = AcquireLeadInRawSectorDescriptorSequenceRetry();
    }
    if(success != 0u)
        SetNoSenseData();
    else
        ReportReadPipelineFailure(4u);
    return success;
}

/* RAM:fb4e93 */
/* The two native Boolean workers retain their own ERET conventions through
 * adapters; every other call uses an already reconstructed C-facing leaf. */
void CompleteCloseTrackSessionAfterOpc(void)
{
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    YieldCoroutine0();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && EnsureOpcForSelectedWriteSpeedForCloseTrack() == 0u)
        return;

    UpdateSelectedTrackTableTailFrameBounds();
    ClearControllerBufferWordsC119AndC517();
    UpdateTrackTableControlBit6Intersection();
    BuildTrackTablePrimaryStagedRawQRecordSequence();
    BuildTrackTableSecondaryStagedRawQRecordSequence();
    FirmwareWriteBigEndianU32Near(FW_NEAR_UNKNOWN_1455, 0u);
    if(AcquireLeadInAndLeadOutRawSectorDescriptorSequencesForCloseTrack() == 0u)
    {
        ClearControllerBufferWordsC119AndC517();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 8u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_128C) == 0u)
            return;
    }
    ClearControllerBufferWordsC119AndC517();
    InitializeWorkingTrackFromTrackTableTail();
}
