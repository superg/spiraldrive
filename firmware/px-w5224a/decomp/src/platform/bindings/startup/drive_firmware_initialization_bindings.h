#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveDriveFirmwareInitialization(u8 event, u32 argument0, u32 argument1, u32 argument2);

#define DRIVE_INITIALIZATION_CALL(event, argument0, argument1, argument2) ObserveDriveFirmwareInitialization((event), (argument0), (argument1), (argument2))
#define SELECT_MAIN_INTERRUPTS() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_SELECT_MAIN_INTERRUPTS, 0ul, 0ul, 0ul))
#define INITIALIZE_CONTROLLER_ASIC() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_INITIALIZE_CONTROLLER_ASIC, 0ul, 0ul, 0ul))
#define RUN_MEMORY_PROBE_20_36() DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_RUN_MEMORY_PROBE_20_36, 0x20ul, 0x36ul, 0ul)
#define RUN_MEMORY_PROBE_300_540() DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_RUN_MEMORY_PROBE_300_540, 0x300ul, 0x540ul, 0ul)
#define VALIDATE_BOOT_MEMORY() DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_VALIDATE_BOOT_MEMORY, 0ul, 0ul, 0ul)
#define ZERO_MEMORY(destination, count) ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_ZERO_MEMORY, (destination), (count), 0ul))
#define FILL_CONTROLLER_BUFFER(address, count, value) ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_FILL_CONTROLLER_BUFFER, (address), (count), (value)))
#define INITIALIZE_DEFAULT_PROFILE() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_INITIALIZE_DEFAULT_PROFILE, 0ul, 0ul, 0ul))
#define LOAD_FIXED_TABLES() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_LOAD_FIXED_TABLES, 0ul, 0ul, 0ul))
#define INITIALIZE_PICKUP_PARAMETERS() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_INITIALIZE_PICKUP_PARAMETERS, 0ul, 0ul, 0ul))
#define RESET_CONTROLLER_STATE() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_RESET_CONTROLLER_STATE, 0ul, 0ul, 0ul))
#define INITIALIZE_CONFIGURATION() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_INITIALIZE_CONFIGURATION, 0ul, 0ul, 0ul))
#define INVOKE_FLASH_IDENTIFICATION_LOADER() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_INVOKE_FLASH_IDENTIFICATION_LOADER, PLEXTOR_RUNTIME_MONITOR_ADDRESS(0x00ff5000ul), 0ul, 0ul))
#define INITIALIZE_DISC_CONTROL() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_INITIALIZE_DISC_CONTROL, 0ul, 0ul, 0ul))
#define VALIDATE_PRIMARY_CONFIGURATION() DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_VALIDATE_PRIMARY_CONFIGURATION, 0ul, 0ul, 0ul)
#define TRY_ERASED_CONFIGURATION() DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_TRY_ERASED_CONFIGURATION, 0ul, 0ul, 0ul)
#define INITIALIZE_RUNTIME_STATE() ((void)DRIVE_INITIALIZATION_CALL(DRIVE_FIRMWARE_INITIALIZE_RUNTIME_STATE, 0ul, 0ul, 0ul))

static void SetP3Bits01(u8 bit0, u8 bit1)
{
    u8 value = FirmwareReadDirectSfr8(B0);

    value = (u8)(value & 0xfcu);
    value = (u8)(value | bit0 | (u8)(bit1 << 1));
    FirmwareWriteDirectSfr8(B0, value);
}

#define SET_P3_BITS_01(bit0, bit1) SetP3Bits01((bit0), (bit1))
#define SET_STARTUP_MEMORY_BIT() FirmwareWriteBit0032(1u)
#define READ_STARTUP_MEMORY_BIT() FirmwareReadBit0032()
#define SET_EX0(value) WriteSfrBit(0xa8u, 0x01u, (value))
#define SET_EC(value) WriteSfrBit(0xa8u, 0x40u, (value))
#define SET_EA(value) WriteSfrBit(0xa8u, 0x80u, (value))
#define SET_A9_BIT2() WriteSfrBit(0xa9u, 0x04u, 1u)
#define SET_TIMER_RUN_BIT() WriteSfrBit(0x88u, 0x10u, 1u)

static void WriteSfrBit(u8 address, u8 mask, u8 value)
{
    u8 current = FirmwareReadSfr8(address);

    if(value != 0u)
        current = (u8)(current | mask);
    else
        current = (u8)(current & (u8)~mask);
    FirmwareWriteSfr8(address, current);
}
#else
extern u8 RunStartupMemoryProbe20_36(void);
extern u8 RunStartupMemoryProbe300_540(void);
extern void InvokeRuntimeFlashIdentificationLoader(void);
sbit DriveInitializationP3Bit0 = 0xb0;
sbit DriveInitializationP3Bit1 = 0xb1;
u8 bdata DriveInitializationMemoryBitByte _at_ 0x26;
sbit DriveInitializationMemoryBit = DriveInitializationMemoryBitByte ^ 2;
sbit DriveInitializationExternalInterrupt0Enable = 0xa8;
sbit DriveInitializationControllerInterruptEnable = 0xae;
sbit DriveInitializationGlobalInterruptEnable = 0xaf;
extern void SetSfrA9Bit2Atomic(void);
sbit DriveInitializationTimerRunBit = 0x8c;

#define SELECT_MAIN_INTERRUPTS() SelectMainFirmwareInterruptHandlers()
#define INITIALIZE_CONTROLLER_ASIC() InitializeControllerAsic()
#define RUN_MEMORY_PROBE_20_36() RunStartupMemoryProbe20_36()
#define RUN_MEMORY_PROBE_300_540() RunStartupMemoryProbe300_540()
#define VALIDATE_BOOT_MEMORY() ValidateBootAndConfigurationMemory()
#define ZERO_MEMORY(destination, count) ZeroMemoryWords((destination), (count))
#define FILL_CONTROLLER_BUFFER(address, count, value) FillControllerBufferRange((address), (count), (value))
#define INITIALIZE_DEFAULT_PROFILE() InitializeDefaultControllerOperatingProfile()
#define LOAD_FIXED_TABLES() LoadFixedControllerParameterTables()
#define INITIALIZE_PICKUP_PARAMETERS() InitializePickupSeekControlParameters()
#define RESET_CONTROLLER_STATE() ResetControllerOperatingState()
#define INITIALIZE_CONFIGURATION() InitializeDriveConfigurationState()
#define INVOKE_FLASH_IDENTIFICATION_LOADER() InvokeRuntimeFlashIdentificationLoader()
#define INITIALIZE_DISC_CONTROL() InitializeDiscControlStateFromConfiguration()
#define VALIDATE_PRIMARY_CONFIGURATION() ValidatePrimaryDriveConfigurationChecksum()
#define TRY_ERASED_CONFIGURATION() TryInitializeErasedDriveConfiguration()
#define INITIALIZE_RUNTIME_STATE() InitializeRuntimeDriveState()
#define SET_P3_BITS_01(bit0, bit1)                                                                                                                                                                     \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        DriveInitializationP3Bit0 = (bit0);                                                                                                                                                            \
        DriveInitializationP3Bit1 = (bit1);                                                                                                                                                            \
    } while(0)
#define SET_STARTUP_MEMORY_BIT() (DriveInitializationMemoryBit = 1)
#define READ_STARTUP_MEMORY_BIT() DriveInitializationMemoryBit
#define SET_EX0(value) (DriveInitializationExternalInterrupt0Enable = (value))
#define SET_EC(value) (DriveInitializationControllerInterruptEnable = (value))
#define SET_EA(value) (DriveInitializationGlobalInterruptEnable = (value))
#define SET_A9_BIT2() SetSfrA9Bit2Atomic()
#define SET_TIMER_RUN_BIT() (DriveInitializationTimerRunBit = 1)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
FirmwareWriteNear8(address, (u8)(value >> 24));
FirmwareWriteNear8((u16)(address + 1u), (u8)(value >> 16));
FirmwareWriteNear8((u16)(address + 2u), (u8)(value >> 8));
FirmwareWriteNear8((u16)(address + 3u), (u8)value);
#else
*((volatile u32 near *)address) = value;
#endif
#endif
