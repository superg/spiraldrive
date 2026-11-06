#include "firmware_programmer_handoff.h"
#include "firmware_memory.h"
#include "leaf_workers.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/firmware_programmer_handoff_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void RestoreFirmwareInterruptSources(void)
{
    SET_GLOBAL_INTERRUPT(1u);
    SET_CONTROLLER_INTERRUPT(1u);
    SET_A9_BIT2();
}

static void WriteRuntimeProgrammerEntry(fw_addr_t entry)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/firmware_programmer_handoff_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:fd6b13 */
u8 PLEXTOR_NEAR_FUNCTION ValidateAndInvokeRuntimeMainFirmwareProgrammer(void)
{
    if(VALIDATE_REVISION(0u) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_HOST_COMPLETION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0xdfu));
    SET_GLOBAL_INTERRUPT(0u);
    if(VALIDATE_HEADER() == 0u)
    {
        RestoreFirmwareInterruptSources();
        return 0u;
    }
    if(VALIDATE_CHECKSUM(0ul, 0x00079ffeul) == 0u)
    {
        RestoreFirmwareInterruptSources();
        return 0u;
    }

    RestoreFirmwareInterruptSources();
    SERVICE_PENDING_HOST_COMMAND();
    SET_GLOBAL_INTERRUPT(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_00FA, 0xffu);
    WriteRuntimeProgrammerEntry(PLEXTOR_RUNTIME_MONITOR_ADDRESS(FW_ROM_UNKNOWN_FF4000));
    INVOKE_RUNTIME_PROGRAMMER();
    return 1u;
}
