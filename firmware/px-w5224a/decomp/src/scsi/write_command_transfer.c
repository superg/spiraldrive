#include "write_command_transfer.h"
#include "../media/write_command_transfer_pipeline.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

extern void YieldCoroutine0(void);
extern u8 ExecuteWriteTransferBlockRange(u32 block_count);
extern u8 PrepareWritePipelineForStartAddress(s32 start_address);

static u32 ReadWriteCommandU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteWriteCommandU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void PublishWriteCommandStartAddress(u32 start_address)
{
    if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x2eu)
        WriteWriteCommandU32(FW_HUGE_UNKNOWN_2BC707, start_address);
}

static void MapWriteCommandStartAddress(u32 start_address)
{
    u32 mapped_address;

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) < 0)
        mapped_address = MapFixedPacketLogicalToPhysicalAddress(start_address);
    else
        mapped_address = (u32)ExpandWriteLogicalAddressForRecordOverhead((s32)start_address);
    WriteWriteCommandU32(FW_NEAR_UNKNOWN_0B02, mapped_address);
}

static void PublishWriteCommandSpeedSnapshot(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7C, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7A, SelectStateEligibleLowestWriteSpeed());
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C7B, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
}

static void SetWriteCommandPrimaryCountdown(void)
{
    FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, (FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0u ? 60u : 20u);
}

static void FinishWriteCommandSuccessState(void)
{
    u8 pipeline_state;

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) < 0)
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x04u));
    else
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfbu));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_122D, 5u);
    pipeline_state = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);
    if(pipeline_state == 3u)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) < 0)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 1u));
            FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 20u);
        }
        return;
    }
    if(pipeline_state != 1u && pipeline_state != 2u)
        return;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5B1) != 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5AF) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 8u));
        SetWriteCommandPrimaryCountdown();
        return;
    }
    if(ReadWriteCommandU32(FW_HUGE_UNKNOWN_2BA59F) < ReadWriteCommandU32(FW_NEAR_UNKNOWN_1598))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_2BA5AF, 0u);
        FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 2u);
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 8u));
        SetWriteCommandPrimaryCountdown();
        return;
    }
    if((s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) < 0)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 1u));
        FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 20u);
    }
}

/* RAM:fcbe46 */
u8 StartOrContinueWriteCommandTransfer(u32 start_address, u32 block_count)
{
    u8 initialize_transfer;

    NormalizeWriteSpeedForCurrentMedia();
    initialize_transfer = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) == 0u);
    if(initialize_transfer != 0u)
    {
        u32 mapped_address;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 3u && (FirmwareReadController8(FW_CONTROLLER_COMMAND_ARGUMENT0) & 1u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) | 0x80u));
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x7fu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x08u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x10u));
        WriteWriteCommandU32(FW_NEAR_UNKNOWN_DD5B, 0ul);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
        ResetRawReadQueueAndTransferState();
        InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        YieldCoroutine0();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        {
            SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
            return 0u;
        }

        PublishWriteCommandStartAddress(start_address);
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) < 0)
        {
            mapped_address = MapFixedPacketLogicalToPhysicalAddress(start_address);
            WriteWriteCommandU32(FW_NEAR_UNKNOWN_0B02, mapped_address);
            if((s32)mapped_address >= 0x4e0l)
            {
                HandleUnsupportedScsiCommand();
                return 0u;
            }
        }
        else
        {
            mapped_address = (u32)ExpandWriteLogicalAddressForRecordOverhead((s32)start_address);
            WriteWriteCommandU32(FW_NEAR_UNKNOWN_0B02, mapped_address);
            if((s32)ReadWriteCommandU32(FW_HUGE_UNKNOWN_2BA529) < (s32)start_address)
                WriteWriteCommandU32(FW_NEAR_UNKNOWN_0B02, MapFixedPacketLogicalToPhysicalAddress(start_address));
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u && EnsureOpcForSelectedWriteSpeed() == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
            return 0u;
        }
        if(PrepareWritePipelineForStartAddress((s32)start_address) == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB69, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) | 1u));
    }
    else
    {
        u8 same_transfer_mode;
        u8 contiguous;
        u8 must_finish_previous;

        if(block_count == 0ul)
        {
            HandleUnsupportedScsiCommand();
            return 0u;
        }
        same_transfer_mode = (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >> 7) & 1u) == ((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >> 2) & 1u));
        contiguous = (u8)(start_address == ReadWriteCommandU32(FW_HUGE_UNKNOWN_2BC703) + ReadWriteCommandU32(FW_NEAR_UNKNOWN_0B06));
        must_finish_previous = (u8)(same_transfer_mode == 0u || contiguous == 0u);
        if(must_finish_previous == 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >= 0 && (s32)start_address % 0x1100l == 0l)
            must_finish_previous = 1u;

        if(must_finish_previous != 0u)
        {
            if(ExecuteWriteTransferBlockRange(ReadWriteCommandU32(FW_NEAR_UNKNOWN_0B06)) == 0u)
                return 0u;
            if(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0x2eu)
            {
                ExecuteWriteAndVerifyReadback();
                FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x40u));
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 0x10u));
            PublishWriteCommandStartAddress(start_address);
            MapWriteCommandStartAddress(start_address);
            if(PrepareWritePipelineForStartAddress((s32)start_address) == 0u)
                return 0u;
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xefu));
            MapWriteCommandStartAddress(start_address);
        }
    }

    WriteWriteCommandU32(FW_NEAR_UNKNOWN_0B06, block_count);
    WriteWriteCommandU32(FW_HUGE_UNKNOWN_2BC703, start_address);
    if(ExecuteWriteCommandTransferPipeline((u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 4) & 1u)) == 0u)
    {
        PublishWriteCommandSpeedSnapshot();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfbu));
        UpdateWorkingTrackAfterWriteFailure();
        return 0u;
    }

    PublishWriteCommandSpeedSnapshot();
    FinishWriteCommandSuccessState();
    return 1u;
}
