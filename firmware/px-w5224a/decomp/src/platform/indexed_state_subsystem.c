#include "indexed_state_subsystem.h"
#include "../media/strategy_profile_initialization.h"
#include "arithmetic.h"
#include "controller_current_profile_configuration.h"
#include "controller_fixed_profile_generation.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/indexed_state_subsystem_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadIndexedStateWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* RAM:fd8043 */
/* non-returning. */
void InitializeIndexedStateSubsystem(void)
{
    u8 configuration[2];
    u16 configuration_value;

    ResetIndexedStateController();
    SetIndexedStateControllerFlag(1u);
    WaitIndexedStateDuration(20u, 1u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB4) & 0x80u) != 0u)
        WriteIndexedStateE8E9(0u);
    else
        WriteIndexedStateE8E9(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1008, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5D, 0x2cu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_113B, 0x50u);
    FirmwareWriteNear8(FW_NEAR_CURRENT_FRAME_POSITION, 0x00u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1154, 0x05u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1155, 0x7eu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1156, 0x40u);
    InitializeIndexedStateMediaStrategy();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, 0x28u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4C, 0x28u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x28u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 0x28u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, 0x28u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018001, 0x28u);
    BuildIndexedStateProfiles(ReadIndexedStateWord(FW_NEAR_UNKNOWN_F9FC));
    ConfigureIndexedStateProfile();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x40u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018032, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018033, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018034, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018035, 0x13u);

    ReadIndexedStateConfiguration(configuration);
    configuration_value = (u16)(((u16)configuration[0] << 8) | configuration[1]);
    WriteIndexedStateEAEB(configuration_value);
    SetIndexedStateHandshakeBits();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x10u));

    for(;;)
        YieldIndexedStateCoroutine1();
}
