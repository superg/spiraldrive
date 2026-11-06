#include "coroutine_initialization.h"
#include "../boot/led_boot_diagnostics.h"
#include "../boot/startup_progress_diagnostics.h"
#include "../startup/drive_firmware_initialization.h"
#include "configuration_range_update.h"
#include "firmware_memory.h"
#include "read_acquisition_request_dispatch.h"
#include "status_mapping.h"

extern void ResumeCoroutine1(void);
extern void ResumeCoroutine2(void);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/coroutine_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fdcea9 */
void FirmwareMainLoop(void)
{
    LED_BOOT_DIAGNOSTICS_ARM();
    STARTUP_DIAGNOSTIC_ARM();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FB) != 1u)
    {
        FirmwareMainInitializeDrive();
        FirmwareMainEnableInterrupts();
    }
    STARTUP_DIAGNOSTIC_MARK(2u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) != 1u || FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEB, 1u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x80u) == 0u)
        FirmwareMainClearP3Bits();

    FirmwareMainConfigureCoroutines();
    STARTUP_DIAGNOSTIC_MARK(3u);
    if(FirmwareMainIsIndexedState(5u) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1157, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018040));
        if(FirmwareMainUpdateConfiguration(FW_NEAR_UNKNOWN_1157, 0x47u, 1u) == 0u)
            FirmwareMainInitializeCoroutine1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
    }

    for(;;)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x40u) != 0u)
            FirmwareMainServiceTransition();
        else if(FirmwareReadNear8(FW_NEAR_FORMAT_OPERATION_ACTIVE) == 1u)
            FirmwareMainApplyPendingRestart();

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15F6) == 1u && FirmwareMainIsIndexedState(8u) == 0u)
            FirmwareMainDispatchReadRequest();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x08u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0x20u) == 0u)
            FirmwareMainServiceP1Transition();

        FirmwareMainResume0();
        FirmwareMainResume1();
        if(FirmwareMainReadBit0029() != 1u)
            FirmwareMainResume2();
        if(FirmwareMainReadBit0046() != 1u)
            FirmwareMainResume3();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x20u) != 0u)
            FirmwareMainInitializeCoroutine0(FW_ENTRY_SCSI_COMMAND_PROCESSOR_COROUTINE0);
        FirmwareMainLoopEnd();
    }
}

/* RAM:fdcfad */
/* ERET or no return. */
void ConfigureInitialCoroutines(void)
{
    u8 state;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1214, 5u);
    while((s8)INITIAL_COROUTINE_COUNTDOWN() > 0)
    {
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) != 0u)
    {
        CONFIGURE_INITIAL_COROUTINE0(FW_ENTRY_SCSI_COMMAND_PROCESSOR_COROUTINE0);
        CONFIGURE_INITIAL_COROUTINE1(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
        for(;;)
        {
            CONFIGURE_INITIAL_RESUME0();
            CONFIGURE_INITIAL_RESUME1();
        }
    }

    CONFIGURE_INITIAL_COROUTINE0(FW_ENTRY_SCSI_COMMAND_PROCESSOR_COROUTINE0);
    CONFIGURE_INITIAL_COROUTINE3(FW_ENTRY_IDLE_COROUTINE3);
    CONFIGURE_INITIAL_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
#if defined(PLEXTOR_LED_HOST_ROUTE_DIAGNOSTICS)
    /* Diagnostic-only tagged initial entry; later replacements remain real. */
    CONFIGURE_INITIAL_COROUTINE1((fw_addr_t)StartupRouteInitialEntry);
#else
    CONFIGURE_INITIAL_COROUTINE1(FW_ENTRY_RUN_STARTUP_SEQUENCE_COROUTINE1);
#endif

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 1u)
        return;
    state = FirmwareReadNear8(FW_NEAR_UNKNOWN_130A);
    switch(state)
    {
    case 0u:
    case 8u:
    case 16u:
        return;
    case 5u:
        CONFIGURE_INITIAL_COROUTINE1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
        return;
    case 15u:
        CONFIGURE_INITIAL_COROUTINE0(FW_ENTRY_RUN_INDEXED_STATE0F_OPERATION_COROUTINE0);
        return;
    case 31u:
        CONFIGURE_INITIAL_COROUTINE1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
        CONFIGURE_INITIAL_COROUTINE0(FW_ENTRY_RUN_DRIVE_OPERATION_RESULT_COROUTINE0);
        return;
    case 1u:
    case 2u:
    case 3u:
    case 4u:
    case 6u:
    case 7u:
    case 17u:
    case 20u:
    case 21u:
    case 22u:
    case 23u:
    case 0xf2u:
    case 0xfdu:
    case 0xffu:
        if(state == 4u && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 1u)
            return;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
            CONFIGURE_INITIAL_COROUTINE0(FW_ENTRY_RUN_DRIVE_OPERATION_RESULT_COROUTINE0);
        return;
    default:
        CONFIGURE_INITIAL_COROUTINE0(FW_ENTRY_YIELD_COROUTINE0_FOREVER);
        return;
    }
}

/* RAM:fdd0e3 */
void ServiceDriveInitializationStateTransition(void)
{
    u8 flags;
    u8 operation_mode;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xbfu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfbu));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C35) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) != 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u
        && (FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 10u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 12u))
    {
        flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF);
        if((flags & 0x40u) == 0u)
            return;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(flags & 0xbfu));
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1309) == 1u)
    {
        flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF);
        if((flags & 0x40u) == 0u)
            return;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(flags & 0xbfu));
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x12u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0x80u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0u);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 9u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 10u)
        return;

    operation_mode = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);
    if(operation_mode == 1u || operation_mode == 3u)
    {
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u)
        {
            FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 1u);
            FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_ACTIVE, 1u);
            FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_RESULT, 2u);
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 1u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_ACTIVE, 1u);
            FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_RESULT, 3u);
            return;
        }
    }
    else if(operation_mode == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_ACTIVE, 1u);
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 2u) != 0u)
        {
            FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 1u);
            FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_RESULT, 0u);
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x80u));
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u)
        {
            FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 1u);
            FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_RESULT, 2u);
            return;
        }
        FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_RESULT, 1u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x80u));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0u);
    InitializeCoroutine1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    ResumeCoroutine1();
    ResumeCoroutine2();
}

/* RAM:fee4ad */
void InitializeCoroutine0(u32 entry_point)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0300, (u8)(entry_point >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0301, (u8)(entry_point >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0302, (u8)(entry_point >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0303, (u8)entry_point);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_115E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_115F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1160, 0x03u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1161, 0x07u);
}

/* RAM:fee4c1 */
void InitializeCoroutine1(u32 entry_point)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0500, (u8)(entry_point >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0501, (u8)(entry_point >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0502, (u8)(entry_point >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0503, (u8)entry_point);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1162, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1163, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1164, 0x05u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1165, 0x07u);
}

/* RAM:fee4d5 */
void InitializeCoroutine2(u32 entry_point)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0700, (u8)(entry_point >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0701, (u8)(entry_point >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0702, (u8)(entry_point >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0703, (u8)entry_point);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1166, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1167, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1168, 0x07u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1169, 0x07u);
}

/* RAM:fee4e9 */
void InitializeCoroutine3(u32 entry_point)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0900, (u8)(entry_point >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0901, (u8)(entry_point >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0902, (u8)(entry_point >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0903, (u8)entry_point);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_116A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_116B, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_116C, 0x09u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_116D, 0x07u);
}
