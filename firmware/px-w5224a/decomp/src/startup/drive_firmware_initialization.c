#include "drive_firmware_initialization.h"
#include "../boot/image.h"
#include "../platform/controller_asic_initialization.h"
#include "../platform/controller_parameters.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/media_interrupt_leaves.h"
#include "../platform/memory.h"
#include "../platform/runtime_drive_state.h"
#include "disc_control_state_initialization.h"
#include "drive_configuration_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/drive_firmware_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void WriteBigEndianU32Near(u16 address, u32 value)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/startup/drive_firmware_initialization_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:fdc43b */
void InitializeDriveFirmware(void)
{
    u8 startup_memory_probe_valid;
    u8 boot_memory_valid;

    SELECT_MAIN_INTERRUPTS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 0u);
    SET_P3_BITS_01((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FA) == 0xffu), 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
    FirmwareWriteDirectSfr8(85, 0u);
    FirmwareWriteDirectSfr8(86, 0x1bu);
    INITIALIZE_CONTROLLER_ASIC();

    (void)RUN_MEMORY_PROBE_20_36();
    startup_memory_probe_valid = RUN_MEMORY_PROBE_300_540();
    boot_memory_valid = VALIDATE_BOOT_MEMORY();

    SET_STARTUP_MEMORY_BIT();
    ZERO_MEMORY(FW_NEAR_UNKNOWN_0020, 0x0060u);
    if(READ_STARTUP_MEMORY_BIT() != 0u)
        startup_memory_probe_valid = 0u;
    ZERO_MEMORY(FW_NEAR_UNKNOWN_0B00, 0x0d00u);
    ZERO_MEMORY(FW_NEAR_UNKNOWN_8000, 0x8000u);

    SET_EX0(1u);
    SET_EA(1u);
    FILL_CONTROLLER_BUFFER(FW_HUGE_UNKNOWN_1B0000, 0x00010000ul, 0u);
    SET_EA(0u);
    SET_EX0(0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_00FA) == 0xffu)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) | 0x80u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FA, 0u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12F7, 0u);
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_12F8, 0ul);

    INITIALIZE_DEFAULT_PROFILE();
    LOAD_FIXED_TABLES();
    INITIALIZE_PICKUP_PARAMETERS();
    RESET_CONTROLLER_STATE();
    INITIALIZE_CONFIGURATION();
    WriteBigEndianU32Near(FW_NEAR_UNKNOWN_130E, PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FF5000));
    INVOKE_FLASH_IDENTIFICATION_LOADER();
    INITIALIZE_DISC_CONTROL();

    SET_TIMER_RUN_BIT();
    FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) & 0xf5u));
    FirmwareWriteDirectSfr8(91, (u8)(FirmwareReadDirectSfr8(91) & 0xf5u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0u);

    if(startup_memory_probe_valid == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
        return;
    }
    if(boot_memory_valid == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 2u);
        return;
    }
    if(VALIDATE_PRIMARY_CONFIGURATION() == 0u && TRY_ERASED_CONFIGURATION() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 10u);
        return;
    }

    FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) & 0xf5u));
    FirmwareWriteDirectSfr8(91, (u8)(FirmwareReadDirectSfr8(91) & 0xf5u));
    SET_EX0(1u);
    SET_EA(1u);
    SET_EC(1u);
    SET_A9_BIT2();
    FILL_CONTROLLER_BUFFER(0ul, FW_HUGE_UNKNOWN_1B0000, 0u);
    SET_EA(1u);
    SET_EC(1u);
    SET_A9_BIT2();
    INITIALIZE_RUNTIME_STATE();
}
