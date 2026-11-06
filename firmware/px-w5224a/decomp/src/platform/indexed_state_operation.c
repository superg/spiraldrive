#include "indexed_state_operation.h"
#include "controller_parameters.h"
#include "controller_state_profile.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/indexed_state_operation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fd7f26 */
void ProcessIndexedStateOperation(void)
{
    u32 attempt;

    WriteOperationPort(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
    for(attempt = 0ul; attempt < OperationAttemptLimit(); ++attempt)
    {
        u8 cycle_index;

        WriteOperationPort(FW_CONTROLLER_UNKNOWN_01849D, 0u);
        ConfigureOperationDrive(4u, 1u);
        WriteOperationTimer(FW_NEAR_UNKNOWN_1208, 50u);
        while(ReadOperationInputBit2() == 0u)
        {
            if((s8)ReadOperationTimer(FW_NEAR_UNKNOWN_1208) < 0)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 9u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 9u);
                RestartOperationCoroutine(FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
            }
        }

        ConfigureOperationDrive(4u, 0u);
        WriteOperationTimer(FW_NEAR_UNKNOWN_1216, 5u);
        while((s8)ReadOperationTimer(FW_NEAR_UNKNOWN_1216) >= 0)
            ;

        WriteOperationPort(FW_CONTROLLER_UNKNOWN_01849D, 0u);
        for(cycle_index = 0u; cycle_index < 10u; ++cycle_index)
        {
            ApplyOperationProfile(0u, 0x0b55u);
            WriteOperationPort(FW_CONTROLLER_UNKNOWN_01849D, 0u);
            WriteOperationTimer(FW_NEAR_UNKNOWN_1216, 2u);
            while((s8)ReadOperationTimer(FW_NEAR_UNKNOWN_1216) > 0)
                ;

            ApplyOperationProfile(1u, 0x0b55u);
            WriteOperationPort(FW_CONTROLLER_UNKNOWN_01849D, 0u);
            WriteOperationTimer(FW_NEAR_UNKNOWN_1216, (u8)(cycle_index == 9u ? 30u : 2u));
            while((s8)ReadOperationTimer(FW_NEAR_UNKNOWN_1216) > 0)
                ;
        }
    }
}
