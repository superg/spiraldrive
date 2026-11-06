#include "host_command_dispatch.h"
#include "../platform/bindings/ata_bindings.h"
#include "../platform/firmware_memory.h"

/* C251 O5/SIZE emits these adapters out of line.  Keep them local to preserve
 * the established register allocation and linked layout. */
static u32 ReadHostDispatchU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteHostDispatchU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fa34b2 */
void DispatchCurrentHostCommandAndPublishCompletion(void)
{
    u8 completion_flags;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & HOST_COMMAND_STATUS_BOUNDARY_REACHED) == 0u)
    {
        s32 upper_boundary = (s32)ReadHostDispatchU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY);
        s32 mapped_position = (s32)ReadHostDispatchU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION);

        if(mapped_position > upper_boundary)
            FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_STATUS, HOST_COMMAND_STATUS_BOUNDARY_REACHED);
    }

    if(FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH) != 0u)
    {
        FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH, 0u);
        FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_STATUS, HOST_COMMAND_STATUS_FAILURE);
        FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, (FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0u ? 60u : 20u);
        return;
    }

    if(FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) != 0u)
    {
        FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_EVENTS, HOST_COMMAND_EVENT_ACTIVE_RECORD);
        FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_STATUS, HOST_COMMAND_STATUS_BOUNDARY_REACHED);
        FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & HOST_COMPLETION_CLEAR_REQUESTED) != 0u)
    {
        ClearCurrentHostCompletion();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PROTOCOL) == 0u)
    {
        if(DispatchAtaForCurrentHostCommand() != 0u)
            RestartAfterHostCommand();
    }
    else
    {
        DispatchScsiForCurrentHostCommand(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE));
    }

    completion_flags = FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS);
    if((completion_flags & HOST_COMPLETION_DEFERRED) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & HOST_COMPLETION_PREPARED) == 0u)
            PrepareCurrentHostCompletion();
        else
            FirmwareClearNearBits8(FW_NEAR_HOST_COMPLETION_FLAGS, HOST_COMPLETION_PREPARED_CLEAR_MASK);
        PublishCurrentHostCompletion();
        return;
    }

    FirmwareClearNearBits8(FW_NEAR_HOST_COMPLETION_FLAGS, HOST_COMPLETION_DEFERRED_CLEAR_MASK);
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & HOST_COMMAND_STATUS_RETRYABLE_READ) != 0u
        && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x08u || FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x28u || FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0xa8u
            || FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0xbeu))
    {
        FirmwareSetNearBits8(FW_NEAR_HOST_COMPLETION_FLAGS, HOST_COMPLETION_DEFERRED);
        FirmwareClearNearBits8(FW_NEAR_HOST_COMMAND_STATUS, HOST_COMMAND_STATUS_RETRYABLE_READ_CLEAR_MASK);
    }
}

/* RAM:fa4103 */
void ServiceCurrentHostCommandDuringActiveOperation(void)
{
    u8 format_state;
    u8 completion_flags;
    u16 scan_result;
    u32 position;

    if(FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH) == 1u)
    {
        FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 0u);
        FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x04u));
        if(FirmwareReadNear8(FW_NEAR_FORMAT_OPERATION_ACTIVE) == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u)
            {
                FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_RESULT, 1u);
                FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_EVENTS, HOST_COMMAND_EVENT_FORMAT_TRANSITION);
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_RESULT, 3u);
                FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 0x01u));
            }
        }
        else
        {
            format_state = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);
            if(format_state == 1u || FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u)
                FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 0x01u));
        }
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfeu));
        return;
    }

    format_state = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);
    if(format_state == 1u || FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u)
        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfeu));

    position = ReadHostDispatchU32(FW_NEAR_CURRENT_FRAME_POSITION);
    scan_result = ScanActiveHostControllerBanks(1u, position, 0l);
    if(scan_result == 0u)
    {
        position = ReadHostDispatchU32(FW_NEAR_CURRENT_FRAME_POSITION);
        WriteHostDispatchU32(FW_NEAR_SCAN_POSITION_SECOND, position);
        WriteHostDispatchU32(FW_NEAR_SCAN_POSITION_FIRST, position);
    }
    else
    {
        position = ReadHostDispatchU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT);
        WriteHostDispatchU32(FW_NEAR_SCAN_POSITION_FIRST, position);
        position = ReadHostDispatchU32(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT);
        WriteHostDispatchU32(FW_NEAR_SCAN_POSITION_SECOND, position);
    }

    if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & HOST_COMPLETION_CLEAR_REQUESTED) != 0u)
    {
        ClearCurrentHostCompletion();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PROTOCOL) == 0u)
    {
        if(DispatchAtaForCurrentHostCommand() != 0u)
            RestartAfterHostCommand();
    }
    else
    {
        DispatchScsiForCurrentHostCommand(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE));
    }

    completion_flags = FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS);
    if((completion_flags & HOST_COMPLETION_DEFERRED) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & HOST_COMPLETION_PREPARED) == 0u)
            PrepareCurrentHostCompletion();
        else
            FirmwareClearNearBits8(FW_NEAR_HOST_COMPLETION_FLAGS, HOST_COMPLETION_PREPARED_CLEAR_MASK);
        PublishCurrentHostCompletion();
    }
    else
    {
        FirmwareClearNearBits8(FW_NEAR_HOST_COMPLETION_FLAGS, HOST_COMPLETION_DEFERRED_CLEAR_MASK);
    }
}

/* RAM:fa457d */
void ReceiveAndDispatchHostPacketDuringActiveOperation(void)
{
    u8 index;
    u8 opcode;
    u8 pending_state;

    if(FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE) == 1u)
        return;

    pending_state = FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE);
    if(pending_state == 0x10u)
    {
        FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_EVENTS, HOST_COMMAND_EVENT_PACKET_PENDING);
        FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);
        return;
    }

    if(pending_state != 0x11u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);
        return;
    }

    for(index = 0u; index < 12u; ++index)
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_OPCODE + index, FirmwareReadController8(FW_CONTROLLER_COMMAND_WINDOW));
    FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);

    if(ValidateActiveHostAdmission() == 0u)
    {
        PrepareCurrentHostCompletion();
        PublishCurrentHostCompletion();
        return;
    }

    opcode = FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE);
    if(opcode == 0x08u || opcode == 0x0au || opcode == 0x28u || opcode == 0x2au || opcode == 0x2eu || opcode == 0x35u || opcode == 0xa1u || opcode == 0xa8u || opcode == 0xaau || opcode == 0xb9u
        || opcode == 0xbbu || opcode == 0xbeu || opcode == 0x04u)
    {
        FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_EVENTS, HOST_COMMAND_EVENT_PACKET_PENDING);
        return;
    }

    if(opcode == 0x1bu)
    {
        if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PACKET + 4u) != 3u)
        {
            FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_EVENTS, HOST_COMMAND_EVENT_PACKET_PENDING);
            if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PACKET + 4u) == 2u)
                FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 0x01u));
            return;
        }
    }
    else if(opcode == 0x4au)
    {
        if(FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH) != 0u)
            FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) & 0xfbu));
    }

    DispatchScsiForCurrentHostCommand(opcode);
    PrepareCurrentHostCompletion();
    PublishCurrentHostCompletion();
}

static u8 IsPendingHostDeferredOpcode(u8 opcode)
{
    return (u8)(opcode == 0x08u || opcode == 0x0au || opcode == 0x28u || opcode == 0x2au || opcode == 0x2eu || opcode == 0x35u || opcode == 0x5bu || opcode == 0xa1u || opcode == 0xa8u
                || opcode == 0xaau || opcode == 0xb9u || opcode == 0xbeu || opcode == 0x04u);
}

static void CompletePendingHostCommand(void)
{
    PrepareCurrentHostCompletion();
    PublishCurrentHostCompletion();
}

/* RAM:fb4739 */
void PLEXTOR_NEAR_FUNCTION ServicePendingHostCommandDuringMediaOperation(void)
{
    u8 index;
    u8 opcode;
    u8 pending_state = FirmwareReadNear8(FW_NEAR_HOST_PACKET_STATE);

    if(pending_state == 0x10u)
    {
        FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_STATUS, 0x04u);
        FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 1u) == 0u || (FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & HOST_COMPLETION_CLEAR_REQUESTED) != 0u)
            return;
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0x41u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, 4u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 3u);
        FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0xc0u);
        return;
    }

    if(pending_state == 0x11u)
    {
        for(index = 0u; index < 12u; index++)
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_OPCODE + index, FirmwareReadController8(FW_CONTROLLER_COMMAND_WINDOW));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);
        return;
    }
    FirmwareWriteNear8(FW_NEAR_HOST_PACKET_STATE, 1u);

    if(ValidateActiveHostAdmission() == 0u)
    {
        CompletePendingHostCommand();
        return;
    }

    opcode = FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x40u) != 0u)
    {
        if(opcode != 0x03u)
        {
            if(opcode != 0x52u)
            {
                SetPendingHostSense(2u, 0x0404u);
                CompletePendingHostCommand();
                return;
            }
            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
            {
                SetPendingHostSense(2u, 0x0408u);
                CompletePendingHostCommand();
                return;
            }
        }
        DispatchScsiForCurrentHostCommand(opcode);
        CompletePendingHostCommand();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x40u) != 0u && FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH) == 0u)
    {
        if(IsPendingHostDeferredOpcode(opcode) != 0u || (opcode == 0x1bu && FirmwareReadNear8(FW_NEAR_HOST_COMMAND_PACKET + 4u) == 1u))
        {
            FirmwareSetNearBits8(FW_NEAR_HOST_COMMAND_STATUS, 0x04u);
            return;
        }
        DispatchScsiForCurrentHostCommand(opcode);
        CompletePendingHostCommand();
        return;
    }

    if(opcode == 0x03u || opcode == 0x12u || opcode == 0x46u || opcode == 0xebu)
    {
        DispatchScsiForCurrentHostCommand(opcode);
    }
    else if(opcode == 0x4au)
    {
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u && FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH) != 0u)
            FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) & 0xfbu));
        DispatchScsiForCurrentHostCommand(opcode);
    }
    else if(opcode == 0x52u)
    {
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
            SetPendingHostSense(2u, 0x0408u);
        else
            DispatchScsiForCurrentHostCommand(opcode);
    }
    else if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0x20u) != 0u)
    {
        UpdatePendingHostTrayStatus();
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x20u) != 0u)
    {
        SetPendingHostSense(2u, 0x0404u);
    }
    else
    {
        SetPendingHostSense(2u, 0x0408u);
    }
    CompletePendingHostCommand();
}
