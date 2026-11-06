#include "controller_record_leadout_acquisition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_leadout_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadLeadOutU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteLeadOutU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteLeadOutU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildLeadOutPackedTuple(s32 frame_position, fw_addr_t temporary_tuple, fw_addr_t destination)
{
    LEADOUT_CONVERT(frame_position, temporary_tuple);
    LEADOUT_PACK((u16)temporary_tuple);
    LEADOUT_COPY((fw_addr_t)(u16)temporary_tuple, destination, 3u);
}

static fw_addr_t EmitLeadOutDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple, u8 config_index, u8 use_config, u8 terminal)
{
    LEADOUT_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    if(use_config != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + config_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
    else if(terminal != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_142B) | 0xa0u));
    LEADOUT_SUBMIT(cursor, 0u);
    LEADOUT_UPDATE_Q();
    LEADOUT_ENQUEUE();
    WriteLeadOutU32(FW_NEAR_UNKNOWN_1598, ReadLeadOutU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    LEADOUT_INCREMENT((u16)temporary_tuple);
    LEADOUT_INCREMENT(FW_NEAR_UNKNOWN_1428);
    cursor = LEADOUT_ADVANCE_RING(cursor);
    LEADOUT_ADVANCE_Q(0u);
    return cursor;
}

static void ResetLeadOutRawReadState(void)
{
    u32 ring_slot_count = ReadLeadOutU32(FW_NEAR_UNKNOWN_156A);

    WriteLeadOutU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteLeadOutU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteLeadOutU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteLeadOutU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteLeadOutU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteLeadOutU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteLeadOutU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteLeadOutU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteLeadOutU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteLeadOutU32(FW_NEAR_UNKNOWN_126A, ReadLeadOutU32(FW_NEAR_UNKNOWN_1562));
    WriteLeadOutU32(FW_NEAR_UNKNOWN_126E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
}

/* RAM:fa24e9 */
u8 AcquireType2LeadOutControllerRecordSequence(void)
{
    u8 attempt = 0u;

    for(;;)
    {
        fw_addr_t main_data_cursor;
        u32 target;
        u16 body_index;
        u8 descriptor_index;
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_leadout_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 10u);
        main_data_cursor = LEADOUT_RESUME_RING();
        target = ReadLeadOutU32(FW_HUGE_UNKNOWN_2BA58D);
        WriteLeadOutU32(FW_NEAR_UNKNOWN_1598, target);
        WriteLeadOutU32(FW_NEAR_UNKNOWN_8000, target);
        BuildLeadOutPackedTuple((s32)target, temporary_tuple, FW_NEAR_UNKNOWN_1428);
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
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 0xaau);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1275, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1276, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1277, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
        BuildLeadOutPackedTuple((s32)target, temporary_tuple, FW_NEAR_UNKNOWN_1279);

        for(descriptor_index = 0u; descriptor_index != 5u; ++descriptor_index)
            main_data_cursor = EmitLeadOutDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
        for(body_index = 0u; body_index != 0x1a57u; ++body_index)
        {
            if(LEADOUT_WAIT_CAPACITY() == 0u)
                break;
            main_data_cursor = EmitLeadOutDescriptor(main_data_cursor, temporary_tuple, 0u, 0u, 0u);
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        {
            for(descriptor_index = 5u; descriptor_index != 7u; ++descriptor_index)
                main_data_cursor = EmitLeadOutDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
            main_data_cursor = EmitLeadOutDescriptor(main_data_cursor, temporary_tuple, 0u, 0u, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
            if(LEADOUT_WAIT_FOR_READ() != 0u)
            {
                LEADOUT_SET_NO_SENSE();
                return 1u;
            }
        }

        LEADOUT_LATCH_FAILURE();
        ++attempt;
        ResetLeadOutRawReadState();
        if(attempt >= 3u)
        {
            LEADOUT_REPORT_FAILURE(5u);
            return 0u;
        }
    }
}
