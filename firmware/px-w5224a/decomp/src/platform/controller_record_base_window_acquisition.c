#include "controller_record_base_window_acquisition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_base_window_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBaseWindowU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadBaseWindowU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteBaseWindowU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteBaseWindowU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildBaseWindowPackedTuple(s32 frame_position, fw_addr_t temporary_tuple)
{
    BASE_CONVERT(frame_position, temporary_tuple);
    BASE_PACK((u16)temporary_tuple);
    BASE_COPY_FAR((fw_addr_t)(u16)temporary_tuple, FW_NEAR_UNKNOWN_1428, 3u);
}

static void CopyBaseWindowQPacket(fw_addr_t temporary_tuple)
{
    u16 record_index = ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C5);
    u16 source = (u16)(0x8012u + (u16)(record_index * 10u));

    BASE_COPY_Q(FW_NEAR_UNKNOWN_1272, source, 10u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x0fu) == 1u)
        BASE_COPY_FAR((fw_addr_t)(u16)temporary_tuple, FW_NEAR_UNKNOWN_1275, 3u);
}

static fw_addr_t FinishBaseWindowDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple)
{
    CopyBaseWindowQPacket(temporary_tuple);
    BASE_UPDATE_Q();
    BASE_ENQUEUE();
    WriteBaseWindowU32(FW_NEAR_UNKNOWN_1598, ReadBaseWindowU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    BASE_INCREMENT((u16)temporary_tuple);
    BASE_INCREMENT(FW_NEAR_UNKNOWN_1428);
    return BASE_ADVANCE_RING(cursor);
}

static void SelectNextBaseWindowQRecord(void)
{
    u16 selected_record;

    WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C7, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14C9) == 0u)
    {
        u16 low_cursor = (u16)(ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C1) + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
        if(low_cursor == (u16)(ReadBaseWindowU16(FW_NEAR_UNKNOWN_8008) + 1u))
            low_cursor = 0u;
        WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C1, low_cursor);
        selected_record = low_cursor;
    }
    else
    {
        u16 high_cursor = ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C3);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 0u);
        selected_record = high_cursor;
        ++high_cursor;
        if(high_cursor == (u16)(ReadBaseWindowU16(FW_NEAR_UNKNOWN_800A) + 1u))
            high_cursor = (u16)(ReadBaseWindowU16(FW_NEAR_UNKNOWN_8008) + 1u);
        WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C3, high_cursor);
    }
    WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C5, selected_record);
}

static void ResetBaseWindowRawReadState(void)
{
    u32 ring_slot_count = ReadBaseWindowU32(FW_NEAR_UNKNOWN_156A);

    WriteBaseWindowU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteBaseWindowU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteBaseWindowU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteBaseWindowU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteBaseWindowU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteBaseWindowU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteBaseWindowU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteBaseWindowU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteBaseWindowU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteBaseWindowU32(FW_NEAR_UNKNOWN_126A, ReadBaseWindowU32(FW_NEAR_UNKNOWN_1562));
    WriteBaseWindowU32(FW_NEAR_UNKNOWN_126E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
}

/* RAM:fa0000 */
u8 AcquireFortyFrameType2ControllerRecordBaseWindow(void)
{
    u8 attempt = 0u;

    for(;;)
    {
        fw_addr_t main_data_cursor;
        s32 start_frame_position = (s32)ReadBaseWindowU32(FW_HUGE_UNKNOWN_2BA591);
        u8 descriptor_index;
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_base_window_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 10u);
        main_data_cursor = BASE_RESUME_RING();
        WriteBaseWindowU32(FW_NEAR_UNKNOWN_1598, (u32)start_frame_position);
        WriteBaseWindowU32(FW_NEAR_UNKNOWN_8000, (u32)start_frame_position);
        BuildBaseWindowPackedTuple(start_frame_position, temporary_tuple);
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

        WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C5, 0u);
        WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C1, 0u);
        WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C3, (u16)(ReadBaseWindowU16(FW_NEAR_UNKNOWN_8008) + 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
        WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C7, 0u);
        BASE_ADVANCE_CAPTURE();

        for(descriptor_index = 0u; descriptor_index != 5u; ++descriptor_index)
        {
            BASE_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + descriptor_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
            BASE_SUBMIT(main_data_cursor, 0u);
            main_data_cursor = FinishBaseWindowDescriptor(main_data_cursor, temporary_tuple);
            WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C7) + 1u));
            if(ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C7) >= 3u && descriptor_index + 1u < 5u)
                SelectNextBaseWindowQRecord();
        }

        for(descriptor_index = 0u; descriptor_index != 32u; ++descriptor_index)
        {
            BASE_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            BASE_SUBMIT(main_data_cursor, 0u);
            BASE_BUILD_MIP();
            BASE_COPY_PAYLOAD(main_data_cursor + 0x18ul, FW_HUGE_UNKNOWN_2BA5DF, 0x0014u);
            main_data_cursor = FinishBaseWindowDescriptor(main_data_cursor, temporary_tuple);
            WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C7) + 1u));
            if(ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C7) >= 3u && descriptor_index + 1u < 32u)
                SelectNextBaseWindowQRecord();
        }

        for(descriptor_index = 5u; descriptor_index != 7u; ++descriptor_index)
        {
            BASE_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + descriptor_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
            BASE_SUBMIT(main_data_cursor, 0u);
            main_data_cursor = FinishBaseWindowDescriptor(main_data_cursor, temporary_tuple);
            WriteBaseWindowU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C7) + 1u));
            if(ReadBaseWindowU16(FW_NEAR_UNKNOWN_14C7) >= 3u && descriptor_index + 1u < 7u)
                SelectNextBaseWindowQRecord();
        }

        BASE_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_142B) | 0xa0u));
        BASE_SUBMIT(main_data_cursor, 0u);
        FinishBaseWindowDescriptor(main_data_cursor, temporary_tuple);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        if(BASE_WAIT_FOR_READ() != 0u)
        {
            BASE_SET_NO_SENSE();
            return 1u;
        }

        BASE_LATCH_FAILURE();
        ++attempt;
        ResetBaseWindowRawReadState();
        if(attempt >= 3u)
        {
            BASE_REPORT_FAILURE(4u);
            return 0u;
        }
    }
}
