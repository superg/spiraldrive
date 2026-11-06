#include "command_processor_coroutine.h"
#include "../ata/task_file_command_dispatch.h"
#include "../boot/led_boot_diagnostics.h"
#include "../boot/startup_progress_diagnostics.h"
#include "../media/active_write_pipeline.h"
#include "../platform/controller_startup_mode.h"
#include "../platform/coroutine_initialization.h"
#include "../platform/encoding.h"
#include "../platform/firmware_entry.h"
#include "../platform/firmware_memory.h"
#include "../platform/format_unit_type2_pipeline.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "command_admission.h"
#include "command_dispatch.h"
#include "controller_command_preparation.h"
#include "format_unit_code11.h"
#include "format_unit_code11_scan.h"
#include "inquiry_configuration_refresh.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/command_processor_coroutine_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void InitializeScsiCommandProcessor(void)
{
    u8 status;
    u8 sample;
    u8 count;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x20u) != 0u)
    {
        STARTUP_HOST_PACKET_CHECK(1u);
        PROCESSOR_PREPARE_CONTROLLER();
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018059, 0x97u);
        return;
    }

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 4u);
    for(count = 0u; count < 5u; ++count)
    {
        while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043) & 1u) == 0u)
        {
        }
    }
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0u);
    PROCESSOR_STARTUP_MODE();
    PROCESSOR_REFRESH_INQUIRY();
    FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);
    STARTUP_HOST_PACKET_CHECK(2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C80, 0u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xdcu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xf2u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x37u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1582, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1596, 0u);
    PROCESSOR_BUILD_CRC();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018059, 0x97u);
    PROCESSOR_ENABLE_TIMER0();

    status = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FD) & 0xf9u);
    if(status != 0x98u)
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0x80u);
    sample = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FD) & 0xf9u);
    if(sample == 0x99u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FD, 0u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 0u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
    }
    sample = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FD) & 0xf9u);
    if(sample == 0x98u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FD, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 1u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 1u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
    }
}

static void PrepareIdleScsiCommandProcessorYield(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 3u) == 3u)
        PROCESSOR_PREMIUM_UPDATE();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u)
    {
        if(FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u && FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH) == 0u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 8u) != 0u
            && (s8)FirmwareReadNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN) < 0)
            PROCESSOR_PRIMARY_EXPIRED();

        if(FirmwareReadNear8(FW_NEAR_FORMAT_OPERATION_ACTIVE) != 0u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u)
        {
            if(FirmwareReadNear8(FW_NEAR_FORMAT_OPERATION_RESULT) != 2u)
            {
                if(FirmwareReadNear8(FW_NEAR_FORMAT_OPERATION_RESULT) == 0u)
                    FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_RESULT, 1u);
                PROCESSOR_RESET_WRITE_PIPELINE();
            }
        }
        else if(FirmwareReadNear8(FW_NEAR_FORMAT_OPERATION_ACTIVE) == 1u && (FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 1u) != 0u)
        {
            PROCESSOR_PREPARE_WRITE_PIPELINE();
        }

        if(FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) != 0u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x80u) != 0u && (s8)FirmwareReadNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN) < 0)
            PROCESSOR_MEDIA_SCAN();
    }
}

static void ContinueIdleScsiCommandProcessorAfterYield(void)
{
    if(FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1596) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) != 0u)
        {
            PROCESSOR_FINALIZE_WRITE();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        }
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x41u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 4u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 3u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
    }
}

static u8 ReceiveScsiCommandPacket(void)
{
    u8 state = FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE);
    u8 index;

    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1596, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1597, 0u);
    if(state == 0x11u)
    {
        for(index = 0u; index < 12u; ++index)
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_OPCODE + index, FirmwareReadController8(FW_CONTROLLER_COMMAND_WINDOW));
        state = 5u;
    }
    else if(state == 0x10u)
    {
        state = 5u;
    }
    else
    {
        state = 4u;
    }
    FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);
    return state;
}

static void PublishScsiCommandProcessorCompletion(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u)
        PROCESSOR_FINALIZE_SCSI_WRITE();
    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 2u) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 1u) == 0u)
            PROCESSOR_PREPARE_COMPLETION();
        else
            FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xfeu));
        PROCESSOR_PUBLISH_COMPLETION();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xfdu));
    }
}

/* RAM:fdbe7d */
void ScsiCommandProcessorCoroutine0(void)
{
    u8 phase;

    InitializeScsiCommandProcessor();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x80u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 0u)
    {
        PROCESSOR_INITIALIZE_COROUTINE1(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
        PROCESSOR_SET_P3();
        PROCESSOR_YIELD();
    }
    for(;;)
    {
        while(FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) == 1u)
        {
            PrepareIdleScsiCommandProcessorYield();
            PROCESSOR_YIELD();
            ContinueIdleScsiCommandProcessorAfterYield();
        }

        phase = ReceiveScsiCommandPacket();
        if(phase == 4u)
        {
            PROCESSOR_CLEAR_COMPLETION();
            continue;
        }

        LED_COMMAND_BEGIN();
        if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PROTOCOL) == 0u)
        {
            if(PROCESSOR_ATA_DISPATCH() != 0u)
                PROCESSOR_RESTART();
        }
        else if(PROCESSOR_VALIDATE_SCSI() != 0u)
        {
            PROCESSOR_DISPATCH_SCSI();
        }
        LED_COMMAND_EXIT();

        if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        {
            PROCESSOR_CLEAR_COMPLETION();
            continue;
        }
        PublishScsiCommandProcessorCompletion();
    }
}
