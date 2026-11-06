#include "controller_record_index_range_acquisition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_index_range_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRecordRangeU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadRecordRangeU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRecordRangeU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteRecordRangeU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildRecordRangePackedTuple(s32 frame_position, fw_addr_t temporary_tuple, fw_addr_t destination)
{
    RANGE_CONVERT(frame_position, temporary_tuple);
    RANGE_PACK((u16)temporary_tuple);
    RANGE_COPY((fw_addr_t)(u16)temporary_tuple, destination, 3u);
}

static fw_addr_t EmitRecordRangeDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple, u8 config_index, u8 use_config, u8 reverse_relative)
{
    u32 target;

    RANGE_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    if(use_config != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + config_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
    RANGE_SUBMIT(cursor, 0xaau);
    RANGE_UPDATE_Q();
    RANGE_ENQUEUE();
    target = ReadRecordRangeU32(FW_NEAR_UNKNOWN_1598) + 1ul;
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_1598, target);
    RANGE_INCREMENT((u16)temporary_tuple);
    RANGE_INCREMENT(FW_NEAR_UNKNOWN_1428);
    cursor = RANGE_ADVANCE_RING(cursor);
    RANGE_ADVANCE_Q(reverse_relative);
    return cursor;
}

/* RAM:fa299d */
u8 AcquireThirtyNineFrameControllerRecordIndexRange(u16 first_index, u16 last_index, u8 operation_mode)
{
    fw_addr_t main_data_cursor;
    u32 target;
    u32 ring_slot_count;
    u16 current_index;
    u16 final_last_index = last_index;
    u8 descriptor_index;
    u8 aborted = 0u;
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_index_range_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5D8, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 10u);
    main_data_cursor = RANGE_RESUME_RING();

    target = (u32)first_index * 39ul + 145ul;
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_1598, target);
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_8000, target);
    BuildRecordRangePackedTuple((s32)target, temporary_tuple, FW_NEAR_UNKNOWN_1428);
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
    BuildRecordRangePackedTuple(target < 150ul ? 4l : (s32)(target - 150ul), temporary_tuple, FW_NEAR_UNKNOWN_1275);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
    BuildRecordRangePackedTuple((s32)target, temporary_tuple, FW_NEAR_UNKNOWN_1279);
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_1455, 39ul);

    current_index = first_index;
    while(current_index <= last_index)
    {
        RANGE_WAIT_CAPACITY();

        for(descriptor_index = 0u; descriptor_index != 5u; ++descriptor_index)
        {
            target = ReadRecordRangeU32(FW_NEAR_UNKNOWN_1598) + 1ul;
            main_data_cursor = EmitRecordRangeDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, (u8)(target < 150ul));
            if(target == 150ul)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1275, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1276, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1277, 0u);
            }
        }
        for(descriptor_index = 0u; descriptor_index != 32u; ++descriptor_index)
            main_data_cursor = EmitRecordRangeDescriptor(main_data_cursor, temporary_tuple, 0u, 0u, 0u);
        for(descriptor_index = 5u; descriptor_index != 7u; ++descriptor_index)
            main_data_cursor = EmitRecordRangeDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);

        RANGE_SERVICE_HOST();
        RANGE_YIELD();
        if((FirmwareReadNear8(FW_NEAR_HOST_COMPLETION_FLAGS) & 0x10u) != 0u)
        {
            if(operation_mode == 3u)
                RANGE_CLEAR_COMPLETION();
            else
                FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 4u));
        }
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 4u) != 0u)
        {
            aborted = 1u;
            final_last_index = current_index;
            break;
        }
        if(FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH) == 1u)
        {
            FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH, 1u);
            aborted = 1u;
            final_last_index = current_index;
            break;
        }
        ++current_index;
    }

    RANGE_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_142B) | 0xa0u));
    RANGE_SUBMIT(main_data_cursor, 0xaau);
    RANGE_UPDATE_Q();
    RANGE_ENQUEUE();
    target = ReadRecordRangeU32(FW_NEAR_UNKNOWN_1598) + 1ul;
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_1598, target);
    RANGE_INCREMENT((u16)temporary_tuple);
    RANGE_INCREMENT(FW_NEAR_UNKNOWN_1428);
    main_data_cursor = RANGE_ADVANCE_RING(main_data_cursor);
    RANGE_ADVANCE_Q(1u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    WriteRecordRangeU32(FW_HUGE_UNKNOWN_2BA59F, target - 1ul);
    while(ReadRecordRangeU16(FW_NEAR_UNKNOWN_1292) > 1u)
        RANGE_WAIT_FOR_READ();
    WriteRecordRangeU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteRecordRangeU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteRecordRangeU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteRecordRangeU16(FW_NEAR_UNKNOWN_1292, 0u);
    ring_slot_count = ReadRecordRangeU32(FW_NEAR_UNKNOWN_156A);
    WriteRecordRangeU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteRecordRangeU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_126A, ReadRecordRangeU32(FW_NEAR_UNKNOWN_1562));
    WriteRecordRangeU32(FW_NEAR_UNKNOWN_126E, 0ul);
    RANGE_REFRESH(first_index, final_last_index);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA5D8, 0u);
    return aborted;
}
