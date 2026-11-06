#include "firmware_entry.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/firmware_entry_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fee7ed */
void InitializeAndEnterFirmwareMain(void)
{
    FirmwareEntryResetStackAndEnter();
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/firmware_entry_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* Addressless C continuation entered after the native SPX reset. */
void InitializeAndEnterFirmwareMainAfterStackReset(void)
{
    FirmwareEntryClearInterruptEnable();
    FirmwareEntryClearInterruptControl();
    FirmwareEntryInitializeHardware();
    FirmwareEntryInitializeDriveSubsystems();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FB, 1u);
    FirmwareEntryEnableInterrupts();
    FirmwareEntryRunMainLoop();
}

/* RAM:fee819 */
void RestartFirmware(void)
{
    FirmwareEntryClearInterruptEnable();
    FirmwareEntryClearInterruptControl();
    InitializeAndEnterFirmwareMain();
}
