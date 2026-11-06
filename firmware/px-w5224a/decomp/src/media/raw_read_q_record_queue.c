#include "raw_read_q_record_queue.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "raw_read_q_record_table_build.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_q_record_queue_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadQRecordQueueU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadQRecordQueueU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteQRecordQueueU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteQRecordQueueU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fe3e0d */
u8 InitializeAndBuildRawReadQRecordQueue(void)
{
    u8 attempt_count = 0u;
    u8 acquisition_complete = 0u;
    u8 write_type_two_shortfall = 0u;
    WriteQRecordQueueU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteQRecordQueueU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteQRecordQueueU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteQRecordQueueU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteQRecordQueueU16(FW_NEAR_UNKNOWN_129A, (u16)(ReadQRecordQueueU32(FW_NEAR_UNKNOWN_156A) - 1ul));
    WriteQRecordQueueU16(FW_NEAR_UNKNOWN_129C, (u16)(ReadQRecordQueueU32(FW_NEAR_UNKNOWN_156A) - 6ul));
    WriteQRecordQueueU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteQRecordQueueU32(FW_NEAR_UNKNOWN_1262, 0u);
    WriteQRecordQueueU32(FW_NEAR_UNKNOWN_1266, 0u);
    WriteQRecordQueueU32(FW_NEAR_UNKNOWN_126A, ReadQRecordQueueU32(FW_NEAR_UNKNOWN_1562));
    WriteQRecordQueueU32(FW_NEAR_UNKNOWN_126E, 0u);
    Q_RECORD_QUEUE_FILL(0ul, 0x0000b7c0ul, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);

build_or_finish:
    if(attempt_count >= 2u || acquisition_complete == 1u)
        goto finish;
    if(Q_RECORD_QUEUE_BUILD() == 0u)
        return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u);

wait_or_finish:
    if(attempt_count >= 2u || acquisition_complete == 1u)
        goto finish;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    Q_RECORD_QUEUE_WAIT();
    ++attempt_count;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 4u)
    {
        if(ReadQRecordQueueU16(FW_NEAR_UNKNOWN_1292) < (u16)(ReadQRecordQueueU16(FW_NEAR_UNKNOWN_8010) - 1u))
        {
            write_type_two_shortfall = 1u;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
                return 1u;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
    {
        acquisition_complete = 1u;
        goto finish;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 3u)
    {
        Q_RECORD_QUEUE_REPORT(3u);
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
    {
        acquisition_complete = 1u;
        goto finish;
    }

    if(ReadQRecordQueueU16(FW_NEAR_UNKNOWN_8010) > ReadQRecordQueueU16(FW_NEAR_UNKNOWN_1292))
    {
        if((u16)(ReadQRecordQueueU16(FW_NEAR_UNKNOWN_8010) - 1u) == ReadQRecordQueueU16(FW_NEAR_UNKNOWN_1292))
        {
            if(Q_RECORD_QUEUE_RESTORE(1u) == 0u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
                    return 1u;
                Q_RECORD_QUEUE_REPORT(3u);
                return 0u;
            }
        }
        else
        {
            if(Q_RECORD_QUEUE_DISCARD(1u) == 0u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
                    return 1u;
                Q_RECORD_QUEUE_REPORT(3u);
                return 0u;
            }
            if(Q_RECORD_QUEUE_RESTORE(1u) == 0u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
                    return 1u;
                Q_RECORD_QUEUE_REPORT(3u);
                return 0u;
            }
            if(write_type_two_shortfall == 1u)
                goto build_or_finish;
        }
    }
    goto wait_or_finish;

finish:
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        return 1u;
    Q_RECORD_QUEUE_REPORT(3u);
    return 0u;
}
