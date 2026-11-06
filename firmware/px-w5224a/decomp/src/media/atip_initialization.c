#include "atip_initialization.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/atip_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void SelectAtipOperatingCode(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
}

static u8 MapAtipRetryFailure(void)
{
    u8 result = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
    if(result == 0x16u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 6u);
        return 0x16u;
    }
    if(result == 0x15u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 5u);
        return 0x15u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 3u);
    return 0x14u;
}

/* RAM:fb8085 */
u8 InitializeControllerAndAcquireAtipState(void)
{
    u8 retries = 0u;
    u8 retry_requested = 0u;

    SelectAtipOperatingCode();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
    ATIP_RESET();

    for(;;)
    {
        u8 acquisition;

        if(retries > 4u)
            return 4u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xfbu));
        ATIP_RESET_DRIVE();
        if(ATIP_IS_INDEXED(0xf7u) == 1u)
            ATIP_PROCESS_INDEXED();

        if(ATIP_INITIALIZE_CONTROLLER() == 0u)
        {
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) >> 2) & 1u) != 1u)
            {
                ATIP_RESET_PORTS();
                ATIP_RESET_HARDWARE();
                ATIP_RESET_OPERATING();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 0u);
                return 2u;
            }
            retries++;
            ATIP_RESET_DRIVE();
            if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) == 7u)
                retries = (u8)(retries + 2u);
            if(retries >= 4u)
                return MapAtipRetryFailure();
            continue;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
            return 0u;

        SelectAtipOperatingCode();
        ATIP_WRITE_CONTROLLER(FW_CONTROLLER_UNKNOWN_018082, 0xb0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 15u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
        {
            ATIP_RESET();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3B, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3C, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3D, 0xdcu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3E, 0xd8u);
            acquisition = ATIP_ACQUIRE_POSITION();
            if(acquisition == 1u)
            {
                ATIP_APPLY_OPERATING();
                FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0u);
                ATIP_ARM_PACKET();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 2u));
                while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
                {
                    u8 packet = ATIP_PARSE_PACKET();

                    if(packet != 0xfeu)
                    {
                        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
                        {
                            retries++;
                            retry_requested = 1u;
                            break;
                        }
                        ATIP_ARM_PACKET();
                    }
                    ATIP_YIELD();
                    if(ATIP_VALIDATE(1u) != 0u)
                    {
                        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 1u) != 0u)
                            ATIP_ACQUIRE_SUPPLEMENTAL();
                        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0x80u);
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfdu));
                        return 0u;
                    }
                }
            }
            else
            {
                retries++;
                retry_requested = 1u;
            }
        }

        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0x80u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfdu));
        ATIP_RESET_DRIVE();
        if(retry_requested == 0u)
            return 4u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 1u));
        retry_requested = 0u;
    }
}
