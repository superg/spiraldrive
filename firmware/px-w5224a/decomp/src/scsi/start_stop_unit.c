#include "start_stop_unit.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/start_stop_unit_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadStartStopU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteStartStopU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 StartStopImmediate(void)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 1u);
}

static void SetStartStopP3(u8 bit0, u8 bit1)
{
    START_STOP_P3(0u, bit0);
    START_STOP_P3(1u, bit1);
}

static void ArmSecondaryOperationCountdown(void)
{
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 1u));
    if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 8u) != 0u)
        FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 60u);
    else
        FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
}

static void FinishStartStopReadRequest(void)
{
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u)
    {
        if(FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 8u));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0u)
                FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 60u);
            else
                FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 20u);
        }
    }
    else if((FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 1u || FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u) && (s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) < 0)
    {
        ArmSecondaryOperationCountdown();
    }
}

static void HandleStartStopAction0(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        START_STOP_STATUS_SENSE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
        return;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
    {
        START_STOP_SET_SENSE(2u, 0x0401u);
        return;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x40u) != 0u)
    {
        START_STOP_SET_SENSE(2u, 0x0404u);
        return;
    }

    SetStartStopP3(0u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 0u);
    START_STOP_RESET_COROUTINES();
    if(StartStopImmediate() == 0u)
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C3) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x40u) == 0u)
            START_STOP_YIELD();
    START_STOP_NO_SENSE();
    SetStartStopP3(1u, 0u);
    FinishStartStopReadRequest();
}

static void HandleStartStopAction1(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        START_STOP_STATUS_SENSE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
        return;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
    {
        START_STOP_SET_SENSE(2u, 0x0401u);
        return;
    }

    SetStartStopP3(0u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
    WriteStartStopU32(FW_NEAR_UNKNOWN_0C8E, ReadStartStopU32(FW_NEAR_UNKNOWN_0E3B));
    if(StartStopImmediate() == 0u)
        START_STOP_DISPATCH_AND_WAIT();
    else
    {
        START_STOP_DISPATCH_PENDING();
        START_STOP_NO_SENSE();
    }
    SetStartStopP3(1u, 0u);
    FinishStartStopReadRequest();
}

static void HandleStartStopAction2(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C35) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C36) == 0u)
    {
        START_STOP_SET_SENSE(5u, 0x5302u);
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u)
    {
        START_STOP_SET_SENSE(2u, 0x0404u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u && (s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) < 0)
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 8u) != 0u)
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));

    if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 1u) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 0x20u));
        START_STOP_NO_SENSE();
        START_STOP_PREPARE_ATA();
        START_STOP_PUBLISH_ATA();
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 2u));
        START_STOP_PREPARE_WRITE();
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xdfu));
    }

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 9u)
        START_STOP_YIELD();
    SetStartStopP3(0u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C4, 0u);
    START_STOP_ACTION2_WORKER();
    SetStartStopP3(1u, 1u);
    if(StartStopImmediate() == 0u)
    {
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C4) == 0u)
            START_STOP_YIELD();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 10u)
            START_STOP_YIELD();
    }
    START_STOP_NO_SENSE();
}

static void HandleStartStopAction3(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 4u) == 0u)
    {
        START_STOP_UNSUPPORTED();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xfdu));
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 10u)
        START_STOP_YIELD();
    START_STOP_ACTION3_WORKER();
    if(StartStopImmediate() == 0u)
    {
        while((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 2u) == 0u)
            START_STOP_YIELD();
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 9u)
            START_STOP_YIELD();
    }
    START_STOP_YIELD();

    if(StartStopImmediate() == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u)
            START_STOP_SET_SENSE(2u, 0x0401u);
    }
    else
    {
        while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u)
            START_STOP_YIELD();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 15u)
            START_STOP_SET_SENSE(2u, 0x5300u);
        else
            START_STOP_NO_SENSE();
    }
}

/* RAM:fb2520 */
void HandleStartStopUnitCommand(void)
{
    u8 action;

    if(START_STOP_VALIDATE(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E0C)) == 0u)
    {
        START_STOP_UNSUPPORTED();
        return;
    }

    action = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E);
    if(action == 0u)
        HandleStartStopAction0();
    else if(action == 1u)
        HandleStartStopAction1();
    else if(action == 2u)
        HandleStartStopAction2();
    else if(action == 3u)
        HandleStartStopAction3();
    else
        START_STOP_UNSUPPORTED();
}
