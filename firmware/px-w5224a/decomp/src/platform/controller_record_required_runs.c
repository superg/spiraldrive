#include "controller_record_required_runs.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_required_runs_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRequiredRunsU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadRequiredRunsU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static u16 RequiredRunsBoundaryIndex(u16 segment)
{
    u32 position = ReadRequiredRunsU32(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84490) + (fw_addr_t)(segment * 4u));

    return (u16)REQUIRED_DIVIDE(position - 145ul, 39ul);
}

/* RAM:fa1871 */
u8 AcquireRequiredControllerRecordIndexRuns(u8 operation_mode)
{
    u16 last_index;

    FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH, 0u);
    last_index = (u16)(ReadRequiredRunsU16(FW_HUGE_UNKNOWN_2BA5D9) - 1u);
    if(operation_mode == 3u)
    {
        u8 segment_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA588);
        u16 boundary_index;

        FirmwareWrite8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH, 1u);
        boundary_index = RequiredRunsBoundaryIndex((u16)(segment_count - 1u));
        if(ReadRequiredRunsU16(FW_HUGE_UNKNOWN_2BA5DB) < boundary_index)
        {
            u16 segment = (u16)(segment_count - 2u);

            for(;;)
            {
                boundary_index = RequiredRunsBoundaryIndex(segment);
                if(boundary_index <= ReadRequiredRunsU16(FW_HUGE_UNKNOWN_2BA5DB) || segment == 0u)
                    break;
                --segment;
            }
            last_index = (u16)(boundary_index + 0xafu);
        }
    }
    REQUIRED_ACQUIRE(last_index, operation_mode);

    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 4u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfbu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xbfu));
        REQUIRED_RECALCULATE();
        REQUIRED_BUILD_LEADIN(0u);
        REQUIRED_ACQUIRE_BASE();
        return 0u;
    }
    if(FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfbu));
        FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 4u));
        FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 0u);
        return 0u;
    }

    FirmwareWrite8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xbfu));
    return 1u;
}
