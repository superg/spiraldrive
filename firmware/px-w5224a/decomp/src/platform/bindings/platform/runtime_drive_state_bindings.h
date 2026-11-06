#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRuntimeDriveStateReadConfiguration(fw_addr_t offset, fw_addr_t destination, u16 count);
extern void ObserveRuntimeDriveStateInitializeControllerAsic(void);
extern void ObserveRuntimeDriveStateConfigureStartupMode(u8 run_handshake);
extern void ObserveRuntimeDriveStateInitializeInquiry(void);
extern void ObserveRuntimeDriveStateInitializeModePages(void);
extern void ObserveRuntimeDriveStateRefreshConfigurationFlag(void);
extern void ObserveRuntimeDriveStateInitializeReadRuntime(void);
extern void ObserveRuntimeDriveStateClearWriteBufferSnapshot(void);
extern void ObserveRuntimeDriveStateClearBit0547(void);
extern void ObserveRuntimeDriveStateSetBit0547(void);
extern void ObserveRuntimeDriveStateSetBit0546(void);
extern void ObserveRuntimeDriveStateSetSfrA9Bit2(void);
extern void ObserveRuntimeDriveStateSetBit000B(void);
#define READ_CONFIGURATION(offset, destination, count) ObserveRuntimeDriveStateReadConfiguration((offset), (destination), (count))
#define INITIALIZE_CONTROLLER_ASIC() ObserveRuntimeDriveStateInitializeControllerAsic()
#define CONFIGURE_STARTUP_MODE(value) ObserveRuntimeDriveStateConfigureStartupMode(value)
#define INITIALIZE_INQUIRY() ObserveRuntimeDriveStateInitializeInquiry()
#define INITIALIZE_MODE_PAGES() ObserveRuntimeDriveStateInitializeModePages()
#define REFRESH_CONFIGURATION_FLAG() ObserveRuntimeDriveStateRefreshConfigurationFlag()
#define INITIALIZE_READ_RUNTIME() ObserveRuntimeDriveStateInitializeReadRuntime()
#define CLEAR_WRITE_BUFFER_SNAPSHOT() ObserveRuntimeDriveStateClearWriteBufferSnapshot()
#define CLEAR_BIT_0547() ObserveRuntimeDriveStateClearBit0547()
#define SET_BIT_0547() ObserveRuntimeDriveStateSetBit0547()
#define SET_BIT_0546() ObserveRuntimeDriveStateSetBit0546()
#define SET_SFR_A9_BIT2() ObserveRuntimeDriveStateSetSfrA9Bit2()
#define SET_BIT_000B() ObserveRuntimeDriveStateSetBit000B()
#define READ_P0_BIT(bit) ((u8)((FirmwareReadDirectSfr8(80) >> (bit)) & 1u))
#else
#include "boot/image.h"
#include "scsi/mode_page_initialization.h"
extern u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit0ForRuntimeDriveState(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit1ForRuntimeDriveState(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit2ForRuntimeDriveState(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit5ForRuntimeDriveState(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadP0Bit6ForRuntimeDriveState(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit000BForRuntimeDriveStateAtomic(void);
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
#define READ_CONFIGURATION(offset, destination, count) ReadDriveConfigurationBytes((offset), (destination), (count))
#define INITIALIZE_CONTROLLER_ASIC() InitializeControllerAsic()
#define CONFIGURE_STARTUP_MODE(value) ConfigureControllerStartupModeRegisters(value)
#define INITIALIZE_INQUIRY() InitializeInquiryResponseBuffer()
#define INITIALIZE_MODE_PAGES() InitializeModePageStateFromFirmwareDefaults()
#define REFRESH_CONFIGURATION_FLAG() RefreshConfigurationByte45Flag()
#define INITIALIZE_READ_RUNTIME() InitializeReadRuntimeState()
#define CLEAR_WRITE_BUFFER_SNAPSHOT() ClearWriteBufferSnapshotValid()
#define CLEAR_BIT_0547() ClearBit0547Atomic()
#define SET_BIT_0547() SetBit0547Atomic()
#define SET_BIT_0546() SetBit0546Atomic()
#define SET_SFR_A9_BIT2() SetSfrA9Bit2Atomic()
#define SET_BIT_000B() SetBit000BForRuntimeDriveStateAtomic()
static u8 ReadRuntimeDriveStateP0Bit(u8 bit_index)
{
    switch(bit_index)
    {
    case 0u:
        return ReadP0Bit0ForRuntimeDriveState();
    case 1u:
        return ReadP0Bit1ForRuntimeDriveState();
    case 2u:
        return ReadP0Bit2ForRuntimeDriveState();
    case 5u:
        return ReadP0Bit5ForRuntimeDriveState();
    default:
        return ReadP0Bit6ForRuntimeDriveState();
    }
}
#define READ_P0_BIT(bit) ReadRuntimeDriveStateP0Bit(bit)
#endif
#endif
