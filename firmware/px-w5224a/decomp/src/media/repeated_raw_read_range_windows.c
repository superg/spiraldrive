#include "repeated_raw_read_range_windows.h"
#include "../platform/arithmetic.h"
#include "../platform/coroutine_initialization.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_read_opc.h"
#include "raw_read_frame_prefix_wait.h"
#include "raw_read_q_record_queue.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/repeated_raw_read_range_windows_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRepeatedU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadRepeatedU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRepeatedU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 FailRepeatedRangeOpc(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 8u);
    if(((ReadRepeatedU16(FW_NEAR_UNKNOWN_FAD7) >> 3) & 1u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 7u);
    return 0u;
}

/* RAM:febbd1 */
u8 PLEXTOR_NEAR_FUNCTION AcquireRepeatedRawReadRangeWindows(u32 frame_spacing)
{
    u8 last_track;
    u16 iteration;
    u16 iteration_limit;
    s32 current;
    s32 next;
    s32 end;

    while(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 3u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 2) & 1u) != 1u)
        REPEATED_YIELD();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C48, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD31, 0x41u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD32, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, 0x41u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x11u)
            iteration_limit = 1000u;
        else
            iteration_limit = 5u;
    else
        iteration_limit = 1u;

    last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
    if(last_track == 0u)
    {
        current = 0l;
        next = 0l;
        WriteRepeatedU32(FW_NEAR_UNKNOWN_1598, 0ul);
    }
    else
    {
        last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
        if(last_track == 99u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 9u);
            REPEATED_INITIALIZE(FW_ENTRY_RUN_DRIVE_INITIALIZATION_COROUTINE1);
            return 0u;
        }
        last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
        current = REPEATED_CONVERT(FW_HUGE_UNKNOWN_2BB03E + (fw_addr_t)last_track * 10ul);
        next = current;
        WriteRepeatedU32(FW_NEAR_UNKNOWN_1598, (u32)current);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
    {
        current = 45000l;
        next = 45000l;
        WriteRepeatedU32(FW_NEAR_UNKNOWN_1598, 45000ul);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x11u)
    {
        current = 750l;
        next = 750l;
        WriteRepeatedU32(FW_NEAR_UNKNOWN_1598, 750ul);
    }

    for(iteration = 0u; (s16)iteration < (s16)iteration_limit; ++iteration)
    {
        current = next;
        WriteRepeatedU32(FW_NEAR_UNKNOWN_1598, (u32)current);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x11u)
            end = (s32)(ReadRepeatedU32(FW_NEAR_CURRENT_FRAME_POSITION) - 2ul);
        else
            end = (s32)((u32)current + REPEATED_MULTIPLY(75ul, frame_spacing) - 2ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C48, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 8u);
        WriteRepeatedU32(FW_NEAR_UNKNOWN_DD3D, (u32)current);
        WriteRepeatedU32(FW_NEAR_UNKNOWN_8000, (u32)end);
        if(REPEATED_PREPARE(0u, current, end) == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
        if(REPEATED_PREFIX(0u, 1u) == 0u)
            return 0u;
        next = (s32)ReadRepeatedU32(FW_NEAR_UNKNOWN_1598);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
            break;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x11u && REPEATED_REMAINDER(iteration) == 19u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74E, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E74F, 1u);
            if(REPEATED_OPC() == 0u)
                return FailRepeatedRangeOpc();
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(REPEATED_BUILD_Q() == 0u)
            return 0u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 6u)
    {
        if(REPEATED_BUILD_Q() == 0u)
            return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xfbu));
    return 1u;
}
