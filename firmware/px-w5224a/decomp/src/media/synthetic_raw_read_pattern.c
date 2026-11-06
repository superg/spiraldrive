#include "synthetic_raw_read_pattern.h"
#include "../platform/controller_buffer_chunk_fill.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "raw_read_frame_sequence.h"
#include "raw_read_runtime.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/synthetic_raw_read_pattern_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadSyntheticPatternU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteSyntheticPatternU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void WriteSyntheticPatternDescriptor(fw_addr_t main_data_cursor, fw_addr_t tuple_address, s32 source_frame_position)
{
    fw_addr_t descriptor = FirmwareAddressIncrement24(main_data_cursor, 0x10ul);
    fw_addr_t tuple_field = FirmwareAddressIncrement24(descriptor, 0x0cul);
    u8 index;

    for(index = 0u; index != 3u; ++index)
    {
        u8 value = FirmwareRead8(tuple_address + index);

        FirmwareWrite8(tuple_field + index, value);
        FirmwareWrite8(tuple_field + 8u + index, value);
    }
    FirmwareWrite8(tuple_field + 3u, 1u);
    FirmwareWrite8(tuple_field + 11u, 1u);
    WriteSyntheticPatternU32(tuple_field + 4u, (u32)source_frame_position);
}

/* RAM:febee5 */
u8 PLEXTOR_NEAR_FUNCTION GenerateSyntheticRawReadPatternRange(s32 start_frame_position, s32 end_frame_position_exclusive)
{
    s32 initial_fill_end = (s32)(ReadSyntheticPatternU32(FW_NEAR_UNKNOWN_156A) + (u32)start_frame_position);
    fw_addr_t main_data_cursor = SYNTHETIC_RESUME_RING();
    u8 batch_count = 45u;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/synthetic_raw_read_pattern_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    ConvertFramePositionToBinaryMsfTuple((s32)ReadSyntheticPatternU32(FW_NEAR_UNKNOWN_1598), tuple_address);
    ConvertThreeBinaryBytesToPackedBcd((u16)tuple_address);
    SET_SYNTHETIC_ACTIVE_BITS();
    WriteSyntheticPatternU32(FW_NEAR_UNKNOWN_1455, 50ul);

    while(start_frame_position < end_frame_position_exclusive)
    {
        s32 source_frame_position;

        while(FirmwareReadBit0036() == 1u)
            SYNTHETIC_YIELD();

        if(batch_count == 50u)
        {
            if(SYNTHETIC_WAIT_CAPACITY() == 0u)
            {
                SYNTHETIC_REPORT_FAILURE(0u);
                return 0u;
            }
            SYNTHETIC_YIELD();
            batch_count = 0u;
        }
        ++batch_count;

        source_frame_position = (s32)(ReadSyntheticPatternU32(FW_NEAR_UNKNOWN_1598) - 150ul);
        if(start_frame_position < initial_fill_end)
        {
            u8 fill_value = (u8)SYNTHETIC_REMAINDER(source_frame_position, 256l);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14DA, 0u);
            SYNTHETIC_FILL(main_data_cursor, 0x0930u, fill_value);
            SYNTHETIC_COPY_AUXILIARY(main_data_cursor, PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF967A), 12u);
        }

        CopyThreeBytes16(FW_NEAR_UNKNOWN_1279, (u16)tuple_address);
        WriteSyntheticPatternDescriptor(main_data_cursor, tuple_address, source_frame_position);
        SYNTHETIC_ENQUEUE();
        SYNTHETIC_ADVANCE_MSF(0u);
        WriteSyntheticPatternU32(FW_NEAR_UNKNOWN_1598, ReadSyntheticPatternU32(FW_NEAR_UNKNOWN_1598) + 1ul);
        main_data_cursor = SYNTHETIC_ADVANCE_RING(main_data_cursor);
        ++start_frame_position;
    }

    CLEAR_SYNTHETIC_ACTIVE_BITS();
    return 1u;
}
