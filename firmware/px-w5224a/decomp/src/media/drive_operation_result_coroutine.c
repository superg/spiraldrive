#include "drive_operation_result_coroutine.h"
#include "../platform/configuration_range_update.h"
#include "../platform/coroutine_entries.h"
#include "../platform/coroutine_initialization.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "../platform/timer_debounce.h"
#include "../scsi/read_verify_range.h"
#include "../startup/drive_control_state10.h"
#include "raw_read_operation_mode_speed.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/drive_operation_result_coroutine_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadDriveOperationResultU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteDriveOperationResultU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void StartRawReadOperationForDriveState(u8 state)
{
    switch(state)
    {
    case 1u:
        RESULT_EXECUTE(0u, 0x18u);
        break;
    case 2u:
        RESULT_EXECUTE(0u, 0x20u);
        break;
    case 3u:
        RESULT_EXECUTE(0u, 0x28u);
        break;
    case 4u:
        RESULT_EXECUTE(0u, 0x34u);
        break;
    case 6u:
        RESULT_EXECUTE(3u, 0x30u);
        break;
    case 7u:
        RESULT_EXECUTE(0u, 0x14u);
        break;
    case 0x11u:
        RESULT_EXECUTE(4u, 2u);
        break;
    case 0x14u:
        RESULT_EXECUTE(0u, 0x10u);
        break;
    case 0x15u:
        RESULT_EXECUTE(0u, 8u);
        break;
    case 0x16u:
        RESULT_EXECUTE(0u, 4u);
        break;
    case 0x17u:
        RESULT_EXECUTE(0u, 1u);
        break;
    case 0x1fu:
        RESULT_EXECUTE(3u, 0x10u);
        break;
    case 0xf2u:
        RESULT_EXECUTE(0u, 0x0cu);
        break;
    case 0xfdu:
        RESULT_EXECUTE(0u, 2u);
        break;
    case 0xffu:
        RESULT_EXECUTE(0u, 0x24u);
        break;
    default:
        break;
    }
}

static void RestartStartupStatusAfterConfigurationFailure(void)
{
    RESULT_STATE10();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C4, 1u);
    RESULT_RESTART(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
}

/* RAM:feb6bb */
void RunDriveOperationResultCoroutine0(void)
{
    u16 configuration_word;

    StartRawReadOperationForDriveState(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));

    configuration_word = ReadDriveOperationResultU16(FW_NEAR_UNKNOWN_FAAC);
    configuration_word = (u16)((configuration_word & 0xf9ffu) | 0x0400u);
    WriteDriveOperationResultU16(FW_NEAR_UNKNOWN_FAAC, configuration_word);

    if(RESULT_INDEXED(4u) != 0u)
    {
        if(RESULT_CONFIGURATION(FW_NEAR_UNKNOWN_FB96, 0x003cu, 2u) == 0u)
            RestartStartupStatusAfterConfigurationFailure();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD6) | 0xf0u));
            if(RESULT_CONFIGURATION(FW_NEAR_UNKNOWN_FAD6, 0x003eu, 1u) == 0u)
                RestartStartupStatusAfterConfigurationFailure();
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE9, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF1, 2u);
            RESULT_RECOVERY();
        }
        for(;;)
            RESULT_YIELD();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C3, 0u);
    do
    {
        RESULT_YIELD();
    } while(FirmwareReadNear8(FW_NEAR_UNKNOWN_15C3) == 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
    {
        RESULT_STATUS_UPDATE();
        RESULT_PATTERN(1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
        for(;;)
        {
            RESULT_PATTERN(0u);
            RESULT_YIELD();
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) | 0x30u));
    RESULT_INITIALIZE(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
    for(;;)
        RESULT_YIELD();
}
