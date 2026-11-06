#include "raw_read_q_record_sector_acquisition.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/raw_sector_capture.h"
#include "../platform/status_mapping.h"
#include "raw_read_frame_sequence.h"
#include "rid01_identity_block.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_q_record_sector_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadQRecordSectorU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteQRecordSectorU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void PublishQRecordForSectorGroup(u32 record_index)
{
    u16 source = (u16)(0x8012u + (u16)Q_RECORD_MULTIPLY(10ul, record_index));
    u8 index;

    for(index = 0u; index != 10u; ++index)
        FirmwareWrite8(FW_HUGE_UNKNOWN_001272 + index, FirmwareReadNear8((u16)(source + index)));
}

static void ReportQRecordSectorCaptureFailure(void)
{
    Q_RECORD_SET_SENSE(4u, 0x4400u);
    Q_RECORD_SET_INFORMATION(ReadQRecordSectorU32(FW_NEAR_UNKNOWN_1598));
}

/* RAM:fe9821 */
u8 PLEXTOR_NEAR_FUNCTION AcquireRawSectorsFromQRecordTable(u32 expected_sector_count, u16 q_record_count)
{
    fw_addr_t main_data_cursor = 0ul;
    u32 record_index;
    u32 record_count;
    volatile u16 remaining_q_record_count = q_record_count;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_q_record_sector_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if(expected_sector_count == 0ul)
        return 1u;

    WriteQRecordSectorU32(FW_NEAR_UNKNOWN_1598, ReadQRecordSectorU32(FW_NEAR_UNKNOWN_114F) - 1000ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    Q_RECORD_CONVERT((s32)ReadQRecordSectorU32(FW_NEAR_UNKNOWN_1598), tuple_address);
    Q_RECORD_PACK(tuple_address);
    Q_RECORD_COPY_TUPLE(FW_NEAR_UNKNOWN_1428, tuple_address);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 1u);

    record_count = Q_RECORD_DIVIDE(expected_sector_count, 10ul);
    for(record_index = 0ul; record_index < record_count; ++record_index)
    {
        u32 sector_index;

        remaining_q_record_count = (u16)(remaining_q_record_count - 1u);
        PublishQRecordForSectorGroup(record_index);

        for(sector_index = 0ul; sector_index < 10ul; ++sector_index)
        {
            u32 ring_span;

            if(Q_RECORD_WAIT_CAPACITY() == 0u)
            {
                Q_RECORD_REPORT_FAILURE(3u);
                return 0u;
            }
            Q_RECORD_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            if(Q_RECORD_CAPTURE(main_data_cursor, 10u) == 0u)
            {
                ReportQRecordSectorCaptureFailure();
                return 0u;
            }
            Q_RECORD_BUILD_RID01(main_data_cursor + 0x10ul);
            Q_RECORD_INCREMENT(FW_NEAR_UNKNOWN_1428);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, (u8)sector_index);
            WriteQRecordSectorU32(FW_NEAR_UNKNOWN_126E, main_data_cursor);
            Q_RECORD_UPDATE_Q();
            Q_RECORD_ENQUEUE();
            WriteQRecordSectorU32(FW_NEAR_UNKNOWN_1598, ReadQRecordSectorU32(FW_NEAR_UNKNOWN_1598) + 1ul);

            ring_span = Q_RECORD_MULTIPLY(0x930ul, ReadQRecordSectorU32(FW_NEAR_UNKNOWN_156A));
            if(main_data_cursor + 0x930ul > ring_span - 1ul)
                main_data_cursor = 0ul;
            else
                main_data_cursor += 0x930ul;
        }
    }

    if(Q_RECORD_CAPTURE(main_data_cursor, 6u) == 0u)
    {
        ReportQRecordSectorCaptureFailure();
        return 0u;
    }
    Q_RECORD_INCREMENT(FW_NEAR_UNKNOWN_1428);
    return 1u;
}
