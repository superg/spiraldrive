#include "write_pipeline_finalize.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"
#include "lead_out_raw_sector_descriptor_sequence.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/write_pipeline_finalize_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadFinalizeWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static s32 ReadFinalizeSignedDword(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static void WriteFinalizeDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fe5dd0 */
u8 PLEXTOR_NEAR_FUNCTION FinalizeWriteCommandRawReadPipelinePreparation(void)
{
    u8 write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 0u)
    {
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) != 1u)
            WriteFinalizeDword(FW_NEAR_UNKNOWN_8000, (u32)(ReadFinalizeSignedDword(FW_NEAR_UNKNOWN_1598) - 1l));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 1u);
    }
    else if(write_type == 1u || write_type == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 1u);
    }
    else if(write_type == 2u)
    {
        u8 target_tuple[3];
        u8 lead_out_tuple_source[3];
        u8 lead_out_tuple[3];
        u16 record_address;
        u8 restart_after_failure;
        u8 retry_count;
        u8 success;

        if(ReadFinalizeWord(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        {
            FINALIZE_REPORT_SENSE_030C09();
            return 0u;
        }

        if(FINALIZE_ISRC_AVAILABLE() == 1u)
            FINALIZE_LOAD_ISRC();

        FINALIZE_CONVERT_FRAME(ReadFinalizeSignedDword(FW_NEAR_UNKNOWN_1598), target_tuple);
        record_address = (u16)(0xb4c1u + (u16)(ReadFinalizeWord(FW_NEAR_UNKNOWN_149F) * 8u));
        lead_out_tuple_source[0] = FirmwareReadNear8(record_address);
        lead_out_tuple_source[1] = FirmwareReadNear8((u16)(record_address + 2u));
        lead_out_tuple_source[2] = FirmwareReadNear8((u16)(record_address + 3u));
        FINALIZE_COPY_TUPLE(lead_out_tuple, lead_out_tuple_source);

        if(FINALIZE_COMPARE_TUPLES(target_tuple, lead_out_tuple) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1554, 1u);
            return 1u;
        }

        restart_after_failure = 0u;
        retry_count = 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 3u);
        for(;;)
        {
            success = FINALIZE_ACQUIRE_LEAD_OUT(restart_after_failure);
            if(success != 0u)
                break;
            if(retry_count >= 9u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 4u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 2u)
            {
                FINALIZE_REPORT_FAILURE(5u);
                return 0u;
            }

            restart_after_failure = 1u;
            retry_count++;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) > 0x10u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x10u);
        }
        FINALIZE_SET_NO_SENSE();
    }

    return 1u;
}
