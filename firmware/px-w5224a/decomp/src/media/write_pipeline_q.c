#include "write_pipeline_q.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/write_pipeline_q_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static s32 ReadWritePipelineTargetAddress(void)
{
    return (s32)FirmwareReadNearU32(FW_NEAR_UNKNOWN_1598);
}

static u32 ReadBigEndianU32ForWritePipeline(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteBigEndianU32ForWritePipeline(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void EncodePackedBcdMsf(u8 output[3], s32 frame_position)
{
    u32 position;
    u32 total_seconds;
    u16 frames;
    u16 seconds;

    if(frame_position < 0)
        frame_position += 450000l;
    position = (u32)frame_position;
    frames = (u16)(position % 75ul);
    total_seconds = position / 75ul;
    seconds = (u16)(total_seconds % 60ul);
    output[0] = BinaryByteToPackedBcd((u8)(total_seconds / 60ul));
    output[1] = BinaryByteToPackedBcd((u8)seconds);
    output[2] = BinaryByteToPackedBcd((u8)frames);
}

static void WritePackedBcdMsf(fw_addr_t destination, const u8 value[3])
{
    FirmwareWrite8(destination, value[0]);
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 1u), value[1]);
    FirmwareWrite8(FirmwareAddressIncrementLow16(destination, 2u), value[2]);
}

/* RAM:fdea32 */
void BuildWritePipelineQPacketForTargetAddress(void)
{
    s32 target = ReadWritePipelineTargetAddress();
    s32 relative_position;
    u8 tuple[3];

    if(target < 150l)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 0u);
        relative_position = 4l;
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
        relative_position = target - 150l;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, 0x71u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 1u);
    EncodePackedBcdMsf(tuple, relative_position);
    WritePackedBcdMsf(FW_NEAR_UNKNOWN_1275, tuple);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
    EncodePackedBcdMsf(tuple, target);
    WritePackedBcdMsf(FW_NEAR_UNKNOWN_1279, tuple);
}

/* RAM:fbd412 */
void PLEXTOR_NEAR_FUNCTION AdvanceWorkingTrackWriteCursorByCallerFlag(u32 frame_delta, u8 caller_flag)
{
    fw_addr_t base_address;

    if(caller_flag == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) <= 3u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u))
        base_address = FW_NEAR_UNKNOWN_DD4D;
    else
        base_address = FW_NEAR_UNKNOWN_DD3D;

    WriteBigEndianU32ForWritePipeline(FW_NEAR_UNKNOWN_DD4D, ReadBigEndianU32ForWritePipeline(base_address) + frame_delta);
}

/* RAM:feca71 */
u8 InitializeWriteCommandRawReadPipelineState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1284, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B4, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1286, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_800E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_800F, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);

    INIT_SECTOR_CONTROL();
    INIT_SUBCHANNEL_IDENTIFICATION();
    INIT_TRACK_SELECTION();
    CLEAR_PIPELINE_STATUS();
    WriteBigEndianU32ForWritePipeline(FW_NEAR_UNKNOWN_1488, 0u);
    WriteBigEndianU32ForWritePipeline(FW_NEAR_UNKNOWN_148C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0x20u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C71));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C72));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C73));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C74));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1590, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1280, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1288, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x20u) == 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 0u);
    if(IS_ISRC_AVAILABLE() == 1u)
        LOAD_ISRC_CHARACTERS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 0x0au);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B2, 0u);
    WriteBigEndianU32ForWritePipeline(FW_NEAR_UNKNOWN_14B8, 0u);
    INIT_WRITE_TRANSFER_LAYOUT();
    return 1u;
}
