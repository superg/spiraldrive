#include "raw_read_q_control.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/raw_read_q_control_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fea372 */
void PLEXTOR_NEAR_FUNCTION UpdateRawReadQPacketControlAndIdentification(u8 skip_identification_packets, u8 decrement_relative_msf_on_restore)
{
    u8 isrc_available;
    u8 media_catalog_available;
    u8 index;

    UPDATE_Q_CONTROL();
    if(skip_identification_packets == 1u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1405) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1403) == 0u)
        for(index = 0u; index < 10u; index++)
            FirmwareWriteNear8((u16)(0x1491u + index), FirmwareReadNear8((u16)(0x1272u + index)));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1284) == 1u)
        return;

    isrc_available = IS_ISRC_AVAILABLE();
    media_catalog_available = IS_MEDIA_CATALOG_AVAILABLE();

    if(isrc_available == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1284) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1405) == 1u)
        {
            RESTORE_AFTER_ISRC(decrement_relative_msf_on_restore);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1405, 0u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1405, BUILD_ISRC_PACKET());
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14AB) == 1u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AB, 0u);
        }
    }

    if(media_catalog_available == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1284) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1403) == 1u)
        {
            RESTORE_AFTER_MEDIA_CATALOG(decrement_relative_msf_on_restore);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1403, 0u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1403, BUILD_MEDIA_CATALOG_PACKET());
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14AB) == 1u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AB, 0u);
        }
    }
}

/* Reconstruction helper; no direct Ghidra function address. */
static u8 PLEXTOR_NEAR_FUNCTION RestoreCurrentQPacketAfterIdentification(u8 decrement_relative_msf)
{
    u8 index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14AB) == 1u)
    {
        for(index = 0u; index < 10u; index++)
        {
            u8 value = FirmwareReadNear8((u16)(0x88aau + index));

            FirmwareWriteNear8((u16)(0x1272u + index), value);
            FirmwareWriteNear8((u16)(0x1491u + index), value);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14AB, 0u);
    }
    else
    {
        for(index = 0u; index < 10u; index++)
            FirmwareWriteNear8((u16)(0x1272u + index), FirmwareReadNear8((u16)(0x1491u + index)));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, FirmwareReadNear8(FW_NEAR_UNKNOWN_14AA));
        ADVANCE_Q_MSF_TUPLES(decrement_relative_msf);
    }
    return 1u;
}

/* RAM:fea188 */
u8 PLEXTOR_NEAR_FUNCTION RestoreCurrentQPacketAfterIsrc(u8 decrement_relative_msf)
{
    return RestoreCurrentQPacketAfterIdentification(decrement_relative_msf);
}

/* RAM:fea27d */
u8 PLEXTOR_NEAR_FUNCTION RestoreCurrentQPacketAfterMediaCatalog(u8 decrement_relative_msf)
{
    return RestoreCurrentQPacketAfterIdentification(decrement_relative_msf);
}
