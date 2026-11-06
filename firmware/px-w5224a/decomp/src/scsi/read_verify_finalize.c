#include "read_verify_finalize.h"
#include "../platform/firmware_memory.h"
#include "../platform/mcs251.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_verify_finalize_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fcbc19 */
void ReadVerifyFinalizeFunction(void)
{
    u8 mode = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);

    if(mode == 0u)
        return;

    if(mode == 1u || mode == 2u)
    {
        if(FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH) != 0u)
            return;

        if(FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 0x08u));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0u)
                FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 60u);
            else
                FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 20u);
        }
        else if((s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) < 0)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 1u));
            if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0x08u) != 0u)
                FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 60u);
            else
                FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
        }
        return;
    }

    if(mode == 3u)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) >= 0)
            return;
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 1u));
        if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0x08u) != 0u)
            FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 60u);
        else
            FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
        return;
    }

    FAIL_STOP_INVALID_MODE();
}
