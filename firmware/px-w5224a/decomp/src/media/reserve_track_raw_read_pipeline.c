#include "reserve_track_raw_read_pipeline.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "../scsi/send_opc_information.h"
#include "raw_read_acquisition_plan.h"
#include "raw_read_pipeline_reset.h"
#include "raw_read_q_record_queue.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/reserve_track_raw_read_pipeline_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadReserveRawU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadReserveRawU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteReserveRawU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteReserveRawU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void ResetReserveTrackRawReadRing(void)
{
    WriteReserveRawU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteReserveRawU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteReserveRawU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteReserveRawU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteReserveRawU16(FW_NEAR_UNKNOWN_129A, (u16)(ReadReserveRawU32(FW_NEAR_UNKNOWN_156A) - 1ul));
    WriteReserveRawU16(FW_NEAR_UNKNOWN_129C, (u16)(ReadReserveRawU32(FW_NEAR_UNKNOWN_156A) - 6ul));
    WriteReserveRawU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteReserveRawU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteReserveRawU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteReserveRawU32(FW_NEAR_UNKNOWN_126A, ReadReserveRawU32(FW_NEAR_UNKNOWN_1562));
    WriteReserveRawU32(FW_NEAR_UNKNOWN_126E, 0ul);
}

static u8 FailReserveTrackRawReadPipeline(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128B, 0u);
    return 0u;
}

/* RAM:fe577e */
u8 PrepareReserveTrackRawReadPipeline(void)
{
    u8 write_type;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/reserve_track_raw_read_pipeline_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    RESERVE_RAW_RESET();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 0u);
    ResetReserveTrackRawReadRing();
    RESERVE_RAW_FILL(0ul, FW_HUGE_UNKNOWN_1B0000, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128B, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B4, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1286, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 12u)
    {
        if(RESERVE_RAW_INIT_QUEUE() == 0u)
            return FailReserveTrackRawReadPipeline();
    }
    else if(RESERVE_RAW_INIT_QUEUE() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && RESERVE_RAW_RUN_OPC() == 0u)
        {
            RESERVE_RAW_SET_SENSE(3u, 0x7304u);
            return FailReserveTrackRawReadPipeline();
        }
        if(RESERVE_RAW_INIT_QUEUE() == 0u)
            return FailReserveTrackRawReadPipeline();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
        return 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128B, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 1u);
    FirmwareWrite8(tuple_address, FirmwareRead8(FW_HUGE_UNKNOWN_2BC122));
    FirmwareWrite8(tuple_address + 1u, FirmwareRead8(FW_HUGE_UNKNOWN_2BC123));
    FirmwareWrite8(tuple_address + 2u, FirmwareRead8(FW_HUGE_UNKNOWN_2BC124));
    RESERVE_RAW_CONVERT_BCD((u16)tuple_address);
    WriteReserveRawU32(FW_NEAR_UNKNOWN_1598, (u32)RESERVE_RAW_CONVERT_MSF(tuple_address));

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type != 0u && write_type != 1u)
    {
        RESERVE_RAW_SEQUENCE_ERROR();
        return FailReserveTrackRawReadPipeline();
    }

    ResetReserveTrackRawReadRing();
    if(RESERVE_RAW_PREPARE_PLAN(0u) == 0u)
        return FailReserveTrackRawReadPipeline();
    if(RESERVE_RAW_POLL(1u) == 0u)
    {
        if(ReadReserveRawU16(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
            RESERVE_RAW_QUEUE_SENSE();
        else
            RESERVE_RAW_FAILURE(6u);
        return FailReserveTrackRawReadPipeline();
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128B, 0u);
    return 1u;
}
