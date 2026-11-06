#include "write_controller_range_sequence.h"
#include "../media/raw_read_frame_prefix_wait.h"
#include "../media/write_preservation_read.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"
#include "write_controller_range_build.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/write_controller_range_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRangeSequenceU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRangeSequenceU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 RestartWriteRangeRawReadPipeline(u8 clear_active_state)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 0u);
    if(clear_active_state != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    (void)RANGE_PREPARE(0u, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9F) == 2u)
        {
            RANGE_FAILURE(1u);
            return 0u;
        }
        RANGE_YIELD();
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    return 1u;
}

/* RAM:fab049 */
u8 ExecuteWriteControllerBufferRangeSequence(void)
{
    u8 record_count;
    u32 block_count;
    s32 start_frame_position;

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CE, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CD, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CC, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CF, 0u);
    block_count = (u32)FirmwareRead8(FW_HUGE_UNKNOWN_2BA5C6);
    start_frame_position = (s32)ReadRangeSequenceU32(FW_HUGE_UNKNOWN_2BA5B4);
    record_count = (u8)RANGE_BUILD(start_frame_position, block_count);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CC, record_count);
    if(FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CC) == 0u)
        return 1u;

    if(FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CB) != 1u && RestartWriteRangeRawReadPipeline(0u) == 0u)
        return 0u;

    for(;;)
    {
        u8 start_index;
        u8 end_index;
        u32 saved_command_start;
        u32 range_start;

        start_index = FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CD);
        record_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CC);
        if(start_index >= record_count && FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CF) != 1u)
            break;

        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CF, 0u);
        record_count = FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CC);
        start_index = FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CD);
        end_index = RANGE_SELECT(start_index, record_count);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CE, end_index);
        saved_command_start = ReadRangeSequenceU32(FW_NEAR_UNKNOWN_0B02);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x10u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA1, 1u);
        RANGE_INITIALIZE_TRACK();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA0, 0u);
        RANGE_FILL(0x0000cca0ul, 0ul, 0u);
        range_start = ReadRangeSequenceU32(FW_HUGE_UNKNOWN_2BA5BC);
        (void)RANGE_ACQUIRE((s32)range_start, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15D7, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0CA1, 0u);
        WriteRangeSequenceU32(FW_NEAR_UNKNOWN_0B02, saved_command_start);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C8, 1u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CA, 1u);
        WriteRangeSequenceU32(FW_NEAR_UNKNOWN_1598, ReadRangeSequenceU32(FW_HUGE_UNKNOWN_2BA5BC));
        RANGE_WAIT();
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5C8, 0u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CA, 0u);
        if(RestartWriteRangeRawReadPipeline(1u) == 0u)
            return 0u;

        start_index = (u8)(FirmwareRead8(FW_HUGE_UNKNOWN_2BA5CE) + 1u);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CE, start_index);
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5CD, start_index);
    }
    return 1u;
}
