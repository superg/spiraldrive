#include "raw_read_interleaved_q_batch.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/raw_read_q_control.h"
#include "../platform/raw_sector_capture.h"
#include "../platform/status_mapping.h"
#include "raw_read_runtime.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_interleaved_q_batch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadInterleavedU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadInterleavedU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteInterleavedU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteInterleavedU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void ClearInterleavedAuxiliarySlot(fw_addr_t main_data_cursor)
{
    fw_addr_t address = FirmwareAddressIncrement24(main_data_cursor, FW_HUGE_BUFFER_BANK_BASE);
    u8 index;

    for(index = 0u; index != 16u; ++index)
        FirmwareWrite8(FirmwareAddressIncrement24(address, index), 0u);
}

static void PublishSelectedInterleavedQRecord(u16 tuple_address)
{
    u16 record_index = ReadInterleavedU16(FW_NEAR_UNKNOWN_14C5);
    u16 source = (u16)(0x8012u + (u16)(record_index * 10u));
    u8 index;

    for(index = 0u; index != 10u; ++index)
        FirmwareWrite8(FW_HUGE_UNKNOWN_001272 + index, FirmwareReadNear8((u16)(source + index)));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x0fu) == 1u)
        INTERLEAVED_COPY_TUPLE(FW_NEAR_UNKNOWN_1275, tuple_address);
}

static void SelectNextInterleavedQRecord(void)
{
    u16 first_range_end = ReadInterleavedU16(FW_NEAR_UNKNOWN_8008);

    WriteInterleavedU16(FW_NEAR_UNKNOWN_14C7, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14C9) == 0u || first_range_end == ReadInterleavedU16(FW_NEAR_UNKNOWN_800A))
    {
        u16 cursor = (u16)(ReadInterleavedU16(FW_NEAR_UNKNOWN_14C1) + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
        if(cursor == first_range_end)
            cursor = 0u;
        WriteInterleavedU16(FW_NEAR_UNKNOWN_14C1, cursor);
        WriteInterleavedU16(FW_NEAR_UNKNOWN_14C5, cursor);
    }
    else
    {
        u16 cursor = ReadInterleavedU16(FW_NEAR_UNKNOWN_14C3);
        u16 next_cursor = (u16)(cursor + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 0u);
        WriteInterleavedU16(FW_NEAR_UNKNOWN_14C5, cursor);
        if(next_cursor == ReadInterleavedU16(FW_NEAR_UNKNOWN_800A))
            next_cursor = first_range_end;
        WriteInterleavedU16(FW_NEAR_UNKNOWN_14C3, next_cursor);
    }
}

/* RAM:fe7ab2 */
u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadFrameBatchWithInterleavedQRecords(u32 frame_count)
{
    fw_addr_t main_data_cursor = INTERLEAVED_RESUME_RING();
    u16 captured = 0u;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_interleaved_q_batch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    INTERLEAVED_CONVERT((s32)ReadInterleavedU32(FW_NEAR_UNKNOWN_1598), tuple_address);
    INTERLEAVED_PACK(tuple_address);

    while((u32)captured < frame_count)
    {
        while(ReadInterleavedU16(FW_NEAR_UNKNOWN_14C7) < 3u && (u32)captured < frame_count)
        {
            (void)INTERLEAVED_CAPTURE(main_data_cursor, 7u);
            ClearInterleavedAuxiliarySlot(main_data_cursor);
            PublishSelectedInterleavedQRecord(tuple_address);
            INTERLEAVED_UPDATE_Q();
            INTERLEAVED_ENQUEUE();
            WriteInterleavedU32(FW_NEAR_UNKNOWN_1598, ReadInterleavedU32(FW_NEAR_UNKNOWN_1598) + 1ul);
            INTERLEAVED_INCREMENT(tuple_address);
            main_data_cursor = INTERLEAVED_ADVANCE_RING(main_data_cursor);
            captured = (u16)(captured + 1u);
            WriteInterleavedU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadInterleavedU16(FW_NEAR_UNKNOWN_14C7) + 1u));
        }
        if(ReadInterleavedU16(FW_NEAR_UNKNOWN_14C7) != 3u)
            break;
        SelectNextInterleavedQRecord();
    }
    return 1u;
}
