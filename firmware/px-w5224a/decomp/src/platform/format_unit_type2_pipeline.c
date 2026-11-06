#include "format_unit_type2_pipeline.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/format_unit_type2_pipeline_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadFormatUnitU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u16 ReadFormatUnitU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteFormatUnitU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteFormatUnitU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fa4263 */
void PrepareMediaWritePipelineState(void)
{
    u32 ring_slot_count;

    PREPARE_WRITE_NORMALIZE_SPEED();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    PREPARE_WRITE_RESET_QUEUE();
    PREPARE_WRITE_INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    PREPARE_WRITE_YIELD_COROUTINE0();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 2u));

    if(PREPARE_WRITE_ENSURE_OPC() != 0u)
    {
        PREPARE_WRITE_RESET_BUFFERS();
        WriteFormatUnitU16(FW_NEAR_UNKNOWN_1298, 0u);
        WriteFormatUnitU16(FW_NEAR_UNKNOWN_1294, 0u);
        WriteFormatUnitU16(FW_NEAR_UNKNOWN_1296, 0u);
        WriteFormatUnitU16(FW_NEAR_UNKNOWN_1292, 0u);

        ring_slot_count = ReadFormatUnitU32(FW_NEAR_UNKNOWN_156A);
        WriteFormatUnitU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
        ring_slot_count = ReadFormatUnitU32(FW_NEAR_UNKNOWN_156A);
        WriteFormatUnitU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
        WriteFormatUnitU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
        WriteFormatUnitU32(FW_NEAR_UNKNOWN_1262, 0ul);
        WriteFormatUnitU32(FW_NEAR_UNKNOWN_1266, 0ul);
        WriteFormatUnitU32(FW_NEAR_UNKNOWN_126A, ReadFormatUnitU32(FW_NEAR_UNKNOWN_1562));
        WriteFormatUnitU32(FW_NEAR_UNKNOWN_126E, 0ul);

        PREPARE_WRITE_CLEAR_CONTROLLER_REGIONS();
        PREPARE_WRITE_INITIALIZE_SECTOR_CONTROL();
        PREPARE_WRITE_CLEAR_STATUS();
        WriteFormatUnitU32(FW_NEAR_UNKNOWN_1488, 0ul);
        WriteFormatUnitU32(FW_NEAR_UNKNOWN_148C, 0ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 1u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 8u));
        PREPARE_WRITE_RECALCULATE_GEOMETRY();
        PREPARE_WRITE_ACQUIRE_RECORD_WINDOW();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 0u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfeu));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
}

/* RAM:fde4cf */
u8 PrepareFormatUnitCode11Type2ControllerRecordPipeline(void)
{
    u32 ring_slot_count;
    u16 index;

    RESET_RAW_READ_PIPELINE();
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_1292, 0u);

    ring_slot_count = ReadFormatUnitU32(FW_NEAR_UNKNOWN_156A);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    ring_slot_count = ReadFormatUnitU32(FW_NEAR_UNKNOWN_156A);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_126A, ReadFormatUnitU32(FW_NEAR_UNKNOWN_1562));
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_126E, 0ul);

    CLEAR_RAW_READ_CONTROLLER_REGIONS();
    INITIALIZE_RAW_READ_SECTOR_CONTROL();
    CLEAR_RAW_READ_PIPELINE_STATUS();
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_1488, 0ul);
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_148C, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWrite8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH, 0u);

    WriteFormatUnitU32(FW_NEAR_UNKNOWN_FAFF, 0x0000b4bbul);
    for(index = 0u; index < ReadFormatUnitU16(FW_HUGE_UNKNOWN_2BA5D9); ++index)
        FirmwareWrite8(FirmwareAddressIncrementLow16(ReadFormatUnitU32(FW_NEAR_UNKNOWN_FAFF), index), 0u);
    WriteFormatUnitU16(FW_HUGE_UNKNOWN_2BA5DB, 0u);
    WriteFormatUnitU32(FW_HUGE_UNKNOWN_2BAF37, 0ul);
    WriteFormatUnitU32(FW_HUGE_UNKNOWN_2BAF33, 0ul);
    WriteFormatUnitU32(FW_HUGE_UNKNOWN_2BA59F, 0x00000091ul);

    INITIALIZE_TYPE2_GEOMETRY();
    INITIALIZE_RECORD_BOUNDARY_DEFAULTS();

    WriteFormatUnitU32(FW_HUGE_UNKNOWN_2BA59F, 0ul);
    WriteFormatUnitU32(FW_HUGE_UNKNOWN_2BA5AB, 0ul);
    WriteFormatUnitU32(FW_HUGE_UNKNOWN_2BA5A7, 0ul);
    WriteFormatUnitU32(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION, 0ul);
    WriteFormatUnitU32(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY, 0ul);
    WriteFormatUnitU32(FW_HUGE_UNKNOWN_2BA5A3, 0ul);
    WriteFormatUnitU32(FW_NEAR_SCAN_POSITION_SECOND, 0ul);
    WriteFormatUnitU32(FW_NEAR_SCAN_POSITION_FIRST, 0ul);
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 1u);

    BUILD_TYPE2_LEAD_IN_TOC(0u);
    if(PREPARE_PRIMARY_TYPE2_SLOTS(0u) == 0u)
        return 0u;

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA524, 0u);
    if(ACQUIRE_FORTY_FRAME_TYPE2_BASE_WINDOW() == 0u)
        return 0u;
    if(ACQUIRE_TYPE2_TDI_SEQUENCE() == 0u)
        return 0u;

    BUILD_ALL_CONTROLLER_RECORD_PAGES();
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) == 0u)
        COMPLETE_SCSI_NO_SENSE();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 8u));
    return 1u;
}

/* RAM:fde6ad */
u8 InitializeFormatUnitCode11Type2ControllerRecordPipelineState(u8 operation_mode)
{
    u32 ring_slot_count;

    RESET_RAW_READ_PIPELINE();
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_1292, 0u);

    ring_slot_count = ReadFormatUnitU32(FW_NEAR_UNKNOWN_156A);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    ring_slot_count = ReadFormatUnitU32(FW_NEAR_UNKNOWN_156A);
    WriteFormatUnitU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);

    WriteFormatUnitU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_126A, ReadFormatUnitU32(FW_NEAR_UNKNOWN_1562));
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_126E, 0ul);

    CLEAR_RAW_READ_CONTROLLER_REGIONS();
    INITIALIZE_RAW_READ_SECTOR_CONTROL();
    CLEAR_RAW_READ_PIPELINE_STATUS();
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_1488, 0ul);
    WriteFormatUnitU32(FW_NEAR_UNKNOWN_148C, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWrite8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH, 0u);
    INITIALIZE_TYPE2_GEOMETRY();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 1u);
    BUILD_TYPE2_LEAD_IN_TOC(0u);

    if(ACQUIRE_REQUIRED_RECORD_RUNS(operation_mode) != 0u)
    {
        RECALCULATE_TYPE2_GEOMETRY();
        BUILD_ACTIVE_RECORD_BOUNDARIES();
    }
    return 1u;
}
