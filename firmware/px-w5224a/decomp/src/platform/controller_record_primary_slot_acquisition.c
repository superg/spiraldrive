#include "controller_record_primary_slot_acquisition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_primary_slot_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPrimarySlotU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadPrimarySlotU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePrimarySlotU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WritePrimarySlotU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildPrimarySlotPackedTuple(s32 frame_position, fw_addr_t temporary_tuple)
{
    PRIMARY_CONVERT(frame_position, temporary_tuple);
    PRIMARY_PACK((u16)temporary_tuple);
    PRIMARY_COPY_FAR((fw_addr_t)(u16)temporary_tuple, FW_NEAR_UNKNOWN_1428, 3u);
}

static u8 PrimarySlotPageIndex(u8 descriptor_index)
{
    u8 page_index = (u8)((descriptor_index & 8u) >> 1);

    page_index |= descriptor_index & 1u;
    if((descriptor_index & 0x10u) == 0u)
        page_index |= descriptor_index & 2u;
    else
        page_index |= (u8)((descriptor_index & 4u) >> 1);
    return page_index;
}

static void CopyPrimarySlotQPacket(fw_addr_t temporary_tuple)
{
    u16 record_index = ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C5);
    u16 source = (u16)(0x8012u + (u16)(record_index * 10u));

    PRIMARY_COPY_Q(FW_NEAR_UNKNOWN_1272, source, 10u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x0fu) == 1u)
        PRIMARY_COPY_FAR((fw_addr_t)(u16)temporary_tuple, FW_NEAR_UNKNOWN_1275, 3u);
}

static fw_addr_t FinishPrimarySlotDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple)
{
    CopyPrimarySlotQPacket(temporary_tuple);
    PRIMARY_UPDATE_Q();
    PRIMARY_ENQUEUE();
    WritePrimarySlotU32(FW_NEAR_UNKNOWN_1598, ReadPrimarySlotU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    PRIMARY_INCREMENT((u16)temporary_tuple);
    PRIMARY_INCREMENT(FW_NEAR_UNKNOWN_1428);
    return PRIMARY_ADVANCE_RING(cursor);
}

static void SelectNextPrimarySlotQRecord(void)
{
    u16 selected_record;

    WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C7, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14C9) == 0u)
    {
        u16 low_cursor = (u16)(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C1) + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
        if(low_cursor == (u16)(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_8008) + 1u))
            low_cursor = 0u;
        WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C1, low_cursor);
        selected_record = low_cursor;
    }
    else
    {
        u16 high_cursor = ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C3);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 0u);
        selected_record = high_cursor;
        ++high_cursor;
        if(high_cursor == (u16)(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_800A) + 1u))
            high_cursor = (u16)(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_8008) + 1u);
        WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C3, high_cursor);
    }
    WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C5, selected_record);
}

static void ResetPrimarySlotRawReadState(void)
{
    u32 ring_slot_count = ReadPrimarySlotU32(FW_NEAR_UNKNOWN_156A);

    WritePrimarySlotU16(FW_NEAR_UNKNOWN_1298, 0u);
    WritePrimarySlotU16(FW_NEAR_UNKNOWN_1294, 0u);
    WritePrimarySlotU16(FW_NEAR_UNKNOWN_1296, 0u);
    WritePrimarySlotU16(FW_NEAR_UNKNOWN_1292, 0u);
    WritePrimarySlotU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WritePrimarySlotU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WritePrimarySlotU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WritePrimarySlotU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WritePrimarySlotU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WritePrimarySlotU32(FW_NEAR_UNKNOWN_126A, ReadPrimarySlotU32(FW_NEAR_UNKNOWN_1562));
    WritePrimarySlotU32(FW_NEAR_UNKNOWN_126E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
}

/* RAM:fa0ab4 */
u8 AcquirePrimaryType2ControllerRecordSlot(u8 acquisition_mode, u8 record_slot_index, s32 start_frame_position)
{
    u8 attempt = 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 10u);

    for(;;)
    {
        fw_addr_t main_data_cursor;
        u8 descriptor_index;
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_primary_slot_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

        main_data_cursor = PRIMARY_RESUME_RING();
        WritePrimarySlotU32(FW_NEAR_UNKNOWN_1598, (u32)start_frame_position);
        BuildPrimarySlotPackedTuple(start_frame_position, temporary_tuple);
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

        WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C5, 0u);
        WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C1, 0u);
        WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C3, (u16)(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_8008) + 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
        WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C7, 0u);
        if(acquisition_mode == 1u)
            PRIMARY_INCREMENT_RECORD(record_slot_index);

        for(descriptor_index = 0u; descriptor_index != 5u; ++descriptor_index)
        {
            PRIMARY_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + descriptor_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
            PRIMARY_SUBMIT(main_data_cursor, 0u);
            main_data_cursor = FinishPrimarySlotDescriptor(main_data_cursor, temporary_tuple);
            WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C7) + 1u));
            if(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C7) >= 3u && descriptor_index + 1u < 5u)
                SelectNextPrimarySlotQRecord();
        }

        for(descriptor_index = 0u; descriptor_index != 32u; ++descriptor_index)
        {
            u8 page_index = PrimarySlotPageIndex(descriptor_index);

            PRIMARY_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            PRIMARY_SUBMIT(main_data_cursor, 0u);
            if(acquisition_mode == 0u)
                PRIMARY_BUILD_WORKSPACE(record_slot_index, page_index);
            else
                PRIMARY_COPY_WORKSPACE(record_slot_index, page_index);
            PRIMARY_COPY_PAYLOAD(main_data_cursor + 0x18ul, FW_HUGE_UNKNOWN_2BA62F, 0x0200u);
            main_data_cursor = FinishPrimarySlotDescriptor(main_data_cursor, temporary_tuple);
            WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C7) + 1u));
            if(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C7) >= 3u && descriptor_index + 1u < 32u)
                SelectNextPrimarySlotQRecord();
        }

        for(descriptor_index = 5u; descriptor_index != 7u; ++descriptor_index)
        {
            PRIMARY_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + descriptor_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
            PRIMARY_SUBMIT(main_data_cursor, 0u);
            main_data_cursor = FinishPrimarySlotDescriptor(main_data_cursor, temporary_tuple);
            WritePrimarySlotU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C7) + 1u));
            if(ReadPrimarySlotU16(FW_NEAR_UNKNOWN_14C7) >= 3u && descriptor_index + 1u < 7u)
                SelectNextPrimarySlotQRecord();
        }

        PRIMARY_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_142B) | 0xa0u));
        PRIMARY_SUBMIT(main_data_cursor, 0u);
        FinishPrimarySlotDescriptor(main_data_cursor, temporary_tuple);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        if(PRIMARY_WAIT_FOR_READ() != 0u)
        {
            PRIMARY_SET_NO_SENSE();
            return 1u;
        }

        PRIMARY_LATCH_FAILURE();
        ++attempt;
        ResetPrimarySlotRawReadState();
        if(attempt >= 2u)
        {
            PRIMARY_REPORT_FAILURE(4u);
            return 0u;
        }
    }
}
