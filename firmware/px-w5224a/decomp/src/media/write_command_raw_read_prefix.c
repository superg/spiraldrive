#include "write_command_raw_read_prefix.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "raw_read_acquisition_plan.h"
#include "raw_read_frame_prefix_wait.h"
#include "raw_read_track_record_advance.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/write_command_raw_read_prefix_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fe5cf7 */
u8 PLEXTOR_NEAR_FUNCTION PrepareWriteCommandRawReadPrefix(void)
{
    u8 write_type;
    u8 original_track_type;
    u32 target;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1284, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1590, 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_127D) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9A) == 0u && DispatchWritePrefixRecoverPipeline() == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 1u);
    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type == 0u)
    {
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) == 1u)
            return 1u;
        target = ReadBigEndianU32(FW_NEAR_UNKNOWN_1598);
        if(target != (u32)(ReadBigEndianU32(FW_NEAR_UNKNOWN_DD4D) + (u32)7u))
            return 1u;

        WriteBigEndianU32(FW_NEAR_UNKNOWN_1598, (u32)(target - (u32)7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 0u);
        original_track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
        if(DispatchWritePrefixPrepareFrames(1u, 0u) == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 1u);
        if(DispatchWritePrefixPreparePlan(0u) == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, original_track_type);
    }
    else if(write_type == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) == 1u && ReadBigEndianU32(FW_NEAR_UNKNOWN_1598) == 0ul)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B5, 0u);
            DispatchWritePrefixAdvanceTrackRecord();
            DispatchWritePrefixRefreshIsrc();
        }
    }
    return 1u;
}
