#include "raw_read_range_five_frame_preroll.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_read_q_control.h"
#include "raw_read_acquisition_plan.h"
#include "raw_read_frame_prefix_wait.h"
#include "raw_read_frame_sequence.h"
#include "raw_read_runtime.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_range_five_frame_preroll_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadFiveFrameU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteFiveFrameU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fe8871 */
u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadRangeWithFiveFramePreroll(s32 start_frame_address, u32 frame_count)
{
    u32 end_frame_address = (u32)start_frame_address + frame_count;
    volatile u16 retry_count = 0u;

    WriteFiveFrameU32(FW_NEAR_UNKNOWN_1598, (u32)start_frame_address - 5ul);

    for(;;)
    {
        u32 retry_origin;
        fw_addr_t main_data_cursor;
        u32 packet_sector_index;
        u8 retry_requested = 0u;

        if(ReadFiveFrameU32(FW_NEAR_UNKNOWN_1598) >= end_frame_address)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
            if(FIVE_FRAME_PREPARE_PREFIX(0u, 0u) == 0u)
                return 0u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);
            return FIVE_FRAME_PREPARE_PREFIX(1u, 1u) != 0u;
        }

        retry_origin = ReadFiveFrameU32(FW_NEAR_UNKNOWN_1598);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B7, 1u);
        if(FIVE_FRAME_PREPARE_PLAN(0u) == 0u)
        {
            ++retry_count;
            if(retry_count >= 10u)
                return 0u;
            WriteFiveFrameU32(FW_NEAR_UNKNOWN_1598, retry_origin);
            continue;
        }

        retry_count = 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B6, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B7, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 0u);
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B2, 0u);
            WriteFiveFrameU32(FW_NEAR_UNKNOWN_14B8, 0u);
        }

        main_data_cursor = FIVE_FRAME_RESUME_RING();
        packet_sector_index = 0u;
        while(packet_sector_index < ReadFiveFrameU32(FW_NEAR_UNKNOWN_0C4B))
        {
            if(FIVE_FRAME_WAIT_CAPACITY() == 0u)
            {
                ++retry_count;
                if(retry_count >= 10u)
                {
                    FIVE_FRAME_REPORT_FAILURE(1u);
                    return 0u;
                }
                WriteFiveFrameU32(FW_NEAR_UNKNOWN_1598, retry_origin);
                retry_requested = 1u;
                break;
            }

            retry_count = 0u;
            if(FIVE_FRAME_CAPTURE(main_data_cursor, 1u, 1u) == 0u)
            {
                ++retry_count;
                if(retry_count >= 10u)
                {
                    FIVE_FRAME_REPORT_FAILURE(1u);
                    return 0u;
                }
                WriteFiveFrameU32(FW_NEAR_UNKNOWN_1598, retry_origin);
                retry_requested = 1u;
                break;
            }

            retry_count = 0u;
            FIVE_FRAME_POPULATE_SUBHEADER(main_data_cursor, 4u);
            FIVE_FRAME_UPDATE_Q(0u, 0u);
            FIVE_FRAME_ENQUEUE();
            FIVE_FRAME_ADVANCE_MSF(0u);
            WriteFiveFrameU32(FW_NEAR_UNKNOWN_1598, ReadFiveFrameU32(FW_NEAR_UNKNOWN_1598) + 1ul);
            main_data_cursor = FIVE_FRAME_ADVANCE_RING(main_data_cursor);
            ++packet_sector_index;
        }

        if(retry_requested != 0u)
            continue;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
        if(FIVE_FRAME_PREPARE_PREFIX(1u, 0u) != 0u)
        {
            retry_count = 0u;
            continue;
        }

        ++retry_count;
        if(retry_count >= 10u)
            return 0u;
        WriteFiveFrameU32(FW_NEAR_UNKNOWN_1598, retry_origin);
    }
}
