#include "read_master_cue_response.h"
#include "../platform/controller_buffer_atomic.h"
#include "../platform/controller_data_out.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "read_master_cue_status.h"

extern void YieldCoroutine0(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_master_cue_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadMasterCueU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadMasterCueU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteMasterCueU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteMasterCueU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fc62ac */
void ExecuteReadMasterCueResponse(void)
{
    u8 status;
    u16 saved_secondary_cursor;
    u16 descriptor_index;
    u16 descriptor_address;
    fw_addr_t controller_buffer_address;
    u32 remaining;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_master_cue_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    CLEAR_READ_MASTER_CUE_ENTRY_BITS();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        SetSenseData(5u, 0x6400u);
        SetSenseInformation(ReadMasterCueU32(FW_NEAR_UNKNOWN_0B02));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    SET_READ_MASTER_CUE_PINS(0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 12u)
    {
        status = 1u;
    }
    else
    {
        WriteMasterCueU32(FW_NEAR_UNKNOWN_0F0C, 0ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F09, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F08, 0x32u);
        CLEAR_READ_MASTER_CUE_MODE_BITS();
        ResetRawReadQueueAndTransferState();
        ENABLE_READ_MASTER_CUE_ACQUISITION();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
        InitializeCoroutine2(FW_ENTRY_RUN_PRIMARY_RAW_READ_PIPELINE_COROUTINE2);
        WriteMasterCueU32(FW_NEAR_UNKNOWN_1598, ResetRawReadTransferState());
        YieldCoroutine0();
        do
        {
            status = PollReadMasterCueRawReadStatus();
            if(status == 0xfeu)
                YieldCoroutine0();
        } while(status == 0xfeu);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
    SET_READ_MASTER_CUE_PINS(1u);
    if(status != 1u)
    {
        ResetRawReadQueueAndTransferState();
        InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        DispatchReadAcquisitionRequest5ForPipelineTarget();
        YieldCoroutine0();
        return;
    }

    saved_secondary_cursor = ReadMasterCueU16(FW_NEAR_UNKNOWN_1298);
    WriteMasterCueU16(FW_NEAR_UNKNOWN_1298, 0u);
    descriptor_index = 0u;
    for(;;)
    {
        u8 prefix0;
        u8 prefix1;
        u8 prefix3;

        if(descriptor_index >= ReadMasterCueU16(FW_NEAR_UNKNOWN_1292))
        {
            WriteMasterCueU16(FW_NEAR_UNKNOWN_1298, saved_secondary_cursor);
            SetSenseData(5u, 0x6400u);
            return;
        }

        descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
        WriteMasterCueU16(FW_NEAR_UNKNOWN_1298, (u16)((ReadMasterCueU16(FW_NEAR_UNKNOWN_1298) + 1u) & ReadMasterCueU16(FW_NEAR_UNKNOWN_129A)));
        controller_buffer_address = ReadMasterCueU32((fw_addr_t)(u16)(descriptor_address + 4u));
        prefix0 = ReadControllerBufferByteAtomic(FirmwareAddressIncrement24(controller_buffer_address, 0x0cul));
        prefix1 = ReadControllerBufferByteAtomic(FirmwareAddressIncrement24(controller_buffer_address, 0x0dul));
        (void)ReadControllerBufferByteAtomic(FirmwareAddressIncrement24(controller_buffer_address, 0x0eul));
        prefix3 = ReadControllerBufferByteAtomic(FirmwareAddressIncrement24(controller_buffer_address, 0x0ful));
        if(((prefix0 >= 0xaau && prefix0 <= 0xacu) || prefix3 == 0u) && prefix1 == FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E))
            break;
        descriptor_index++;
    }

    controller_buffer_address = FirmwareAddressIncrement24(controller_buffer_address, 0x10u);
    FirmwareWrite8(response_prefix_address, 0u);
    FirmwareWrite8(response_prefix_address + 1u, 0u);
    FirmwareWrite8(response_prefix_address + 2u, 0u);
    FirmwareWrite8(response_prefix_address + 3u, 0u);
    (void)CopyResponseToControllerBufferAndExecuteDataIn(response_prefix_address, 4ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 12u);
    remaining = ReadMasterCueU32(FW_NEAR_UNKNOWN_0B06) - 4ul;
    WriteMasterCueU32(FW_NEAR_UNKNOWN_0B06, remaining);

    while(remaining > 0x800ul)
    {
        ExecuteControllerDataInTransfer(controller_buffer_address, 0x800ul);
        controller_buffer_address = FirmwareAddressIncrement24(controller_buffer_address, 0x930u);
        remaining -= 0x800ul;
        WriteMasterCueU32(FW_NEAR_UNKNOWN_0B06, remaining);
        if(ReadControllerBufferByteAtomic(FirmwareAddressIncrement24(controller_buffer_address, FW_ROM_UNKNOWN_FFFFFD)) != FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E))
            break;
    }
    if(ReadControllerBufferByteAtomic(FirmwareAddressIncrement24(controller_buffer_address, FW_ROM_UNKNOWN_FFFFFD)) == FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E))
        ExecuteControllerDataInTransfer(controller_buffer_address, remaining);

    WriteMasterCueU16(FW_NEAR_UNKNOWN_1298, saved_secondary_cursor);
    SetNoSenseData();
}
