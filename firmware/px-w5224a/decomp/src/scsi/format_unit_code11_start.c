#include "format_unit_code11_start.h"
#include "../platform/firmware_memory.h"
#include "../platform/format_unit_type2_pipeline.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/format_unit_code11_start_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void ResetFormat11StartCoroutine(void)
{
    START11_RESET_RAW_READ();
    START11_INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    START11_YIELD();
}

/* RAM:fa32d0 */
void StartFormatUnitCode11Operation(u32 format_variant)
{
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfeu));

    if(format_variant == 0ul)
    {
        ResetFormat11StartCoroutine();
        if(START11_ENSURE_OPC() == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
            return;
        }

        START11_CONFIGURE_RING(1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xbfu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x7fu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 0x02u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 1u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xf7u));

        if(START11_PREPARE_TYPE2() == 0u)
        {
            START11_SET_SENSE(3u, 0x0c07u);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 8u));
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfdu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xbfu));
            return;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xbfu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 0u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 8u));
        FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 2u);
        return;
    }

    ResetFormat11StartCoroutine();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    if(START11_ENSURE_OPC() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) == 0u)
        START11_COMPLETE_NO_SENSE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    ResetFormat11StartCoroutine();
    START11_CONFIGURE_RING(1u);
    FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1586, 1u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) | 0x40u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 8u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));

    if(START11_INITIALIZE_TYPE2(0u) == 0u)
        return;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 0u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 0x10u));
    START11_DISPATCH_COMPLETION();
}
