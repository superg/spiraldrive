#include "synthetic_track_table.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "../platform/track_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/synthetic_track_table_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void CopyBigEndianU32(fw_addr_t destination, fw_addr_t source)
{
    FirmwareWrite8(destination, FirmwareRead8(source));
    FirmwareWrite8(destination + 1u, FirmwareRead8(source + 1u));
    FirmwareWrite8(destination + 2u, FirmwareRead8(source + 2u));
    FirmwareWrite8(destination + 3u, FirmwareRead8(source + 3u));
}

/* RAM:fb4fa0 */
void BuildSyntheticTrackTableAndAcquireLeadInLeadOutSequences(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C4A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C45, 0u);
    RESET_SYNTHETIC_TRACK_TABLE();
    CLEAR_SYNTHETIC_WORKING_TRACK();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, 2u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD9A, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD93, FirmwareRead8(FW_HUGE_UNKNOWN_2BB045));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD94, FirmwareRead8(FW_HUGE_UNKNOWN_2BB04C));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD95, FirmwareRead8(FW_HUGE_UNKNOWN_2BB04D));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD96, FirmwareRead8(FW_HUGE_UNKNOWN_2BB04E));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD97, FirmwareRead8(FW_HUGE_UNKNOWN_2BB048));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD98, FirmwareRead8(FW_HUGE_UNKNOWN_2BB049));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD99, FirmwareRead8(FW_HUGE_UNKNOWN_2BB04A));

    CopyBigEndianU32(FW_NEAR_UNKNOWN_DD69, FW_NEAR_CURRENT_FRAME_POSITION);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD6D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD6E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD6F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD70, 0u);
    UPDATE_SYNTHETIC_TRACK_TAIL();
    CLEAR_SYNTHETIC_CONTROLLER_WORDS();
    UPDATE_SYNTHETIC_TRACK_CONTROL();
    BUILD_SYNTHETIC_PRIMARY();
    BUILD_SYNTHETIC_SECONDARY();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1455, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1456, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1457, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1458, 0u);
    if(ACQUIRE_SYNTHETIC_DESCRIPTORS() == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 5u);
    CLEAR_SYNTHETIC_CONTROLLER_WORDS();
}
