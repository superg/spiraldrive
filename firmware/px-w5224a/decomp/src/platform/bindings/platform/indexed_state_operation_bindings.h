#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveIndexedStateOperationPortWrite(fw_addr_t address, u8 value);
extern void ObserveIndexedStateOperationDrive(u8 drive_code, u8 enabled);
extern void ObserveIndexedStateOperationProfile(u8 enabled, u16 range_value);
extern void ObserveIndexedStateOperationTimerWrite(fw_addr_t address, u8 value);
extern u8 ObserveIndexedStateOperationTimerRead(fw_addr_t address);
extern u8 ObserveIndexedStateOperationInputBit2(void);
extern void ObserveIndexedStateOperationRestart(u32 entry_point);
extern u32 ObserveIndexedStateOperationAttemptLimit(void);
#define WriteOperationPort(address, value) ObserveIndexedStateOperationPortWrite((address), (value))
#define ConfigureOperationDrive(code, enabled) ObserveIndexedStateOperationDrive((code), (enabled))
#define ApplyOperationProfile(enabled, value) ObserveIndexedStateOperationProfile((enabled), (value))
#define WriteOperationTimer(address, value) ObserveIndexedStateOperationTimerWrite((address), (value))
#define ReadOperationTimer(address) ObserveIndexedStateOperationTimerRead(address)
#define ReadOperationInputBit2() ObserveIndexedStateOperationInputBit2()
#define RestartOperationCoroutine(entry) ObserveIndexedStateOperationRestart(entry)
#define OperationAttemptLimit() ObserveIndexedStateOperationAttemptLimit()
#else
extern void RestartCoroutine1AndYield(u32 entry_point);
#define WriteOperationPort(address, value) WaitControllerPortReadyAndWriteByte((address), (value))
#define ConfigureOperationDrive(code, enabled) ConfigureControllerDrivePorts8496Through849D((code), (enabled))
#define ApplyOperationProfile(enabled, value) ConfigureControllerStateProfileFromFirmwareDefaults((enabled), (value))
#define WriteOperationTimer(address, value) FirmwareWrite8((address), (value))
#define ReadOperationTimer(address) FirmwareRead8(address)
#define ReadOperationInputBit2() ((u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_11ED) & 0x04u) != 0u))
#define RestartOperationCoroutine(entry) RestartCoroutine1AndYield(entry)
#define OperationAttemptLimit() 200000ul
#endif
#endif
