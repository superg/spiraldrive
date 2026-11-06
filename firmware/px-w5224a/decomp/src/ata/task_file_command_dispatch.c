#include "task_file_command_dispatch.h"
#include "../platform/controller_data_out.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_entry.h"
#include "../platform/firmware_memory.h"
#include "../platform/power_control.h"
#include "../platform/status_mapping.h"
#include "../platform/timing.h"
#include "identify_packet_device.h"

extern void YieldCoroutine0(void);
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/ata/task_file_command_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void PrepareAtaControllerCommandTransfer(void)
{
    u8 shadow;

    CLEAR_GLOBAL_INTERRUPT();
    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, 0xf7u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, 0xffu);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0xe0u);
    shadow = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, shadow);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, shadow);
    SET_GLOBAL_INTERRUPT();
    SET_CONTROLLER_INTERRUPT();
    SET_SFR_A9_BIT2();
}

static void WriteAtaControllerTransferPrefix(void)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, 0x14u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, 0xebu);
}

static void CompleteAtaCommonResetCommand(void)
{
    WriteAtaControllerTransferPrefix();
    PrepareAtaControllerCommandTransfer();
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_PROTOCOL, 1u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0x51u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 0x04u);
}

static void HandleAtaDiagnosticCommand(void)
{
    WriteAtaControllerTransferPrefix();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C79) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0C79) == 0u)
    {
        DelayTimerUnits59();
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 4u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_00FF) & 0x01u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_120F, 60u);
            while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043) & 0x10u) != 0u)
            {
                if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_120F) <= 0)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 0x81u);
                    break;
                }
                YieldCoroutine0();
            }
        }
        else
        {
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 1u);
        }
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0u);
    }
    else
    {
        DelayTimerUnits59();
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0xd0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 1u);
    }
    PrepareAtaControllerCommandTransfer();
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_PROTOCOL, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) | 0x20u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0u);
}

static void CompleteAtaPowerCommand(void)
{
    u8 state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);

    if(state != 1u && state != 3u && state != 2u)
        ResetMediaProcessingCoroutines();
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x02u));
}

static void EnterAtaSleepState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E751, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 2u)
    {
        ResetMediaProcessingCoroutines();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 0u);
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C3) == 0u)
            YieldCoroutine0();
    }
    SET_P3_BITS0_AND1();
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x02u));
    CLEAR_P1_TIMER_PINS();
    WriteControllerParameterD6Value7E();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018017, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800C, 0x11u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018040, (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018040) | 0x10u));
    ENABLE_TIMER0_INTERRUPT();
    EnterPowerDownModeWithInterruptsEnabled();
}

static void RestartFromAtaCommand(void)
{
    WriteAtaControllerTransferPrefix();
    PrepareAtaControllerCommandTransfer();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FD, 0x98u);
    RestartFirmware();
}

static void CompleteUnsupportedAtaCommand(void)
{
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x41u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 4u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 3u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x02u));
}

/* RAM:fdb06a */
u8 HandleAtaTaskFileCommand(void)
{
    u8 command = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018057);

    switch(command)
    {
    case 0x20u:
    case 0xdau:
    case 0xecu:
        CompleteAtaCommonResetCommand();
        return 0u;
    case 0x90u:
        HandleAtaDiagnosticCommand();
        return 0u;
    case 0xa1u:
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 0x08u));
        BuildAtaIdentifyPacketDeviceData();
        ExecuteControllerDataInTransfer(ReadBigEndianU32(FW_NEAR_UNKNOWN_155A), 0x200ul);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x08u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x02u));
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x01u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0xd0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xf7u));
        }
        return 0u;
    case 0xe0u:
    case 0xe1u:
        CompleteAtaPowerCommand();
        return 0u;
    case 0xe6u:
        EnterAtaSleepState();
        return 0u;
    case 0xefu:
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_PROTOCOL, 1u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x01u));
        if(ApplyAtaSetFeaturesCommand() != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0x50u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 0u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0x41u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 4u);
        }
        return 0u;
    case 0xfdu:
        RestartFromAtaCommand();
        return 0u;
    case 0xfeu:
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x01u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0x40u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 0u);
        return 0u;
    case 0xffu:
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x01u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2F, 0x40u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C30, 0u);
        return 0x64u;
    default:
        CompleteUnsupportedAtaCommand();
        return 0u;
    }
}
