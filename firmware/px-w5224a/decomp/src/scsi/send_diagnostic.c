#include "send_diagnostic.h"
#include "../boot/image.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/send_diagnostic_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadSendDiagnosticU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* RAM:fb2915 */
void HandleSendDiagnosticCommand(void)
{
    u16 parameter_length;
    u8 self_test;

    if(DIAGNOSTIC_VALIDATE_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E05)) == 0u)
    {
        DIAGNOSTIC_REJECT();
        return;
    }

    parameter_length = ReadSendDiagnosticU16(FW_NEAR_UNKNOWN_0B0D);
    self_test = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x04u);
    if(self_test == 0x04u && parameter_length != 0u)
    {
        DIAGNOSTIC_REJECT();
        return;
    }
    if(self_test == 0u && parameter_length == 0u)
    {
        DIAGNOSTIC_SET_NO_SENSE();
        return;
    }

    DIAGNOSTIC_RESET_RAW_READ();
    DIAGNOSTIC_INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    DIAGNOSTIC_DISPATCH_REQUEST5();
    DIAGNOSTIC_YIELD_COROUTINE0();
    DIAGNOSTIC_ZERO_WORKSPACE(FW_NEAR_UNKNOWN_DCBD, 8u);

    if(self_test == 0x04u)
    {
        u8 diagnostic_failure = 0u;

        if(DIAGNOSTIC_VALIDATE_BOOT() == 0u)
            diagnostic_failure = 1u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 1u)
            diagnostic_failure = 2u;
        if(DIAGNOSTIC_TEST_CONTROLLER_BUFFER() == 0u)
            diagnostic_failure = 3u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
        if(diagnostic_failure != 0u)
        {
            DIAGNOSTIC_SET_SENSE(4u, 0x4000u);
            return;
        }
    }
    else
    {
        u8 boot_test_requested;
        u8 controller_test_requested;

        if(parameter_length != 8u)
        {
            DIAGNOSTIC_REJECT();
            return;
        }
        if(DIAGNOSTIC_DATA_OUT(FW_NEAR_UNKNOWN_0B2A, 8u) == 4u)
            return;

        boot_test_requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2C);
        controller_test_requested = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D);
        if(boot_test_requested != 0u && DIAGNOSTIC_VALIDATE_BOOT() != 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCBF, 1u);
        if(controller_test_requested != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCC0, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DCC1, 0u);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
    DIAGNOSTIC_SET_NO_SENSE();
}
