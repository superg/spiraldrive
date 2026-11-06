#include "raw_sector_range.h"
#include "../media/raw_read_runtime.h"
#include "../platform/coroutine_initialization.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "raw_sector_host_transfer_poll.h"
#include "vendor_raw_read.h"

extern void YieldCoroutine0(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/raw_sector_range_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRangeU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadRangeU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteRangeU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteRangeU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void RestartRawSectorReadAcquisition(void)
{
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    DispatchReadAcquisitionRequest5ForPipelineTarget();
    YieldCoroutine0();
}

/* RAM:fd6cf2 */
u8 ExecuteRawSectorReadRange(s32 start_frame_position, s32 end_frame_position_exclusive, u8 format_code)
{
    s32 previous_end_delta;
    s32 pipeline_target;
    s32 failure_position;
    u16 transfer_length;
    u16 descriptor_address;
    u16 linked_address;
    u8 status;

    WriteRangeU32(FW_NEAR_UNKNOWN_0B02, (u32)start_frame_position - 150ul);
    previous_end_delta = (s32)((u32)start_frame_position - ReadRangeU32(FW_NEAR_UNKNOWN_0F14));
    WriteRangeU32(FW_NEAR_UNKNOWN_0F14, (u32)end_frame_position_exclusive);
    WRITE_RANGE_BIT_001D(0u);
    WRITE_RANGE_BIT_001C(0u);
    WRITE_RANGE_BIT_001B(0u);
    if(format_code == 8u)
        format_code = 4u;

    if(READ_RANGE_BIT_000D() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F09, 0u);
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0A) > 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F09, 0u);
            SetSenseData(5u, 0x6400u);
            return 1u;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0A) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F09) > 1u && previous_end_delta <= 4 && previous_end_delta >= -4 && previous_end_delta != 0)
        {
            SetSenseData(3u, 0x1500u);
            return 1u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F09, 1u);
    }

    if(READ_RANGE_BIT_0013() != 1u)
    {
        if(start_frame_position < 75)
        {
            if(start_frame_position > -1000 || start_frame_position <= -20000)
            {
                SetSenseData(5u, 0x6400u);
                return 1u;
            }
            WRITE_RANGE_BIT_0013(1u);
        }
        else if(ValidateRawSectorReadFrameRangeAgainstMediaBounds(start_frame_position, end_frame_position_exclusive) == 0u)
        {
            return 1u;
        }
    }

    if(start_frame_position == end_frame_position_exclusive)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4A, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C8C, 0u);
        WriteRangeU32(FW_NEAR_UNKNOWN_0C8E, (u32)start_frame_position);
        (void)DispatchAndWaitForReadAcquisitionResult();
        return 1u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 0u);
    WRITE_RANGE_P3_RX(0u);
    WRITE_RANGE_P3_TX(0u);
    WriteRangeU32(FW_NEAR_UNKNOWN_1455, (u32)end_frame_position_exclusive - (u32)start_frame_position);
    UpdateRawReadProgressCountersAndPhaseFlags();
    WRITE_RANGE_BIT_000C(0u);
    pipeline_target = (s32)((u32)start_frame_position + ReadRangeU32(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF95CE) + (u32)format_code * 4ul));
    WriteRangeU32(FW_NEAR_UNKNOWN_0F32, (u32)start_frame_position);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 3u && format_code == FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06))
    {
        POSITION_RAW_READ_PIPELINE(pipeline_target);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
        ResetRawReadQueueAndTransferState();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, READ_RANGE_BIT_000E() == 1u ? 7u : 3u);
        InitializeCoroutine2(FW_ENTRY_RUN_ALTERNATE_RAW_READ_PIPELINE_COROUTINE2);
        WriteRangeU32(FW_NEAR_UNKNOWN_1598, (u32)pipeline_target);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F06, format_code);
        YieldCoroutine0();
    }

    WriteRangeU32(FW_NEAR_UNKNOWN_1459, 0ul);
    if(READ_RANGE_BIT_000D() == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 0xfau) != 0u)
    {
        transfer_length = ReadRangeU16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF95BA) + (u32)format_code * 2ul);
        WRITE_RANGE_BIT_0045(0u);
    }
    else
    {
        transfer_length = ReadRangeU16(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF95C4) + (u32)format_code * 2ul);
        if(transfer_length == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) == 0u)
        {
            SetNoSenseData();
            return 1u;
        }
        WRITE_RANGE_BIT_0045(1u);
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != 0u)
        transfer_length = (u16)(transfer_length + 0x0126u);
    WriteRangeU16(FW_NEAR_UNKNOWN_145D, transfer_length);
    WRITE_RANGE_BIT_0001(0u);

    do
    {
        status = PollRawSectorReadHostTransferStatus();
        if(status == 0xfeu)
            YieldCoroutine0();
    } while(status == 0xfeu);

    WRITE_RANGE_BIT_0045(0u);
    if(READ_RANGE_BIT_000E() != 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1226, 4u);
        WRITE_RANGE_P3_RX(1u);
        WRITE_RANGE_P3_TX(1u);
    }
    if(status != 0u)
    {
        SetNoSenseData();
        return 1u;
    }

    descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
    linked_address = ReadRangeU16(FW_NEAR_UNKNOWN_1457);
    failure_position = (s32)(((u32)ReadRangeU16(linked_address) << 16) | 0x1455ul);
    if(failure_position < 0 || (s32)ReadRangeU32(FW_NEAR_UNKNOWN_12C1) <= failure_position)
    {
        SetSenseData(FirmwareRead8((fw_addr_t)ReadRangeU16((fw_addr_t)(u16)(descriptor_address + 0x0eu)) + 0x10ul), descriptor_address);
        SetSenseInformation((u32)failure_position - 150ul);
    }
    RestartRawSectorReadAcquisition();
    return 0u;
}
