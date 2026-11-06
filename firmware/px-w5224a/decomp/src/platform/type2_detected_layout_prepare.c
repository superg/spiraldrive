#include "type2_detected_layout_prepare.h"
#include "arithmetic.h"
#include "controller_record_boundaries.h"
#include "controller_record_region_scan.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "track_state.h"
#include "type2_mip_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/type2_detected_layout_prepare_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u8 FailType2DetectedLayoutPreparation(void)
{
    RESET_TYPE2_DETECTED_STATE();
    return 0u;
}

/* RAM:fcd0b8 */
u8 PrepareType2ControllerRecordPipelineFromDetectedLayout(void)
{
    s32 probe_window_start;
    s32 probe_window_end;
    u8 status = 0u;
    u8 candidate_index;
    u8 accepted = 0u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD93) & 0xd0u) != 0x50u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD9C) & 1u) == 0u)
        return 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 1u);
    probe_window_start = CONVERT_TYPE2_DETECTED_MSF(FW_NEAR_UNKNOWN_DD94);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);
    probe_window_end = (s32)((u32)probe_window_start + 30ul);

    for(candidate_index = 0u; candidate_index < 4u; ++candidate_index)
    {
        s32 candidate = (s32)((u32)probe_window_start - (u32)probe_window_end + MULTIPLY_TYPE2_DETECTED(5ul, candidate_index));

        status = PROBE_TYPE2_DETECTED_RECORD(candidate);
        if(status == 5u)
            break;
        if(status == 0xfeu)
        {
            RESET_TYPE2_DETECTED_STATE();
            accepted = 1u;
            break;
        }
        if(status == 0x0fu)
        {
            CONFIGURE_TYPE2_DETECTED_RING(1u);
            INITIALIZE_TYPE2_DETECTED_PRIMARY();
            INITIALIZE_TYPE2_DETECTED_TABLES();
            INITIALIZE_TYPE2_DETECTED_MIP();
            INITIALIZE_TYPE2_DETECTED_RUNTIME();
            accepted = 1u;
            break;
        }
    }

    if(accepted == 0u)
    {
        if(status == 0xffu)
        {
            if(PROBE_TYPE2_DETECTED_TDI(1u) == 0u)
                return FailType2DetectedLayoutPreparation();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C9F) != 0u && SCAN_TYPE2_DETECTED_REGIONS(1u) == 0u)
                return FailType2DetectedLayoutPreparation();
            RESET_TYPE2_DETECTED_STATE();
        }
        else if(status == 5u)
        {
            RESET_TYPE2_DETECTED_STATE();
        }
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 3u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 1u)
        return 1u;

    if(PROBE_TYPE2_DETECTED_TDI(0u) == 0u)
        return FailType2DetectedLayoutPreparation();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C9F) == 0u)
    {
        if(LOCATE_TYPE2_DETECTED_WINDOW() == 0u)
            return FailType2DetectedLayoutPreparation();
        if(SCAN_TYPE2_DETECTED_REGIONS(0u) == 0u)
            return FailType2DetectedLayoutPreparation();
    }
    else
    {
        if(SCAN_TYPE2_DETECTED_REGIONS(0u) == 0u)
            return FailType2DetectedLayoutPreparation();
        if(LOCATE_TYPE2_DETECTED_WINDOW() == 0u)
            return FailType2DetectedLayoutPreparation();
    }
    return 1u;
}
