#include "current_mode_raw_read_pipeline.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "../platform/track_state.h"
#include "../scsi/mmc_write.h"
#include "raw_read_acquisition_plan.h"
#include "raw_read_frame_sequence.h"
#include "raw_read_track_record_advance.h"
#include "raw_read_transfer_layout.h"
#include "raw_track_selection.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/current_mode_raw_read_pipeline_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCurrentModeU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadCurrentModeU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteCurrentModeU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteCurrentModeU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void InitializeCurrentModeCommonState(void)
{
    CURRENT_MODE_WRITE_BIT41(0u);
    CURRENT_MODE_WRITE_BIT42((u8)((FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0) & 1u) == 0u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1284, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B4, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1286, 0u);
    WriteCurrentModeU16(FW_NEAR_UNKNOWN_800E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    CURRENT_MODE_INIT_CONTROL();
    CURRENT_MODE_INIT_IDENTIFICATION();
    CURRENT_MODE_INIT_TRACKS();
    CURRENT_MODE_CLEAR_STATUS();
    WriteCurrentModeU32(FW_NEAR_UNKNOWN_1488, 0u);
    WriteCurrentModeU32(FW_NEAR_UNKNOWN_148C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0x20u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C71));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C72));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C73));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C74));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) == 0u)
        CURRENT_MODE_INIT_RANDOM();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1590, 1u);
}

static u8 InitializeCurrentModeZero(void)
{
    CURRENT_MODE_UPDATE_Q_TYPE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1288, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x20u) == 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 0u);
    if(CURRENT_MODE_IS_ISRC_AVAILABLE() == 1u)
        CURRENT_MODE_LOAD_ISRC();
    if(CURRENT_MODE_PREPARE(0u) == 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B4) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
    {
        CURRENT_MODE_WRITE_BIT30(0u);
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
        {
            if(CURRENT_MODE_POLL(1u) == 0u)
            {
                if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) == 1u)
                {
                    CURRENT_MODE_FAILURE(8u);
                }
                else
                {
                    if(ReadCurrentModeU16(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
                        CURRENT_MODE_QUEUE_SENSE();
                    else
                        CURRENT_MODE_FAILURE(0u);
                    return 0u;
                }
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B7, 1u);
            if(CURRENT_MODE_PREPARE(0u) == 0u)
                return 0u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B7, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 0u);
    }
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B2, 0u);
        WriteCurrentModeU32(FW_NEAR_UNKNOWN_14B8, 0u);
    }
    CURRENT_MODE_CONFIGURE_LAYOUT();
    return 1u;
}

static u8 InitializeCurrentModeOne(void)
{
    CURRENT_MODE_UPDATE_Q_TYPE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_146F, 0u);
    if(CURRENT_MODE_IS_MCN_AVAILABLE() == 1u)
        CURRENT_MODE_LOAD_MCN();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1288, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x20u) == 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    if(CURRENT_MODE_IS_ISRC_AVAILABLE() == 1u)
        CURRENT_MODE_LOAD_ISRC();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 0u);
    if(CURRENT_MODE_PREPARE(0u) == 0u)
        return 0u;
    CURRENT_MODE_CONFIGURE_LAYOUT();
    return 1u;
}

static u8 InitializeCurrentModeTwo(void)
{
    CURRENT_MODE_UPDATE_Q_TYPE();
    CURRENT_MODE_INIT_TRACK_RECORDS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1288, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_B4BD) & 0x20u) == 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1404) == 0u)
    {
        if(CURRENT_MODE_ACQUIRE_LEAD_IN() == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1404, 1u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1405, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1403, 0u);
    CURRENT_MODE_ADVANCE_TRACK();
    CURRENT_MODE_REFRESH_ISRC();
    return 1u;
}

static u8 InitializeCurrentModeThree(void)
{
    fw_addr_t ring_address;
    fw_addr_t buffer_address;
    u8 q_type;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/current_mode_raw_read_pipeline_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    CURRENT_MODE_UPDATE_Q_TYPE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    CURRENT_MODE_COPY(FW_NEAR_UNKNOWN_1428, tuple_address, 4u);
    CURRENT_MODE_DECREMENT_MSF((u16)tuple_address);
    WriteCurrentModeU32(FW_NEAR_UNKNOWN_1598, ReadCurrentModeU32(FW_NEAR_UNKNOWN_1598) - 1ul);
    ring_address = CURRENT_MODE_RESUME_RING();
    if(CURRENT_MODE_ACQUIRE_FRAMES(1ul, 0u) == 0u)
        return 0u;

    q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1285);
    if(q_type == 1u)
    {
        u8 index;
        buffer_address = CURRENT_MODE_SELECT_BUFFER(0x10ul, ring_address);
        buffer_address += FW_ADDRESS_BANK_STRIDE;
        for(index = 0u; index < 16u; ++index)
            FirmwareWrite8(buffer_address + index, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 1u);
    }
    else if(q_type == 2u || q_type == 3u)
    {
        buffer_address = CURRENT_MODE_SELECT_BUFFER(0x60ul, ring_address);
        CURRENT_MODE_FILL(buffer_address, 0x60ul, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 2u);
    }
    else if(q_type == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127F, 0u);
    }
    CURRENT_MODE_CONFIGURE_LAYOUT();
    return 1u;
}

/* RAM:fe59b4 */
u8 PLEXTOR_NEAR_FUNCTION InitializeRawReadPipelineForCurrentMode(void)
{
    u8 mode;

    InitializeCurrentModeCommonState();
    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_1283);
    if(mode == 0u)
        return InitializeCurrentModeZero();
    if(mode == 2u)
        return InitializeCurrentModeTwo();
    if(mode == 3u)
        return InitializeCurrentModeThree();
    if(mode == 1u)
        return InitializeCurrentModeOne();
    return 1u;
}
