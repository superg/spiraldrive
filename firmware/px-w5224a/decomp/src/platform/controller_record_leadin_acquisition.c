#include "controller_record_leadin_acquisition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_leadin_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadLeadInU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadLeadInU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteLeadInU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteLeadInU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildLeadInPackedTuple(s32 frame_position, fw_addr_t temporary_tuple, fw_addr_t destination)
{
    LEADIN_CONVERT(frame_position, temporary_tuple);
    LEADIN_PACK((u16)temporary_tuple);
    LEADIN_COPY_TUPLE((fw_addr_t)(u16)temporary_tuple, destination, 3u);
}

static void AdvanceLeadInQRecordSelection(void)
{
    u16 index;

    WriteLeadInU16(FW_NEAR_UNKNOWN_14C7, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14C9) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
        index = (u16)(ReadLeadInU16(FW_NEAR_UNKNOWN_14C1) + 1u);
        if(index == (u16)(ReadLeadInU16(FW_NEAR_UNKNOWN_8008) + 1u))
            index = 0u;
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C1, index);
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C5, index);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 0u);
        index = ReadLeadInU16(FW_NEAR_UNKNOWN_14C3);
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C5, index);
        ++index;
        if(index == (u16)(ReadLeadInU16(FW_NEAR_UNKNOWN_800A) + 1u))
            index = (u16)(ReadLeadInU16(FW_NEAR_UNKNOWN_8008) + 1u);
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C3, index);
    }
}

static fw_addr_t EmitLeadInDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple, u8 config_index, u8 use_config, u8 terminal)
{
    u16 source;

    LEADIN_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    if(use_config != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + config_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
    else if(terminal != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_142B) | 0xa0u));
    LEADIN_SUBMIT(cursor, 0u);
    source = (u16)(0x8012u + ReadLeadInU16(FW_NEAR_UNKNOWN_14C5) * 10u);
    LEADIN_COPY_Q(FW_NEAR_UNKNOWN_1272, source, 10u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x0fu) == 1u)
        LEADIN_COPY_TUPLE((fw_addr_t)(u16)temporary_tuple, FW_NEAR_UNKNOWN_1275, 3u);
    LEADIN_UPDATE_Q();
    LEADIN_ENQUEUE();
    WriteLeadInU32(FW_NEAR_UNKNOWN_1598, ReadLeadInU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    LEADIN_INCREMENT((u16)temporary_tuple);
    LEADIN_INCREMENT(FW_NEAR_UNKNOWN_1428);
    cursor = LEADIN_ADVANCE_RING(cursor);
    if(terminal == 0u)
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadLeadInU16(FW_NEAR_UNKNOWN_14C7) + 1u));
    return cursor;
}

static void ResetLeadInRawReadState(void)
{
    u32 ring_slot_count = ReadLeadInU32(FW_NEAR_UNKNOWN_156A);

    WriteLeadInU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteLeadInU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteLeadInU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteLeadInU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteLeadInU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteLeadInU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteLeadInU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteLeadInU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteLeadInU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteLeadInU32(FW_NEAR_UNKNOWN_126A, ReadLeadInU32(FW_NEAR_UNKNOWN_1562));
    WriteLeadInU32(FW_NEAR_UNKNOWN_126E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
}

/* RAM:fa1ac5 */
u8 AcquireType2LeadInControllerRecordSequence(void)
{
    u8 attempt = 0u;

    for(;;)
    {
        fw_addr_t main_data_cursor;
        u32 frame_count;
        u16 frame_index;
        u8 descriptor_index;
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_leadin_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 10u);
        main_data_cursor = LEADIN_RESUME_RING();
        WriteLeadInU32(FW_NEAR_UNKNOWN_1598, ReadLeadInU32(FW_NEAR_UNKNOWN_114F));
        WriteLeadInU32(FW_NEAR_UNKNOWN_8000, ReadLeadInU32(FW_NEAR_UNKNOWN_114F));
        BuildLeadInPackedTuple((s32)ReadLeadInU32(FW_NEAR_UNKNOWN_1598), temporary_tuple, FW_NEAR_UNKNOWN_1428);
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
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C5, 0u);
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C1, 0u);
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C3, (u16)(ReadLeadInU16(FW_NEAR_UNKNOWN_8008) + 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
        WriteLeadInU16(FW_NEAR_UNKNOWN_14C7, 0u);

        descriptor_index = 0u;
        while(descriptor_index < 5u)
        {
            while(ReadLeadInU16(FW_NEAR_UNKNOWN_14C7) < 3u && descriptor_index < 5u)
            {
                main_data_cursor = EmitLeadInDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
                ++descriptor_index;
            }
            if(descriptor_index < 5u)
                AdvanceLeadInQRecordSelection();
        }

        frame_count = LEADIN_MULTIPLY(39ul, (u32)LEADIN_DIVIDE((s32)(ReadLeadInU32(FW_HUGE_UNKNOWN_2BA577) + 150ul), 32l));
        frame_count = frame_count - 7ul - ReadLeadInU32(FW_NEAR_UNKNOWN_114F);
        frame_index = 0u;
        while((u32)frame_index < frame_count)
        {
            while(ReadLeadInU16(FW_NEAR_UNKNOWN_14C7) < 3u && (u32)frame_index < frame_count && LEADIN_WAIT_CAPACITY() != 0u)
            {
                main_data_cursor = EmitLeadInDescriptor(main_data_cursor, temporary_tuple, 0u, 0u, 0u);
                ++frame_index;
            }
            if((u32)frame_index < frame_count)
                AdvanceLeadInQRecordSelection();
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        {
            descriptor_index = 5u;
            while(descriptor_index < 7u)
            {
                while(ReadLeadInU16(FW_NEAR_UNKNOWN_14C7) < 3u && descriptor_index < 7u)
                {
                    main_data_cursor = EmitLeadInDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
                    ++descriptor_index;
                }
                if(descriptor_index < 7u)
                    AdvanceLeadInQRecordSelection();
            }
            main_data_cursor = EmitLeadInDescriptor(main_data_cursor, temporary_tuple, 0u, 0u, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
            if(LEADIN_WAIT_FOR_READ() != 0u)
            {
                LEADIN_SET_NO_SENSE();
                return 1u;
            }
        }

        LEADIN_LATCH_FAILURE();
        ++attempt;
        ResetLeadInRawReadState();
        if(attempt >= 3u)
        {
            LEADIN_REPORT_FAILURE(4u);
            return 0u;
        }
    }
}
