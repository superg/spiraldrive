#include "request_sense.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/request_sense_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fde7b0 */
void HandleRequestSenseCommand(void)
{
    u8 allocation_length;

    if(VALIDATE_REQUEST_SENSE(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DE5)) == 0u)
    {
        HANDLE_UNSUPPORTED_REQUEST_SENSE();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B1A) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
            SET_REQUEST_SENSE_FROM_STATUS(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x02u) == 0u)
            SET_REQUEST_SENSE_DATA(2u, 0x0401u);

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x01u) != 0u)
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x20u) != 0u)
                SET_REQUEST_SENSE_DATA(2u, 0x0404u);
            else
                SET_REQUEST_SENSE_DATA(2u, 0x0408u);
        else if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x02u) != 0u)
            SET_REQUEST_SENSE_DATA(0u, 0x0404u);
    }

    allocation_length = MinU8(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E), 18u);
    if(TRANSFER_REQUEST_SENSE(FW_NEAR_UNKNOWN_0B18, (u32)allocation_length) != 4u)
        CLEAR_REQUEST_SENSE_DATA();
}
