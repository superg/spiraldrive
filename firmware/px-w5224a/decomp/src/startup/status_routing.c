#include "status_routing.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/status_routing_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadStatusRoutingWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u8 SelectPendingStartupStatusCode(u8 pending_status)
{
    if(pending_status == 4u)
        return 0x25u;
    if(pending_status == 8u)
        return 0x26u;
    if(pending_status == 7u)
        return 0x49u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F9B5) >= 0xc0u)
            return 0x23u;
        if((ReadStatusRoutingWord(FW_NEAR_UNKNOWN_FAAC) & 0x0004u) != 0u)
            return 0x24u;
        return 0x22u;
    }
    if((ReadStatusRoutingWord(FW_NEAR_UNKNOWN_FAAC) & 0x0004u) != 0u)
        return 0x24u;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x10u) != 0u)
        return 0x23u;
    return 0x22u;
}

/* RAM:fb0672 */
void PLEXTOR_NEAR_FUNCTION RouteStartupStatusAndScheduleCoroutine0(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x13u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu))
    {
        RouteStatusYieldCoroutine1();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB2) != 0u)
        {
            u8 pending_status = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB2);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 6u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, SelectPendingStartupStatusCode(pending_status));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 0u);
            RouteStatusSignalPattern();
        }
    }

    if((ReadStatusRoutingWord(FW_NEAR_UNKNOWN_FAAC) & 0x0100u) == 0u)
        RouteStatusConfigurationMaintenance();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F976) != 1u)
    {
        if(RouteStatusIndexedStateActive(5u) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_F972) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) == 0u
            && (FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x08u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130A, 0x1fu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 0u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F973, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F972, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F974, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F975, 0u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x13u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
        RouteStatusInitializeCoroutine0(FW_ENTRY_RUN_DRIVE_OPERATION_RESULT_COROUTINE0);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x0fu)
        RouteStatusInitializeCoroutine0(FW_ENTRY_RUN_INDEXED_STATE0F_OPERATION_COROUTINE0);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 6u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
        RouteStatusInitializeCoroutine0(FW_ENTRY_RUN_DRIVE_OPERATION_RESULT_COROUTINE0);
    }
}
