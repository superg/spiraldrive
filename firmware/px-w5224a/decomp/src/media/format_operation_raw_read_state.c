#include "format_operation_raw_read_state.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "../platform/track_state.h"
#include "raw_read_acquisition_plan.h"
#include "raw_read_pipeline_reset.h"
#include "raw_read_q_record_queue.h"
#include "raw_read_range_five_frame_preroll.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/format_operation_raw_read_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadFormatRawU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadFormatRawU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteFormatRawU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteFormatRawU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void ClearFormatRawActiveState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 0u);
}

static u8 ExecuteFormatRawTenRecordSequence(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 1u);
    WriteFormatRawU16(FW_NEAR_UNKNOWN_8010, 10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    if(FORMAT_RAW_EXECUTE_Q(1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 0u);
        ClearFormatRawActiveState();
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B9, 0u);
    return 1u;
}

/* RAM:fe85ff */
u8 PrepareFormatOperationRawReadState(void)
{
    s32 range_start;
    u32 range_count;
    u32 quotient;

    FORMAT_RAW_RESET();
    WriteFormatRawU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteFormatRawU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteFormatRawU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteFormatRawU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteFormatRawU16(FW_NEAR_UNKNOWN_129A, (u16)(ReadFormatRawU32(FW_NEAR_UNKNOWN_156A) - 1ul));
    WriteFormatRawU16(FW_NEAR_UNKNOWN_129C, (u16)(ReadFormatRawU32(FW_NEAR_UNKNOWN_156A) - 6ul));
    WriteFormatRawU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteFormatRawU32(FW_NEAR_UNKNOWN_1262, 0u);
    WriteFormatRawU32(FW_NEAR_UNKNOWN_1266, 0u);
    WriteFormatRawU32(FW_NEAR_UNKNOWN_126A, ReadFormatRawU32(FW_NEAR_UNKNOWN_1562));
    WriteFormatRawU32(FW_NEAR_UNKNOWN_126E, 0u);
    FORMAT_RAW_INIT_CONTROL();
    FORMAT_RAW_CLEAR_STATUS();
    WriteFormatRawU32(FW_NEAR_UNKNOWN_1488, 0u);
    WriteFormatRawU32(FW_NEAR_UNKNOWN_148C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B7, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u)
            range_start = 150l;
        else
            range_start = (s32)(ReadFormatRawU32(FW_NEAR_UNKNOWN_E616) + 151ul);
        WriteFormatRawU32(FW_NEAR_UNKNOWN_1598, (u32)range_start);
        if(FORMAT_RAW_PREPARE_PLAN(0u) == 0u)
        {
            ClearFormatRawActiveState();
            return 0u;
        }
        if(FORMAT_RAW_POLL(1u) == 0u)
        {
            if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) == 1u)
                FORMAT_RAW_REPORT(8u);
            else if(ReadFormatRawU16(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
                FORMAT_RAW_REPORT_SENSE();
            else
                FORMAT_RAW_REPORT(0u);
            ClearFormatRawActiveState();
            return 0u;
        }
        quotient = FORMAT_RAW_DIVIDE(ReadFormatRawU32(FW_NEAR_UNKNOWN_E5FC), ReadFormatRawU32(FW_NEAR_UNKNOWN_0C4B));
        range_count = FORMAT_RAW_MULTIPLY(ReadFormatRawU32(FW_NEAR_UNKNOWN_0C4B) + 7ul, quotient) - 5ul;
    }
    else
    {
        range_start = (s32)(ReadFormatRawU32(FW_NEAR_UNKNOWN_DD41) + 5ul);
        quotient = FORMAT_RAW_DIVIDE(ReadFormatRawU32(FW_NEAR_UNKNOWN_E5FC), ReadFormatRawU32(FW_NEAR_UNKNOWN_0C4B));
        range_count = FORMAT_RAW_MULTIPLY(ReadFormatRawU32(FW_NEAR_UNKNOWN_0C4B) + 7ul, quotient) - 5ul;
        range_count -= ReadFormatRawU32(FW_NEAR_UNKNOWN_DD41) - (ReadFormatRawU32(FW_NEAR_UNKNOWN_DD3D) - 5ul);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));
    if(FORMAT_RAW_ACQUIRE_PREROLL(range_start, range_count) == 0u)
    {
        ClearFormatRawActiveState();
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 1u)
    {
        if(ExecuteFormatRawTenRecordSequence() == 0u)
            return 0u;
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 1u)
    {
        if(FORMAT_RAW_INIT_Q_QUEUE() == 0u)
        {
            ClearFormatRawActiveState();
            return 0u;
        }
    }
    else if(ExecuteFormatRawTenRecordSequence() == 0u)
    {
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));
    if(FORMAT_RAW_ACQUIRE_LEAD() == 0u)
    {
        ClearFormatRawActiveState();
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) == 0xc0u)
    {
        range_start = (s32)(ReadFormatRawU32(FW_NEAR_UNKNOWN_E61E) + 1ul);
        range_count = ReadFormatRawU32(FW_NEAR_CURRENT_FRAME_POSITION) - ReadFormatRawU32(FW_NEAR_UNKNOWN_E61E) + 0x08cbul;
        if(FORMAT_RAW_ACQUIRE_MODE2(range_start, range_count) == 0u)
        {
            ClearFormatRawActiveState();
            return 0u;
        }
    }

    ClearFormatRawActiveState();
    return 1u;
}
