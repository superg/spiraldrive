#include "host_interface_interrupt.h"
#include "../boot/led_boot_diagnostics.h"
#include "../media/raw_read_descriptor_completion.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

extern void HandleRawReadHostTransferCompletion(void);
extern void CompleteBatchedHostTransferBuffers(void);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/host_interface_interrupt_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadHostInterruptU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u8 HostTransferCountIsComplete(void)
{
    return (u8)(ReadHostInterruptU32(FW_NEAR_UNKNOWN_1455) == ReadHostInterruptU32(FW_NEAR_UNKNOWN_1459));
}

static void DisableControllerQueueIfIdle(void)
{
    if(READ_CONTROLLER_QUEUE_BIT() != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 3u)
        SetControllerFlagFC07Bit2AndPort8098Bit6(0u);
}

static void PublishControllerBufferRecoveryCommand(void)
{
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, 0x14u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800E, 0xebu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) | 0x08u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, 0xf7u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01800D, 0xffu);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018022, 0xe0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA));
}

static void SelectHostHandshakeResponse(u8 state, u8 missing_response, u8 write_success_response)
{
    if(state == 1u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0u);
    }
    else if(state == 2u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 4u);
        if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043) & 0x10u) == 0u || (FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018043) & 0x20u) == 0u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, missing_response);
        else if(write_success_response != 0u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 0x10u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0u);
    }
    else if(state == 0u)
    {
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018046, 2u);
    }
    else if(state == 3u)
    {
        for(;;)
        {
        }
    }
}

static void CompleteHostDataTransfer(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C33) == 8u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 0x0du);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1597, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1597) | ((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01805D) & 0x10u) >> 4)));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1462) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u && HostTransferCountIsComplete() != 0u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127E) != 0u)
            HandleRawReadHostTransferDescriptorCompletion();
        else
            HandleRawReadHostTransferCompletion();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F47) != 0u)
    {
        CompleteBatchedHostTransferBuffers();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u && HostTransferCountIsComplete() != 0u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, 0u);
        return;
    }

    if(READ_HOST_TRANSFER_BIT() != 0u)
    {
        CompleteSingleHostTransferBuffer();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u && HostTransferCountIsComplete() != 0u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, 0u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, 0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 8u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 2u));
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x50u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 0u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0x80u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C33, 5u);
    }
}

static void HandleHostCommandInterrupt(u8 cause, u8 host_flag)
{
    u8 state;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xdfu));
    state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C79);
    if(state == 0u || state == 1u)
    {
        if(host_flag != 0u)
            return;
    }
    else if(state == 2u)
    {
        if(host_flag == 0u)
        {
            if((cause & 0x40u) != 0u || FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018057) != 0x90u)
                return;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) | 0x20u));
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x20u) == 0u)
            SelectHostHandshakeResponse(2u, 0x10u, 0u);
    }

    if((cause & 0x40u) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 0x11u);
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_PROTOCOL, 1u);
        }
    }
    else if(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018057) == 0xfdu)
    {
        DisableControllerQueueIfIdle();
        PublishControllerBufferRecoveryCommand();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FD, 0x98u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FA, 1u);
    }
    else if(FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 0x10u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_PROTOCOL, 0u);
    }
}

static void HandleHostStatusInterrupt(u8 cause, u8 host_flag)
{
    u8 state;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 1u) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 2u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, FirmwareReadNear8(FW_NEAR_UNKNOWN_158C));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_158C));
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158B, 0u);
    }

    if((cause & 0x30u) == 0x30u)
    {
        state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C79);
        if(state == 0u || state == 1u)
        {
            if(host_flag != 0u)
                return;
        }
        else if(host_flag == 0u)
        {
            return;
        }
        FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 1u));
        FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x20u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1584, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1584) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC20, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xdfu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 8u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 0x40u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x7fu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x10u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u)
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018049, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E751) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E751, 0u);
            SET_RECOVERY_INTERRUPT_PINS();
            WriteControllerParameterD6Value00();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 2u)
                SET_RECOVERY_P3_PATTERN();
        }
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 1u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 1u);
        PublishControllerBufferRecoveryCommand();
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0x80u);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x20u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FD, 0x98u);
        DisableControllerQueueIfIdle();
        if((cause & 0x30u) == 0x10u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FE, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FA, 1u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) | 0x20u));
        }
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xdfu));
    SelectHostHandshakeResponse(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C79), 0xd0u, 1u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 1u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 1u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0x80u);
}

/* RAM:fdd3bf */
void HandleHostInterfaceInterrupt(void)
{
    u8 cause;
    u8 host_flag;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEB, 1u);
    cause = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01805A);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_131B, cause);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018059, 0x17u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA) & 0xe7u));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FCA));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805B, 0x37u);
    host_flag = (u8)(FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018055) & 0x10u);

    if((cause & 0x30u) != 0u)
    {
        HandleHostStatusInterrupt(cause, host_flag);
    }
    else if((cause & 1u) != 0u)
    {
        HandleHostCommandInterrupt(cause, host_flag);
    }
    else if((cause & 2u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1596, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1459, FirmwareReadNear8(FW_NEAR_UNKNOWN_1455));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_145A, FirmwareReadNear8(FW_NEAR_UNKNOWN_1456));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_145B, FirmwareReadNear8(FW_NEAR_UNKNOWN_1457));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_145C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1458));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1462, 0u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x10u));
    }
    else if((cause & 4u) != 0u)
    {
        CompleteHostDataTransfer();
        LED_INQUIRY_MARK(4u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FA) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x20u) == 0u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018059, 0x97u);
    else
        LED_INQUIRY_RESET();
}
