#include "controller_record_secondary_slot_acquisition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_secondary_slot_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadSecondarySlotU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteSecondarySlotU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteSecondarySlotU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildSecondarySlotPackedTuple(s32 frame_position, fw_addr_t temporary_tuple, fw_addr_t destination)
{
    SECONDARY_CONVERT(frame_position, temporary_tuple);
    SECONDARY_PACK((u16)temporary_tuple);
    SECONDARY_COPY_TUPLE((fw_addr_t)(u16)temporary_tuple, destination, 3u);
}

static fw_addr_t EmitSecondarySlotDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple, u8 config_index, u8 use_config, u8 terminal)
{
    SECONDARY_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    if(use_config != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + config_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
    else if(terminal != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_142B) | 0xa0u));
    SECONDARY_SUBMIT(cursor, 0u);
    SECONDARY_UPDATE_Q();
    SECONDARY_ENQUEUE();
    WriteSecondarySlotU32(FW_NEAR_UNKNOWN_1598, ReadSecondarySlotU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    SECONDARY_INCREMENT((u16)temporary_tuple);
    SECONDARY_INCREMENT(FW_NEAR_UNKNOWN_1428);
    cursor = SECONDARY_ADVANCE_RING(cursor);
    SECONDARY_ADVANCE_Q(0u);
    return cursor;
}

static u8 SecondarySlotPageIndex(u8 descriptor_index)
{
    u8 page_index = (u8)((descriptor_index & 8u) >> 1);

    page_index |= descriptor_index & 1u;
    if((descriptor_index & 0x10u) == 0u)
        page_index |= descriptor_index & 2u;
    else
        page_index |= (u8)((descriptor_index & 4u) >> 1);
    return page_index;
}

static void ResetSecondarySlotRawReadState(void)
{
    u32 ring_slot_count = ReadSecondarySlotU32(FW_NEAR_UNKNOWN_156A);

    WriteSecondarySlotU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteSecondarySlotU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteSecondarySlotU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteSecondarySlotU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteSecondarySlotU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteSecondarySlotU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteSecondarySlotU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteSecondarySlotU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteSecondarySlotU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteSecondarySlotU32(FW_NEAR_UNKNOWN_126A, ReadSecondarySlotU32(FW_NEAR_UNKNOWN_1562));
    WriteSecondarySlotU32(FW_NEAR_UNKNOWN_126E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
}

/* RAM:fa1341 */
u8 AcquireSecondaryType2ControllerRecordSlot(u8 acquisition_mode, u8 record_slot_index, s32 start_frame_position)
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
#include "bindings/platform/controller_record_secondary_slot_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

        main_data_cursor = SECONDARY_RESUME_RING();
        WriteSecondarySlotU32(FW_NEAR_UNKNOWN_1598, (u32)start_frame_position);
        BuildSecondarySlotPackedTuple(start_frame_position, temporary_tuple, FW_NEAR_UNKNOWN_1428);
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
        BuildSecondarySlotPackedTuple(start_frame_position - 150l, temporary_tuple, FW_NEAR_UNKNOWN_1275);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
        BuildSecondarySlotPackedTuple(start_frame_position, temporary_tuple, FW_NEAR_UNKNOWN_1279);

        for(descriptor_index = 0u; descriptor_index != 5u; ++descriptor_index)
            main_data_cursor = EmitSecondarySlotDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);

        for(descriptor_index = 0u; descriptor_index != 32u; ++descriptor_index)
        {
            u8 page_index = SecondarySlotPageIndex(descriptor_index);

            SECONDARY_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            SECONDARY_SUBMIT(main_data_cursor, 0u);
            if(acquisition_mode == 0u)
                SECONDARY_BUILD_WORKSPACE(record_slot_index, page_index);
            else
                SECONDARY_COPY_WORKSPACE(record_slot_index, page_index);
            SECONDARY_WRITE_MARKER();
            SECONDARY_COPY_PAYLOAD(main_data_cursor + 0x18ul, FW_HUGE_UNKNOWN_2BA62F, 0x0200u);
            SECONDARY_UPDATE_Q();
            SECONDARY_ENQUEUE();
            WriteSecondarySlotU32(FW_NEAR_UNKNOWN_1598, ReadSecondarySlotU32(FW_NEAR_UNKNOWN_1598) + 1ul);
            SECONDARY_INCREMENT((u16)temporary_tuple);
            SECONDARY_INCREMENT(FW_NEAR_UNKNOWN_1428);
            main_data_cursor = SECONDARY_ADVANCE_RING(main_data_cursor);
            SECONDARY_ADVANCE_Q(0u);
        }

        for(descriptor_index = 5u; descriptor_index != 7u; ++descriptor_index)
            main_data_cursor = EmitSecondarySlotDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
        main_data_cursor = EmitSecondarySlotDescriptor(main_data_cursor, temporary_tuple, 0u, 0u, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        if(SECONDARY_WAIT_FOR_READ() != 0u)
        {
            SECONDARY_SET_NO_SENSE();
            return 1u;
        }

        SECONDARY_LATCH_FAILURE();
        ++attempt;
        ResetSecondarySlotRawReadState();
        if(attempt >= 2u)
        {
            SECONDARY_REPORT_FAILURE(5u);
            return 0u;
        }
    }
}
