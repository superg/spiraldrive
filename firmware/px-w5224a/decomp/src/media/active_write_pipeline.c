#include "active_write_pipeline.h"
#include "fixed_packet_padding.h"
#include "platform/firmware_memory.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#include "platform/track_state.h"
#include "raw_read_frame_prefix_wait.h"
#include "raw_read_runtime.h"
#include "scsi/send_opc_information.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/active_write_pipeline_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadActiveWriteU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteActiveWriteU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 BuildRawQWithWriteSpeedFallback(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 10u)
        return ACTIVE_BUILD_RAW_Q();

    if(ACTIVE_BUILD_RAW_Q() != 0u)
        return 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
    {
        if(ACTIVE_RUN_OPC() == 0u)
        {
            ACTIVE_SET_SENSE(3u, 0x7304u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
            return 0u;
        }
    }
    if(ACTIVE_BUILD_RAW_Q() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    return 1u;
}

/* RAM:fb4471 */
u8 FinalizeActiveWritePipelineAndTrackState(void)
{
    u8 write_type;
    u32 cursor;
    u32 end_frame;

    WriteActiveWriteU32(FW_NEAR_UNKNOWN_1455, 0ul);
    if(ACTIVE_READ_BIT35() != 0u)
    {
        ACTIVE_CLEAR_BIT35();
        ACTIVE_CLEAR_BIT30();
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
            while(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) != 0u)
                ACTIVE_YIELD();
        if(ACTIVE_READ_BIT32() != 0u)
            ACTIVE_RESTART_PRODUCER();
    }

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    cursor = ReadActiveWriteU32(FW_NEAR_UNKNOWN_DD4D);
    end_frame = ReadActiveWriteU32(FW_NEAR_UNKNOWN_DD41);
    if(write_type == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD61) == 2u && (s32)(cursor + 2ul) < (s32)end_frame)
    {
        if(ACTIVE_ACQUIRE_TAIL((s32)((end_frame - 2ul) - cursor)) == 0u)
        {
            ACTIVE_UPDATE_FAILURE();
            return 0u;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 0u);
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) != 0u && write_type == 0u)
        {
            if(ReadActiveWriteU32(FW_NEAR_UNKNOWN_14B8) == 0ul)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);
            else if(ACTIVE_ACQUIRE_PADDING() == 0u)
                return 0u;
        }
        if(ACTIVE_PREPARE_PREFIX(0u, 1u) == 0u)
        {
            ACTIVE_UPDATE_FAILURE();
            return 0u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);
    }
    else
    {
        if(ACTIVE_EXECUTE_RANGE(ReadActiveWriteU32(FW_NEAR_UNKNOWN_0B06)) == 0u)
            return 0u;
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0x20u) != 0u)
        {
            ACTIVE_VERIFY_READBACK();
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xdfu) | 0x40u));
        }
    }

    if(write_type == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD61) == 0u)
        {
            WriteActiveWriteU32(FW_NEAR_UNKNOWN_DD41, ReadActiveWriteU32(FW_NEAR_UNKNOWN_DD4D) + 2ul);
            ACTIVE_CLEAR_CONTROLLER_WORDS();
            ACTIVE_BUILD_ADR1();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E74C) != 0u && BuildRawQWithWriteSpeedFallback() == 0u)
                return 0u;
            ACTIVE_CLEAR_CONTROLLER_WORDS();
        }
    }
    else if(write_type == 2u)
    {
        if(BuildRawQWithWriteSpeedFallback() == 0u)
            return 0u;
    }
    else if(write_type == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
        {
            cursor = ReadActiveWriteU32(FW_NEAR_UNKNOWN_DD4D) + 2ul;
            WriteActiveWriteU32(FW_NEAR_UNKNOWN_DD4D, cursor);
            WriteActiveWriteU32(FW_NEAR_UNKNOWN_DD45, cursor);
        }
        else
        {
            ACTIVE_ADVANCE_PACKET();
        }
        WriteActiveWriteU32(FW_NEAR_UNKNOWN_DD5B, 0ul);
    }

    if(write_type == 1u)
        ACTIVE_FINALIZE_TYPE1();
    else if(write_type == 2u)
        ACTIVE_FINALIZE_TYPE2();
    else if(write_type == 3u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
            ACTIVE_INITIALIZE_COROUTINE1(FW_ENTRY_RUN_STARTUP_SEQUENCE_COROUTINE1);
    }
    else if(write_type == 0u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
    {
        ACTIVE_FINALIZE_OPCODE(0x35u);
    }
    return 1u;
}
