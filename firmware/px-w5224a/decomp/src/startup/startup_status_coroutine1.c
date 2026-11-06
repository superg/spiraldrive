#include "startup_status_coroutine1.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/startup_status_coroutine1_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:f853cf */
void RunStartupStatusCoroutine1(void)
{
    u8 status;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1309, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfdu));
    STATUS_COROUTINE_RESET_DRIVE();
    STATUS_COROUTINE_WRITE_D6();
    STATUS_COROUTINE_WRITE_P1(0u, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12F7) != 2u)
        STATUS_COROUTINE_PERSIST();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12F7) != 0u)
        goto pattern_false_then_yield;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 0x39u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE7) == 4u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0x0du);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF1, 0u);
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 2u);
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
            }
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 1u) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAF1) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 2u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAF1) == 3u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 5u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAF1) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
    }

    status = FirmwareReadNear8(FW_NEAR_UNKNOWN_1009);
    if(status == 4u)
    {
        STATUS_COROUTINE_WRITE_P3(1u, 1u);
        STATUS_COROUTINE_APPLY_CONFIGURATION();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0x40u) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) | 1u));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 4u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0xfbu));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 1u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0xfeu));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 4u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1308, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0xfbu));
        }
        for(;;)
            STATUS_COROUTINE_YIELD();
    }

    if(status == 6u || status == 7u)
        goto pattern_false_tight;
    if(status == 3u || status == 5u || status == 8u || status == 0x0bu || status == 0x0cu || status == 0x0du || status == 0x0eu)
        goto pattern_false_then_yield;

    STATUS_COROUTINE_PATTERN(1u);
    for(;;)
    {
        STATUS_COROUTINE_YIELD();
        STATUS_COROUTINE_PATTERN(0u);
    }

pattern_false_tight:
    STATUS_COROUTINE_PATTERN(1u);
    for(;;)
        STATUS_COROUTINE_PATTERN(0u);

pattern_false_then_yield:
    STATUS_COROUTINE_PATTERN(1u);
    for(;;)
    {
        STATUS_COROUTINE_PATTERN(0u);
        STATUS_COROUTINE_YIELD();
    }
}
