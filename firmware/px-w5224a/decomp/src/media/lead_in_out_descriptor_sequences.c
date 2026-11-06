#include "lead_in_out_descriptor_sequences.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "lead_in_raw_sector_descriptor_sequence.h"
#include "lead_out_raw_sector_descriptor_sequence.h"
#include "raw_read_pipeline_reset.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/lead_in_out_descriptor_sequences_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadLeadInOutU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadLeadInOutU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteLeadInOutU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteLeadInOutU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 ReportLeadInOutWaitFailure(u8 failure_code)
{
    if(ReadLeadInOutU16(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        LEAD_IO_QUEUE_SENSE();
    else
        LEAD_IO_FAILURE(failure_code);
    return 0u;
}

/* RAM:fe4f4e */
u8 AcquireLeadInAndLeadOutRawSectorDescriptorSequences(void)
{
    u8 retry_count = 0u;
    u8 retry = 0u;
    u8 result;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) != 1u)
        LEAD_IO_RESET();
    LEAD_IO_CLEAR_BIT0035();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1282, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1283, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
    {
        do
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
            result = LEAD_IO_ACQUIRE_IN(1u, retry);
            if(result != 0u || retry_count >= 9u || FirmwareReadNear8(FW_NEAR_UNKNOWN_88BA) == 0u)
                break;
            retry = 1u;
            ++retry_count;
        } while(1);
        if(result == 0u)
        {
            LEAD_IO_FAILURE(4u);
            return 0u;
        }
        LEAD_IO_SET_SENSE(0u, 0u);
    }
    else
    {
        result = 0u;
        if((s32)ReadLeadInOutU32(FW_NEAR_UNKNOWN_DD69) >= 0l)
        {
            do
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
                result = LEAD_IO_ACQUIRE_IN(0u, retry);
                if(result != 0u || retry_count >= 9u || FirmwareReadNear8(FW_NEAR_UNKNOWN_88BA) == 0u)
                    break;
                retry = 1u;
                if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) != 0u)
                    break;
                ++retry_count;
            } while(1);
        }
        if(result == 0u)
        {
            LEAD_IO_FAILURE(4u);
            return 0u;
        }
        LEAD_IO_SET_NO_SENSE();
    }

    if(LEAD_IO_WAIT() == 0u)
        return ReportLeadInOutWaitFailure(4u);

    WriteLeadInOutU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteLeadInOutU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteLeadInOutU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteLeadInOutU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteLeadInOutU16(FW_NEAR_UNKNOWN_129A, (u16)(ReadLeadInOutU32(FW_NEAR_UNKNOWN_156A) - 1ul));
    WriteLeadInOutU16(FW_NEAR_UNKNOWN_129C, (u16)(ReadLeadInOutU32(FW_NEAR_UNKNOWN_156A) - 6ul));
    WriteLeadInOutU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteLeadInOutU32(FW_NEAR_UNKNOWN_1262, 0u);
    WriteLeadInOutU32(FW_NEAR_UNKNOWN_1266, 0u);
    WriteLeadInOutU32(FW_NEAR_UNKNOWN_126A, ReadLeadInOutU32(FW_NEAR_UNKNOWN_1562));
    WriteLeadInOutU32(FW_NEAR_UNKNOWN_126E, 0u);
    LEAD_IO_FILL(0ul, FW_HUGE_UNKNOWN_1B0000, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1281, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);

    retry_count = 0u;
    retry = 0u;
    do
    {
        result = LEAD_IO_ACQUIRE_OUT(retry);
        if(result != 0u || retry_count >= 9u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) != 4u || FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) != 0u
            || FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 2u)
            break;
        retry = 1u;
        ++retry_count;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1490, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_127D, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) > 0x10u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 0x10u);
    } while(1);

    if(result == 0u)
    {
        LEAD_IO_FAILURE(5u);
        return 0u;
    }
    LEAD_IO_SET_NO_SENSE();
    if(LEAD_IO_WAIT() == 0u)
        return ReportLeadInOutWaitFailure(5u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) & 0xc0u) != 0x40u
        && ReadLeadInOutU32(FW_NEAR_UNKNOWN_15F2) < ReadLeadInOutU32(FW_NEAR_CURRENT_FRAME_POSITION))
        (void)LEAD_IO_RANGE((s32)ReadLeadInOutU32(FW_NEAR_UNKNOWN_15F2), 4500ul, 2u, retry_count);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B5, 1u);
    return 1u;
}
