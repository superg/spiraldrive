#include "controller_command_preparation.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_memory.h"
#include "../platform/timing.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/controller_command_preparation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBigEndianU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteBigEndianU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fdc28c */
void PLEXTOR_NEAR_FUNCTION PrepareControllerForScsiCommandProcessing(void)
{
    u8 controller_status = 1u;

    if(FirmwareReadBit0008() != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F46) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 3u)
        SET_CONTROLLER_FLAG(0u);

    while((READ_PORT_8058() & 4u) == 4u)
    {
    }

    CONFIGURE_CONTROLLER(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FD, 0u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) | 0x10u));
    WriteBigEndianU16(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadBigEndianU16(FW_NEAR_UNKNOWN_FAD7) | 1u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E751) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E751, 0u);
        SET_P1_TIMER_PINS();
        ENABLE_EXTERNAL_INTERRUPT0();
        WRITE_PARAMETER_D6();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 2u)
            SET_P3_HANDSHAKE();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xdfu));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C79) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 2u) == 0u)
    {
        DELAY_TIMER_UNITS_59();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1204, 0x1fu);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 4u);
        while((READ_PORT_8043() & 0x10u) != 0u)
        {
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1204) <= 0)
            {
                controller_status |= 0x80u;
                break;
            }
            YIELD_COROUTINE0();
        }
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018041, 0u);
    }

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018020, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01801F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT1, 1u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_TRIGGER, 0u);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_ARGUMENT0, controller_status);
    FirmwareWriteController8(FW_CONTROLLER_COMMAND_CODE, 0x80u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01805B, 0x37u);
}
