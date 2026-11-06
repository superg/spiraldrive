#include "repair_track.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/repair_track_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fafe24 */
void HandleRepairTrackCommand(void)
{
    u8 inherited_r7 = CAPTURE_INHERITED_R7();
    u8 track_number;
    u8 track_type;

    if(VALIDATE_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D2C)) == 0u)
    {
        REJECT_COMMAND();
        return;
    }
    if(VALIDATE_WRITE_PARAMETERS() == 0u || VALIDATE_MEDIA() == 0u)
        return;

    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F);
    if(track_number < FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
    {
        REJECT_COMMAND();
        return;
    }
    if(track_number != 0xffu && track_number > FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F))
    {
        REJECT_COMMAND();
        return;
    }
    if(track_number == 0xffu)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) == 100u)
        {
            REJECT_COMMAND();
            return;
        }
        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    }

    CLEAR_WORKING_TRACK();
    if(LOAD_TRACK(track_number, inherited_r7) == 0u)
        return;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) != 0x50u)
    {
        REJECT_COMMAND();
        return;
    }

    track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    if(track_type != 5u && track_type != 6u)
    {
        if(track_type != 7u)
        {
            SET_NO_SENSE();
            return;
        }
        SET_SENSE(2u, 0x3004u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x08u));
    FINALIZE_REPAIR();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
}
