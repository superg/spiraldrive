#include "close_track_sync_operation.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "close_sync_raw_read_prepare.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/close_track_sync_operation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void ClearCloseSyncOperationFlags(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xbfu));
}

static void ClearCloseSyncTransferCount(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1455, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1456, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1457, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1458, 0u);
}

static void CopyCloseSyncFramePosition(fw_addr_t destination, fw_addr_t source)
{
    FirmwareWrite8(destination, FirmwareRead8(source));
    FirmwareWrite8(destination + 1u, FirmwareRead8(source + 1u));
    FirmwareWrite8(destination + 2u, FirmwareRead8(source + 2u));
    FirmwareWrite8(destination + 3u, FirmwareRead8(source + 3u));
}

/* RAM:fb4daa */
void ExecuteCloseTrackOrSynchronizeCacheOperation(void)
{
    RESET_CLOSE_SYNC_RAW_READ();
    INITIALIZE_CLOSE_SYNC_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    YIELD_CLOSE_SYNC_COROUTINE0();
    if(ENSURE_CLOSE_SYNC_OPC() == 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 6u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x40u));

    ClearCloseSyncTransferCount();
    if(PREPARE_CLOSE_SYNC_RAW_READ() == 0u)
    {
        UPDATE_CLOSE_SYNC_WRITE_FAILURE();
        ClearCloseSyncOperationFlags();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 8u));
        return;
    }

    ClearCloseSyncOperationFlags();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
        CopyCloseSyncFramePosition(FW_NEAR_UNKNOWN_DD41, FW_NEAR_UNKNOWN_1488);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u)
    {
        COMMIT_CLOSE_SYNC_STATE();
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
    {
        FINALIZE_CLOSE_SYNC_TRACK(0x5bu);
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 5u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 6u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 6u)
        CopyCloseSyncFramePosition(FW_NEAR_UNKNOWN_DD41, FW_NEAR_UNKNOWN_1488);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 1u);
    COMMIT_CLOSE_SYNC_TAIL();
}
