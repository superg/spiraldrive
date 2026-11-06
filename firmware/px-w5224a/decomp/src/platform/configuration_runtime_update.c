#include "configuration_runtime_update.h"
#include "../boot/startup_progress_diagnostics.h"
#include "../startup/debounced_inputs.h"
#include "../startup/drive_control_state10.h"
#include "configuration_range_update.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/configuration_runtime_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/configuration_runtime_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/configuration_runtime_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/configuration_runtime_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/configuration_runtime_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadImageU16(const u8 *image, u8 offset)
{
    return (u16)(((u16)image[offset] << 8) | image[(u8)(offset + 1u)]);
}

static void WriteImageU16(u8 *image, u8 offset, u16 value)
{
    image[offset] = (u8)(value >> 8);
    image[(u8)(offset + 1u)] = (u8)value;
}

static u32 ReadImageU32(const u8 *image, u8 offset)
{
    u32 value = (u32)image[offset] << 24;

    value |= (u32)image[(u8)(offset + 1u)] << 16;
    value |= (u32)image[(u8)(offset + 2u)] << 8;
    value |= image[(u8)(offset + 3u)];
    return value;
}

static void WriteImageU32(u8 *image, u8 offset, u32 value)
{
    image[offset] = (u8)(value >> 24);
    image[(u8)(offset + 1u)] = (u8)(value >> 16);
    image[(u8)(offset + 2u)] = (u8)(value >> 8);
    image[(u8)(offset + 3u)] = (u8)value;
}

static u16 ReadFirmwareU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadFirmwareU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteFirmwareU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fb0bac */
void PLEXTOR_NEAR_FUNCTION UpdateDriveConfigurationRange6CFromRuntimeState(void)
{
    u8 image[18];
    u16 sample_count;
    u8 drive_control_flags;

    if(ReadFirmwareU32(FW_NEAR_UNKNOWN_14D6) == 0ul && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x01u) == 0u && ReadFirmwareU32(FW_NEAR_UNKNOWN_FBED) == 0ul)
        return;

    ReadConfigurationRange6C(image);
    sample_count = ReadImageU16(image, 12u);
    if(sample_count == 0xffffu || sample_count < 10000u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x01u) != 0u)
        {
            WriteImageU16(image, 8u, ReadFirmwareU16(FW_NEAR_UNKNOWN_0C3A));
            image[7] = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEA);
            image[10] = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D);
        }
        WriteImageU32(image, 14u, ReadImageU32(image, 14u) + ReadFirmwareU32(FW_NEAR_UNKNOWN_14D6));
        WriteImageU16(image, 12u, (u16)(sample_count + 1u));
        if(ReadImageU16(image, 4u) != 0xffffu)
        {
            WriteImageU16(image, 4u, 0xffffu);
            WriteImageU32(image, 0u, 0ul);
        }
        WriteImageU32(image, 0u, ReadImageU32(image, 0u) + ReadFirmwareU32(FW_NEAR_UNKNOWN_FBED));
        ProgramConfigurationRange6C(image);
    }

    drive_control_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_1302);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(drive_control_flags & 0xfeu));
    WriteFirmwareU32(FW_NEAR_UNKNOWN_14D6, 0ul);
}

static void HandleStartupConfigurationUpdateResult(u8 update_result)
{
    if(update_result != 0u)
        return;

    DispatchStartupDriveControlState10Sequence();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C4, 1u);
    DispatchStartupRestartCoroutine1AndYield(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
}

/* RAM:fb0a9c */
void PLEXTOR_NEAR_FUNCTION ApplyStartupDriveConfigurationMaintenance(void)
{
    SetStartupMaintenanceFlagForActiveDriveState();

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) >> 5) & 1u) == 1u)
    {
        DispatchStartupConfigurationByteRangeProgram(0x0000fbdaul, 0x0060u, 12u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x13u))
            HandleStartupConfigurationUpdateResult(DispatchStartupConfigurationRangeUpdate(0x0000fabdul, 0x0029u, 3u));

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
            HandleStartupConfigurationUpdateResult(DispatchStartupConfigurationRangeUpdate(0x0000f9b5ul, 0x003cu, 1u));
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB1) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) != 5u)
        HandleStartupConfigurationUpdateResult(DispatchStartupConfigurationRangeUpdate(0x0000fab4ul, 0x0020u, 26u));

    DispatchStartupRuntimeRange6CUpdate();
}

/* RAM:f84b0b */
void RunDriveInitializationCoroutine1(void)
{
    u8 initialization_result;

    STARTUP_DIAGNOSTIC_MARK(4u);
    DriveInitConfigureRing(2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
    DriveInitRegisters();
    DriveInitClearPorts();
    DriveInitAbortController();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 3u)
        FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x7fu));
    ClearDriveInitBit000E();
    FirmwareWriteNear8(FW_NEAR_FORMAT_OPERATION_ACTIVE, 0u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x7fu));
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 3) & 1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xefu));
        DriveInitResetChannel2();
    }

    initialization_result = DriveInitSequenceResult();
    STARTUP_DIAGNOSTIC_MARK(5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1584, 0xffu);
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 2) & 1u) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 0x18u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xfbu));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C4, 1u);

    if(initialization_result == 2u)
    {
        STARTUP_INTERVAL_CHECK(1u);
        DriveInitYield();
        STARTUP_INTERVAL_CHECK(2u);
        DriveInitRestart(FW_ENTRY_RUN_STARTUP_SEQUENCE_COROUTINE1);
        return;
    }
    if(initialization_result == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 0x0fu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 3u);
        STARTUP_INTERVAL_CHECK(1u);
        DriveInitYield();
        STARTUP_INTERVAL_CHECK(2u);
        DriveInitRestart(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
        return;
    }

    STARTUP_INTERVAL_CHECK(1u);
    DriveInitYield();
    STARTUP_INTERVAL_CHECK(2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFB, 1u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018004, 1u);
    DriveInitCoroutine3(FW_ENTRY_IDLE_COROUTINE3);
    DriveInitRestart(FW_ENTRY_RUN_STARTUP_SEQUENCE_COROUTINE1);
}

/* RAM:f84c30 */
void RunStartupSequenceCoroutine1(void)
{
    STARTUP_ROUTE_CHECK();
    STARTUP_INTERVAL_CHECK(3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfau));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1309, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 1u);
    STARTUP_INTERVAL_CHECK(4u);
    WriteFirmwareU32(FW_NEAR_UNKNOWN_1598, 0x80000000ul);
    STARTUP_INTERVAL_CHECK(5u);
    WriteFirmwareU32(FW_NEAR_UNKNOWN_1484, 0x80000000ul);
    STARTUP_INTERVAL_CHECK(6u);
    StartupSequenceClearBit0017();
    STARTUP_INTERVAL_CHECK(7u);
    STARTUP_DIAGNOSTIC_MARK(6u);
    StartupSequenceInitializeMediaState();
    STARTUP_DIAGNOSTIC_MARK(7u);

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) >= 0)
        StartupSequenceSetP3PinsHigh();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x7fu));

    StartupSequenceEnsureStateFlag11EF();
    StartupSequenceSetP1TimerPinsLow();
    StartupSequenceWriteD6Value7E();
    StartupSequenceDebouncedInputs();
    StartupSequenceSetP1TimerPinsHigh();
    StartupSequenceWriteD6Value00();
    StartupSequenceRestart(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COMPLETION_COROUTINE1);
}

/* RAM:f852eb */
void RunDriveInitializationCompletionCoroutine1(void)
{
    u8 result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD7, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD8, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBD9, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBC, 3u);
    DriveInitCompletionSetP3PinsHigh();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15C4, 0u);

    result = DriveInitCompletionState9Result();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x02u));
    if(result == 2u)
    {
        DriveInitCompletionYield();
        DriveInitCompletionRestart(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
        return;
    }
    if(result == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 0x0fu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 3u);
        DriveInitCompletionYield();
        DriveInitCompletionRestart(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 3u);
    if(DriveInitCompletionIndexedStateActive(0x0bu) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_120A, 5u);
        while(DriveInitCompletionCountdown() > 0)
        {
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 0u);
        DriveInitCompletionRestart(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
        return;
    }

    DriveInitCompletionYield();
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) >> 3) & 1u) == 1u)
        DriveInitCompletionRestart(FW_ENTRY_PROCESS_DRIVE_STATE_MACHINE);
    else
        DriveInitCompletionRestart(FW_ENTRY_RUN_STARTUP_SEQUENCE_COROUTINE1);
}
