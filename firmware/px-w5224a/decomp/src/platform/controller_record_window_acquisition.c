#include "controller_record_window_acquisition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_window_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRecordWindowU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRecordWindowU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteRecordWindowU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildRecordWindowPackedTuple(s32 frame_position, fw_addr_t temporary_tuple, fw_addr_t destination)
{
    WINDOW_CONVERT(frame_position, temporary_tuple);
    WINDOW_PACK((u16)temporary_tuple);
    WINDOW_COPY_TUPLE((fw_addr_t)(u16)temporary_tuple, destination, 3u);
}

static fw_addr_t FinishRecordWindowDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple)
{
    WINDOW_UPDATE_Q();
    WINDOW_ENQUEUE();
    WriteRecordWindowU32(FW_NEAR_UNKNOWN_1598, ReadRecordWindowU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    WINDOW_INCREMENT((u16)temporary_tuple);
    WINDOW_INCREMENT(FW_NEAR_UNKNOWN_1428);
    cursor = WINDOW_ADVANCE_RING(cursor);
    WINDOW_ADVANCE_Q(0u);
    return cursor;
}

static fw_addr_t EmitRecordWindowDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple, u8 config_index, u8 use_config, u8 terminal)
{
    WINDOW_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    if(use_config != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + config_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
    else if(terminal != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_142B) | 0xa0u));
    WINDOW_SUBMIT(cursor, 0u);
    return FinishRecordWindowDescriptor(cursor, temporary_tuple);
}

static void ResetRecordWindowRawReadState(void)
{
    u32 ring_slot_count = ReadRecordWindowU32(FW_NEAR_UNKNOWN_156A);

    WriteRecordWindowU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteRecordWindowU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteRecordWindowU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteRecordWindowU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteRecordWindowU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteRecordWindowU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteRecordWindowU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteRecordWindowU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteRecordWindowU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteRecordWindowU32(FW_NEAR_UNKNOWN_126A, ReadRecordWindowU32(FW_NEAR_UNKNOWN_1562));
    WriteRecordWindowU32(FW_NEAR_UNKNOWN_126E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
}

/* RAM:fa05ef */
u8 AcquireFortyFrameType2ControllerRecordWindow(void)
{
    u8 attempt = 0u;

    for(;;)
    {
        fw_addr_t main_data_cursor;
        s32 start_frame_position = (s32)ReadRecordWindowU32(FW_HUGE_UNKNOWN_2BA595);
        u8 descriptor_index;
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_window_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 10u);
        main_data_cursor = WINDOW_RESUME_RING();
        WriteRecordWindowU32(FW_NEAR_UNKNOWN_1598, (u32)start_frame_position);
        WriteRecordWindowU32(FW_NEAR_UNKNOWN_8000, (u32)start_frame_position);
        BuildRecordWindowPackedTuple(start_frame_position, temporary_tuple, FW_NEAR_UNKNOWN_1428);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 2u);

        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0x20u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018025, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018026, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018027, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018028, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018021, 0u);
        for(descriptor_index = 0u; descriptor_index != 10u; ++descriptor_index)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272 + descriptor_index, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, 0x71u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
        BuildRecordWindowPackedTuple(start_frame_position - 150l, temporary_tuple, FW_NEAR_UNKNOWN_1275);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
        BuildRecordWindowPackedTuple(start_frame_position, temporary_tuple, FW_NEAR_UNKNOWN_1279);

        for(descriptor_index = 0u; descriptor_index != 5u; ++descriptor_index)
            main_data_cursor = EmitRecordWindowDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
        for(descriptor_index = 0u; descriptor_index != 32u; ++descriptor_index)
        {
            WINDOW_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            WINDOW_SUBMIT(main_data_cursor, 0u);
            WINDOW_BUILD_MIP();
            WINDOW_WRITE_MARKER();
            WINDOW_COPY_PAYLOAD(main_data_cursor + 0x18ul, FW_HUGE_UNKNOWN_2BA5DF, 0x0014u);
            main_data_cursor = FinishRecordWindowDescriptor(main_data_cursor, temporary_tuple);
        }
        for(descriptor_index = 5u; descriptor_index != 7u; ++descriptor_index)
            main_data_cursor = EmitRecordWindowDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
        main_data_cursor = EmitRecordWindowDescriptor(main_data_cursor, temporary_tuple, 0u, 0u, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        if(WINDOW_WAIT_FOR_READ() != 0u)
        {
            WINDOW_SET_NO_SENSE();
            return 1u;
        }

        WINDOW_LATCH_FAILURE();
        ++attempt;
        ResetRecordWindowRawReadState();
        if(attempt >= 3u)
        {
            WINDOW_REPORT_FAILURE(5u);
            return 0u;
        }
    }
}
