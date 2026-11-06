#include "raw_sector_sequence.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "raw_read_frame_sequence.h"
#include "raw_read_runtime.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_sector_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadRawSectorSequenceU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRawSectorSequenceU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 RawSectorSequenceTrackHasDataControl(u8 track_number)
{
    fw_addr_t summary_address = FW_NEAR_UNKNOWN_F2B7 + (fw_addr_t)((u16)track_number * 6u);

    return (u8)((FirmwareRead8(summary_address) & 0x40u) == 0x40u);
}

/* RAM:fe8337 */
/* The semantic C order of the final two bytes maps preserve_main_buffer to
 * native R3 and clear_subchannel_slot to native R2. */
u8 PLEXTOR_NEAR_FUNCTION AcquireRawSectorSequence(u32 frame_count, u8 q_metadata_mode, u8 preserve_main_buffer, u8 clear_subchannel_slot)
{
    u8 operation_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_E600);
    fw_addr_t main_data_cursor;
    u32 capture_count;
    u32 capture_index;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_sector_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if(operation_type == 0u || operation_type == 1u)
    {
        preserve_main_buffer = RawSectorSequenceTrackHasDataControl(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF));
    }
    else if(operation_type >= 2u && operation_type <= 4u)
    {
        preserve_main_buffer = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) == 0x40u);
    }
    else if(operation_type == 5u || operation_type == 6u)
    {
        u8 track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_14CC);

        if(track_number == 0xbbu)
            track_number = SEQUENCE_SELECT_TRACK();
        else
            track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_14CC);
        clear_subchannel_slot = 0u;
        preserve_main_buffer = RawSectorSequenceTrackHasDataControl(track_number);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, preserve_main_buffer == 1u ? 8u : 2u);
    main_data_cursor = SEQUENCE_RESUME_RING();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    for(capture_index = 0u; capture_index != 10u; ++capture_index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272 + capture_index, 0u);

    if(q_metadata_mode == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, preserve_main_buffer == 1u ? 0x60u : 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 0xffu);
        if((s32)ReadRawSectorSequenceU32(FW_NEAR_UNKNOWN_1598) < 150l)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 1u);
        SEQUENCE_CONVERT_POSITION((s32)ReadRawSectorSequenceU32(FW_NEAR_UNKNOWN_1598), tuple_address);
        SEQUENCE_PACK_TUPLE((u16)tuple_address);
        SEQUENCE_COPY_TUPLE(FW_NEAR_UNKNOWN_1279, (u16)tuple_address);
    }
    else if(q_metadata_mode == 1u)
    {
        SEQUENCE_CONVERT_POSITION((s32)ReadRawSectorSequenceU32(FW_NEAR_UNKNOWN_1598), tuple_address);
        SEQUENCE_PACK_TUPLE((u16)tuple_address);
        SEQUENCE_COPY_TUPLE(FW_NEAR_UNKNOWN_1275, (u16)tuple_address);
    }

    capture_count = frame_count + 1ul;
    for(capture_index = 0u; capture_index < capture_count; ++capture_index)
    {
        if(SEQUENCE_WAIT_CAPACITY() == 0u)
        {
            SEQUENCE_REPORT_FAILURE(7u);
            return 0u;
        }
        if(SEQUENCE_CAPTURE(main_data_cursor, preserve_main_buffer, clear_subchannel_slot) == 0u)
        {
            SEQUENCE_REPORT_FAILURE(7u);
            return 0u;
        }

        WriteRawSectorSequenceU32(FW_NEAR_UNKNOWN_126E, main_data_cursor);
        SEQUENCE_ENQUEUE();
        WriteRawSectorSequenceU32(FW_NEAR_UNKNOWN_1598, ReadRawSectorSequenceU32(FW_NEAR_UNKNOWN_1598) + 1ul);
        main_data_cursor = SEQUENCE_ADVANCE_RING(main_data_cursor);

        if(q_metadata_mode == 1u)
        {
            SEQUENCE_INCREMENT_MSF(FW_NEAR_UNKNOWN_1275);
        }
        else if(q_metadata_mode == 2u)
        {
            SEQUENCE_INCREMENT_MSF(FW_NEAR_UNKNOWN_1279);
            if((s32)ReadRawSectorSequenceU32(FW_NEAR_UNKNOWN_1598) > 150l)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
        }
        else if(q_metadata_mode == 0u)
        {
            u8 separator = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1278) + 1u);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, separator > 9u ? 0u : separator);
        }
    }

    return 1u;
}
