#include "primary_raw_read_capture_batch.h"
#include "../platform/arithmetic.h"
#include "../platform/controller_buffer_atomic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/primary_raw_read_capture_batch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPrimaryCaptureU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void BuildPrimaryCapturePackedTuple(s32 frame_position, fw_addr_t temporary_tuple, fw_addr_t destination)
{
    CAPTURE_CONVERT(frame_position, temporary_tuple);
    CAPTURE_PACK_BCD((u16)temporary_tuple);
    CAPTURE_COPY((fw_addr_t)(u16)temporary_tuple, destination, 3u);
}

/* RAM:f94500 */
void PLEXTOR_NEAR_FUNCTION PrepareAndSubmitPrimaryRawReadCaptureBatch(u8 batch_count)
{
    u32 main_data_cursor;
    u32 auxiliary_cursor;
    u16 metadata_slot_index;
    u8 batch_index;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/primary_raw_read_capture_batch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    while(CAPTURE_BUSY() != 0u)
        CAPTURE_YIELD();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, 0x71u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
    BuildPrimaryCapturePackedTuple((s32)ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_1598), temporary_tuple, FW_NEAR_UNKNOWN_1428);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 2u);
    BuildPrimaryCapturePackedTuple((s32)(ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_1598) - 150ul), temporary_tuple, FW_NEAR_UNKNOWN_1275);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
    BuildPrimaryCapturePackedTuple((s32)ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_1598), temporary_tuple, FW_NEAR_UNKNOWN_1279);

    main_data_cursor = ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_126E);
    metadata_slot_index = (u16)ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_1262);
    auxiliary_cursor = ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_126A);

    for(batch_index = 0u; batch_index < batch_count; ++batch_index)
    {
        u8 format_index;

        CAPTURE_WRITE_CONTROLLER_BLOCK(FW_NEAR_UNKNOWN_1428);
        CAPTURE_SUBMIT(main_data_cursor, 0xaau);
        main_data_cursor += 0x930ul;
        if(main_data_cursor >= ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_155E))
            main_data_cursor -= ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_155E);

        format_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06);
        if((s8)format_index > 0)
        {
            fw_addr_t metadata_cursor = FirmwareAddressIncrement24(ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_156E) + ((u32)metadata_slot_index << 4), FW_HUGE_BUFFER_BANK_BASE);
            u8 byte_index;

            for(byte_index = 0u; byte_index < 10u; ++byte_index)
            {
                FirmwareWrite8(metadata_cursor, FirmwareReadNear8(FW_NEAR_UNKNOWN_1272 + byte_index));
                metadata_cursor = FirmwareAddressIncrement24(metadata_cursor, 1u);
            }
            FirmwareWrite8(metadata_cursor, 0u);
            metadata_cursor = FirmwareAddressIncrement24(metadata_cursor, 1u);
            FirmwareWrite8(metadata_cursor, 0u);

            ++metadata_slot_index;
            if(metadata_slot_index >= (u16)ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_156A))
                metadata_slot_index = (u16)(metadata_slot_index - (u16)ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_156A));
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != 0u)
        {
            fw_addr_t clear_cursor = FirmwareAddressIncrement24(auxiliary_cursor, FW_HUGE_BUFFER_BANK_BASE);
            u16 byte_index;
            u32 auxiliary_span;

            for(byte_index = 0u; byte_index < 0x126u; ++byte_index)
            {
                FirmwareWrite8(clear_cursor, 0u);
                clear_cursor = FirmwareAddressIncrement24(clear_cursor, 1u);
            }
            auxiliary_cursor += 0x126ul;
            auxiliary_span = CAPTURE_MULTIPLY(0x126ul, ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_156A));
            if(auxiliary_cursor >= ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_1562) + auxiliary_span)
                auxiliary_cursor -= CAPTURE_MULTIPLY(0x126ul, ReadPrimaryCaptureU32(FW_NEAR_UNKNOWN_156A));
        }

        CAPTURE_INCREMENT(FW_NEAR_UNKNOWN_1428);
        CAPTURE_INCREMENT(FW_NEAR_UNKNOWN_1279);
        CAPTURE_INCREMENT(FW_NEAR_UNKNOWN_1275);
    }
}
