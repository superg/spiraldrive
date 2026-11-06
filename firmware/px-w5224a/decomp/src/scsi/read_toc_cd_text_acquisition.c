#include "read_toc_cd_text_acquisition.h"
#include "../platform/controller_data_out.h"
#include "../platform/coroutine_initialization.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "read_toc_format5_response.h"

extern void YieldCoroutine0(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_toc_cd_text_acquisition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCdTextU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadCdTextU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteCdTextU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteCdTextU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 IsCdTextAcquisitionAllowed(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0x13u) == 2u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
        return 0u;
    return 1u;
}

/* RAM:fd74cb */
void ExecuteReadTocFormat5CdTextResponse(u16 allocation_length)
{
    u16 previous_response_length;
    u16 response_length;
    u16 expected_length;
    u16 descriptor_address;
    u8 nonzero_descriptor_count;
    u8 old_attempt;
    u8 attempt;
    u8 stop;
    u8 index;
    s8 status;

    if(IsCdTextAcquisitionAllowed() == 0u)
    {
        ReturnEmptyCdTextResponse(allocation_length);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    WRITE_CD_TEXT_P3_RX(0u);
    WRITE_CD_TEXT_P3_TX(0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 10u)
    {
        status = 1;
    }
    else
    {
        WriteCdTextU32(FW_NEAR_UNKNOWN_0F52, 0ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15EE, 0u);
        status = 0;
        nonzero_descriptor_count = 0u;
        previous_response_length = 0xffffu;
        for(index = 0u; index < 5u; ++index)
            WriteCdTextU16(FW_NEAR_UNKNOWN_F58C + (u16)index * 2u, 0xffffu);
        WRITE_CD_TEXT_BIT_001E(0u);

        for(;;)
        {
            old_attempt = FirmwareReadNear8(FW_NEAR_UNKNOWN_15EE);
            attempt = (u8)(old_attempt + 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15EE, attempt);
            if(old_attempt >= 14u)
                break;

            ResetRawReadQueueAndTransferState();
            InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
            InitializeCdTextRawReadAcquisition(attempt);
            InitializeCoroutine2(FW_ENTRY_RUN_ALTERNATE_RAW_READ_PIPELINE_COROUTINE2);
            WriteCdTextU32(FW_NEAR_UNKNOWN_1598, (u32)(s32)-999);
            do
            {
                YieldCoroutine0();
                status = ValidateLatestCdTextRawSectorPackBatch();
            } while(status == (s8)-2);

            stop = 0u;
            if((u8)status == 0x63u)
            {
                descriptor_address = GetLatestQueuedRawSectorDescriptorAddress();
                if(ReadCdTextU16((u16)(descriptor_address + 0x0eu)) != 0u)
                {
                    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 0x13u) == 2u)
                    {
                        stop = 1u;
                    }
                    else
                    {
                        nonzero_descriptor_count++;
                        if(nonzero_descriptor_count > 2u)
                            stop = 1u;
                    }
                }
            }
            else if(status == 1)
            {
                nonzero_descriptor_count = 0u;
                response_length = ReadCdTextU16(FW_NEAR_UNKNOWN_0F50);
                expected_length = (u16)((u32)(u16)(ReadCdTextU16(FW_NEAR_UNKNOWN_F798) + ReadCdTextU16(FW_NEAR_UNKNOWN_F796)) * 0x12ul + 4ul);
                if((expected_length == response_length && response_length != 4u) || (attempt > 7u && previous_response_length == response_length) || attempt > 12u)
                    stop = 1u;
                else
                    previous_response_length = response_length;
            }
            else if(attempt > 1u)
            {
                stop = 1u;
            }

            if(stop != 0u)
                break;
            WriteCdTextU32(FW_NEAR_UNKNOWN_0F52, ReadCdTextU32(FW_NEAR_UNKNOWN_0F52) + 75ul);
        }

        if((u8)status == 0x63u)
            ReturnEmptyCdTextResponse(allocation_length);
        WriteCdTextU32(FW_NEAR_UNKNOWN_0F52, 0ul);
    }

    if(status == 1)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 10u);
        response_length = ReadCdTextU16(FW_NEAR_UNKNOWN_0F50);
        if(allocation_length > response_length)
            allocation_length = response_length;
        ExecuteControllerDataInTransfer(ReadCdTextU32(FW_NEAR_UNKNOWN_1556) - 0x9000ul, allocation_length);
        SetNoSenseData();
    }

    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    WRITE_CD_TEXT_BIT_0013(0u);
    WriteCdTextU32(FW_NEAR_UNKNOWN_1598, 150ul);
    DispatchReadAcquisitionRequest5ForPipelineTarget();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F06, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    WRITE_CD_TEXT_P3_RX(1u);
    WRITE_CD_TEXT_P3_TX(1u);
}
