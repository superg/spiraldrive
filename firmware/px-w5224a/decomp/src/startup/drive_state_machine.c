#include "drive_state_machine.h"
#include "../boot/startup_progress_diagnostics.h"
#include "../platform/firmware_memory.h"
#include "../platform/mcs251.h"
#include "controller_calibration_configuration.h"
#include "indexed_drive_state_transitions.h"
#include "indexed_read_state_configuration.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/drive_state_machine_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteDriveStateMachineU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void PublishDriveRecognitionResult(u8 result)
{
    if(result == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x7fu));
    }
    else if(result == 0x11u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 4u);
    }
    else if(result == 0x14u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0x0cu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 6u);
    }
    else if(result == 0x15u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0x0eu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 6u);
    }
    else if(result == 0x16u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0x0fu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 6u);
    }
    else if(result == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 5u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 4u);
    }
}

/* RAM:f84f6a */
void ProcessDriveStateMachine(void)
{
    u8 recognition_result;

    STARTUP_DIAGNOSTIC_MARK(8u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
        DRIVE_STATE_WRITE_P3(0u, 1u);
    else
        DRIVE_STATE_WRITE_P3(0u, 0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x20u) != 0u)
        DRIVE_STATE_LOAD_CALIBRATION();
    DRIVE_STATE_LOAD_OFFSETS();
    DRIVE_STATE_CLEAR_COMPLETION();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 0u);
    WriteDriveStateMachineU16(FW_NEAR_UNKNOWN_1004, 0x0078u);
    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEB) == 0u)
        DRIVE_STATE_YIELD();

    recognition_result = DRIVE_STATE_RECOGNIZE();
    PublishDriveRecognitionResult(recognition_result);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, FirmwareReadNear8(FW_NEAR_UNKNOWN_1009));
    DRIVE_STATE_INDEXED_TRANSITIONS();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) != 0u)
        DRIVE_STATE_RESTART(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);

    DRIVE_STATE_CONFIGURE_MEDIA_READ();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        DRIVE_STATE_BUILD_INTERPOLATION();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 1u)
        DRIVE_STATE_INITIALIZE_MEDIA_READ();
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x0eu || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x0fu)
        DRIVE_STATE_INITIALIZE_MEDIA_READ();
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 2u));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 0u);
    DRIVE_STATE_WRITE_P3(1u, 0u);
    if(DRIVE_STATE_INDEXED_STATE(0x0du) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        DRIVE_STATE_RESTART(FW_ENTRY_RUN_MEDIA_READ_TRANSITION_COROUTINE1);
    else
        DRIVE_STATE_RESTART(FW_ENTRY_RUN_MEDIA_STATE_COROUTINE1);
}
