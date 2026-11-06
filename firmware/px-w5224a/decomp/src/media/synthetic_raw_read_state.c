#include "synthetic_raw_read_state.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "raw_read_frame_prefix_wait.h"

extern u8 InitializeAndBuildRawReadQRecordQueue(void);
extern void BuildSyntheticTrackTableAndAcquireLeadInLeadOutSequences(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/synthetic_raw_read_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBigEndianU16At(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadBigEndianU32At(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU16At(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteBigEndianU32At(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:febb0c */
u8 PLEXTOR_NEAR_FUNCTION PrepareSyntheticRawReadOperationState(void)
{
    u32 end_frame;
    u16 write_speed_flags;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C44, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD31, 0x41u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD32, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, 0x41u);

    end_frame = ReadBigEndianU32At(FW_NEAR_CURRENT_FRAME_POSITION);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 4u)
        end_frame += 1ul;
    else
        end_frame -= 2ul;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C48, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 8u);
    WriteBigEndianU32At(FW_NEAR_UNKNOWN_8000, 150ul);
    WriteBigEndianU32At(FW_NEAR_UNKNOWN_DD3D, 150ul);
    WriteBigEndianU32At(FW_NEAR_UNKNOWN_8000, end_frame);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);

    if(PREPARE_SYNTHETIC_RANGE(1u, 150l, (s32)end_frame) == 0u)
        return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    if(PREPARE_SYNTHETIC_PREFIX(0u, 1u) == 0u)
        return 0u;

    CLEAR_SYNTHETIC_PIPELINE_BITS();
    if(BUILD_SYNTHETIC_RAW_Q_QUEUE() == 0u)
        return 0u;

    write_speed_flags = ReadBigEndianU16At(FW_NEAR_UNKNOWN_FAD7);
    WriteBigEndianU16At(FW_NEAR_UNKNOWN_FAD7, (u16)(write_speed_flags | 0x0040u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C48, 8u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 5u);
    BUILD_SYNTHETIC_TRACK_TABLE();
    return 1u;
}
