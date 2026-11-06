#include "controller_record_tdi_acquisition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_record_tdi_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadTdiU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteTdiU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteTdiU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void BuildTdiPackedTuple(s32 frame_position, fw_addr_t temporary_tuple, fw_addr_t destination)
{
    TDI_CONVERT(frame_position, temporary_tuple);
    TDI_PACK((u16)temporary_tuple);
    TDI_COPY_TUPLE((fw_addr_t)(u16)temporary_tuple, destination, 3u);
}

static fw_addr_t EmitTdiDescriptor(fw_addr_t cursor, fw_addr_t temporary_tuple, u8 config_index, u8 use_config, u8 terminal)
{
    TDI_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    if(use_config != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)((FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84890) + config_index) << 5) | FirmwareReadNear8(FW_NEAR_UNKNOWN_142B)));
    else if(terminal != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_018028, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_142B) | 0xa0u));
    TDI_SUBMIT(cursor, 0u);
    TDI_UPDATE_Q();
    TDI_ENQUEUE();
    WriteTdiU32(FW_NEAR_UNKNOWN_1598, ReadTdiU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    TDI_INCREMENT((u16)temporary_tuple);
    TDI_INCREMENT(FW_NEAR_UNKNOWN_1428);
    cursor = TDI_ADVANCE_RING(cursor);
    TDI_ADVANCE_Q(1u);
    return cursor;
}

static void ResetTdiRawReadState(void)
{
    u32 ring_slot_count = ReadTdiU32(FW_NEAR_UNKNOWN_156A);

    WriteTdiU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteTdiU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteTdiU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteTdiU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteTdiU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteTdiU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteTdiU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteTdiU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteTdiU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteTdiU32(FW_NEAR_UNKNOWN_126A, ReadTdiU32(FW_NEAR_UNKNOWN_1562));
    WriteTdiU32(FW_NEAR_UNKNOWN_126E, 0ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
}

/* RAM:fa20f3 */
u8 AcquireType2TdiControllerRecordSequence(void)
{
    static const u8 tdi_payload[13] = { 0x54u, 0x44u, 0x49u, 0x01u, 0x50u, 0x01u, 0x01u, 0x01u, 0x01u, 0x91u, 0x00u, 0x00u, 0x32u };
    u8 attempt = 0u;

    for(;;)
    {
        fw_addr_t main_data_cursor;
        u8 descriptor_index;
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_record_tdi_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 10u);
        main_data_cursor = TDI_RESUME_RING();
        WriteTdiU32(FW_NEAR_UNKNOWN_1598, 0ul);
        WriteTdiU32(FW_NEAR_UNKNOWN_8000, 0ul);
        BuildTdiPackedTuple(0l, temporary_tuple, FW_NEAR_UNKNOWN_1428);
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
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 0u);
        BuildTdiPackedTuple(149l, temporary_tuple, FW_NEAR_UNKNOWN_1275);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
        BuildTdiPackedTuple(0l, temporary_tuple, FW_NEAR_UNKNOWN_1279);
        for(descriptor_index = 0u; descriptor_index != 13u; ++descriptor_index)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_143C + descriptor_index, tdi_payload[descriptor_index]);

        for(descriptor_index = 0u; descriptor_index != 5u; ++descriptor_index)
            main_data_cursor = EmitTdiDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
        for(descriptor_index = 0u; descriptor_index != 138u; ++descriptor_index)
        {
            TDI_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            TDI_SUBMIT(main_data_cursor, 0u);
            TDI_COPY_PAYLOAD(main_data_cursor + 0x18ul, FW_NEAR_UNKNOWN_143C, 13u);
            TDI_UPDATE_Q();
            TDI_ENQUEUE();
            WriteTdiU32(FW_NEAR_UNKNOWN_1598, ReadTdiU32(FW_NEAR_UNKNOWN_1598) + 1ul);
            TDI_INCREMENT((u16)temporary_tuple);
            TDI_INCREMENT(FW_NEAR_UNKNOWN_1428);
            main_data_cursor = TDI_ADVANCE_RING(main_data_cursor);
            TDI_ADVANCE_Q(1u);
            TDI_SERVICE_HOST();
            TDI_YIELD();
        }
        for(descriptor_index = 5u; descriptor_index != 7u; ++descriptor_index)
            main_data_cursor = EmitTdiDescriptor(main_data_cursor, temporary_tuple, descriptor_index, 1u, 0u);
        main_data_cursor = EmitTdiDescriptor(main_data_cursor, temporary_tuple, 0u, 0u, 1u);
        WriteTdiU32(FW_HUGE_UNKNOWN_2BA59F, ReadTdiU32(FW_NEAR_UNKNOWN_1598) - 1ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        if(TDI_WAIT_FOR_READ() != 0u)
        {
            TDI_SET_NO_SENSE();
            return 1u;
        }

        TDI_LATCH_FAILURE();
        ++attempt;
        ResetTdiRawReadState();
        if(attempt >= 3u)
        {
            TDI_REPORT_FAILURE(0u);
            return 0u;
        }
    }
}
