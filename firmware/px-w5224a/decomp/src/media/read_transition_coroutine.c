#include "read_transition_coroutine.h"
#include "../platform/drive_control_reset.h"
#include "../platform/firmware_memory.h"
#include "../platform/read_acquisition_calibration_sequence.h"
#include "../platform/status_mapping.h"
#include "../platform/timer_debounce.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/read_transition_coroutine_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fa6e33 */
void RunMediaReadTransitionCoroutine1(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) | 3u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        RESTART_MEDIA_READ_TRANSITION(FW_ENTRY_RUN_MEDIA_STATE_COROUTINE1);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 2u);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 8u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 20u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    if(ExecuteReadAcquisitionCalibrationSequence() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        RESET_MEDIA_READ_TRANSITION_DRIVE_CONTROL();
        SelectMediaReadTransitionOperationStatus();
        UPDATE_MEDIA_READ_TRANSITION_P3(1u);
        for(;;)
        {
            UPDATE_MEDIA_READ_TRANSITION_P3(0u);
            YIELD_MEDIA_READ_TRANSITION();
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3B, FirmwareReadNear8(FW_NEAR_UNKNOWN_1235));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1236));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1237));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3E, FirmwareReadNear8(FW_NEAR_UNKNOWN_1238));
    }
    RESTART_MEDIA_READ_TRANSITION(FW_ENTRY_RUN_MEDIA_STATE_COROUTINE1);
}
