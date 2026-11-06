#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFirmwareEntryEvent(u8 event);
#define FirmwareEntryResetStackAndEnter() ObserveFirmwareEntryEvent(1u)
#define FirmwareEntryClearInterruptEnable() ObserveFirmwareEntryEvent(2u)
#define FirmwareEntryClearInterruptControl() ObserveFirmwareEntryEvent(3u)
#define FirmwareEntryInitializeHardware() ObserveFirmwareEntryEvent(4u)
#define FirmwareEntryInitializeDriveSubsystems() ObserveFirmwareEntryEvent(5u)
#define FirmwareEntryEnableInterrupts() ObserveFirmwareEntryEvent(6u)
#define FirmwareEntryRunMainLoop() ObserveFirmwareEntryEvent(7u)
#else
extern void ResetFirmwareStackAndEnterMainContinuation(void);
extern void InvokeRuntimeHardwareInitializationEntry(void);
extern void InitializeDriveSubsystems(void);
extern void EnableFirmwareInterruptSources(void);
extern void FirmwareMainLoop(void);
sfr FirmwareEntryInterruptEnable = 0xa8;
sfr FirmwareEntryInterruptControl = 0xa9;
#define FirmwareEntryResetStackAndEnter() ResetFirmwareStackAndEnterMainContinuation()
#define FirmwareEntryClearInterruptEnable()                                                                                                                                                            \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareEntryInterruptEnable = 0u;                                                                                                                                                             \
    } while(0)
#define FirmwareEntryClearInterruptControl()                                                                                                                                                           \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        FirmwareEntryInterruptControl = 0u;                                                                                                                                                            \
    } while(0)
#define FirmwareEntryInitializeHardware() InvokeRuntimeHardwareInitializationEntry()
#define FirmwareEntryInitializeDriveSubsystems() InitializeDriveSubsystems()
#define FirmwareEntryEnableInterrupts() EnableFirmwareInterruptSources()
#define FirmwareEntryRunMainLoop() FirmwareMainLoop()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
InitializeAndEnterFirmwareMainAfterStackReset();
#else
for(;;)
{
}
#endif
#endif
