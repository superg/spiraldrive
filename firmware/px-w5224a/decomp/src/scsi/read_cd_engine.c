#include "read_cd_engine.h"
#include "../media/read_acquisition_request.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "raw_sector_host_transfer_poll.h"
#include "raw_sector_range.h"
#include "read_cd_track_coverage.h"

extern void YieldCoroutine0(void);
extern s8 ExecuteReadCdRawSectorTransferForFormat(u8 raw_read_format_index);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_cd_engine_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static const u8 ReadCdFormatCodeBySelection[5] = { 0u, 4u, 1u, 0u, 2u };

/* Reconstruction helpers; no direct Ghidra addresses. */
static u32 ReadReadCdDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteReadCdDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void ReportReadCdFailure(u16 sense_code)
{
    SetSenseData(3u, sense_code);
    SetSenseInformation(ReadReadCdDword(FW_NEAR_UNKNOWN_0B02) + ReadReadCdDword(FW_NEAR_UNKNOWN_1459));
}

static void RestartReadCdAcquisition(void)
{
    ResetRawReadQueueAndTransferState();
    InitializeCoroutine2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    DispatchReadAcquisitionRequest5ForPipelineTarget();
    YieldCoroutine0();
}

static void ContinueReadCdRange(s32 start_frame_position, s32 end_frame_position, u8 format_code)
{
    (void)ExecuteRawSectorReadRange(start_frame_position, end_frame_position, format_code);
    WriteReadCdDword(FW_NEAR_UNKNOWN_15EA, (u32)end_frame_position);
}

static void StartReadCdRangeAfterHostTransfer(s32 start_frame_position, s32 end_frame_position, u8 format_code)
{
    u16 descriptor_address;
    u32 completed_block_count;
    u8 option_flags;

    if(ExecuteReadCdRawSectorTransferForFormat(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14)) != 0)
        return;

    descriptor_address = GetRawReadSecondaryCursorDescriptorAddress();
    if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x13u)) == 4u)
    {
        ReportReadCdFailure(0x0600u);
        RestartReadCdAcquisition();
        return;
    }

    completed_block_count = ReadReadCdDword(FW_NEAR_UNKNOWN_1459);
    option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B);
    if(completed_block_count == 0ul && (option_flags == 0xf8u || option_flags == 0xfau))
    {
        if((FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x14u)) & 0x40u) == 0u)
            return;
        if(ExecuteRawSectorReadRange((s32)((u32)start_frame_position + completed_block_count), end_frame_position, format_code) == 1u)
        {
            WriteReadCdDword(FW_NEAR_UNKNOWN_15EA, (u32)end_frame_position);
            WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_0010, 1u);
        }
        return;
    }

    ReportReadCdFailure(0x1500u);
}

static void ExecuteValidatedReadCdRange(s32 start_frame_position, s32 end_frame_position, u8 format_code)
{
    if(READ_CD_BIT(FW_BIT_UNKNOWN_0011) == 0u)
    {
        (void)ExecuteReadCdRawSectorTransferForFormat(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14));
        WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_0021, 0u);
        return;
    }

    if(READ_CD_BIT(FW_BIT_UNKNOWN_0010) != 0u && (s32)ReadReadCdDword(FW_NEAR_UNKNOWN_15EA) == start_frame_position)
    {
        ContinueReadCdRange(start_frame_position, end_frame_position, format_code);
        return;
    }

    WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_0010, 0u);
    StartReadCdRangeAfterHostTransfer(start_frame_position, end_frame_position, format_code);
}

static void SelectNegativeReadCdMode(s32 command_start)
{
    WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_001D, 0u);
    WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_001C, 0u);
    WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_0013, 0u);

    if(READ_CD_BIT(FW_BIT_UNKNOWN_0021) != 0u || command_start > -1000l)
        return;
    if(command_start > -20000l)
        WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_0013, 1u);
    else if(command_start > -30000l)
        WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_001C, 1u);
    else
        WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_001D, 1u);
}

/* RAM:fd3442 */
void ExecuteReadCdCommand(void)
{
    u8 validation_options;
    u8 validation_flags;
    u8 validation_control;
    u8 selection;
    u8 option_flags;
    u8 format_code;
    u32 block_count;
    s32 command_start;
    s32 start_frame_position;
    s32 end_frame_position;

    validation_options = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13);
    validation_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B);
    validation_control = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B15);
    if(((validation_options & 5u) | (validation_flags & 0xe3u) | (validation_control & 0xfcu)) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14) > 4u)
    {
        HandleUnsupportedScsiCommand();
        return;
    }

    WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_000E, 0u);
    WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_0018, 0u);
    block_count = ReadReadCdDword(FW_NEAR_UNKNOWN_0B06);
    if(block_count == 0ul)
    {
        PrepareAndDispatchReadOrSeekAcquisitionAtLba((s32)ReadReadCdDword(FW_NEAR_UNKNOWN_0B02));
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0A, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1cu) >> 2));
    option_flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B13);
    if(READ_CD_BIT(FW_BIT_UNKNOWN_000D) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B) & 2u) != (option_flags & 2u))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 7u);
        WRITE_READ_CD_BIT(FW_BIT_UNKNOWN_000D, 1u);
    }
    if(option_flags != FirmwareReadNear8(FW_NEAR_UNKNOWN_0F0B))
        ResetRawReadProgressCountersAndPhaseFlags();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F0B, option_flags);

    command_start = (s32)ReadReadCdDword(FW_NEAR_UNKNOWN_0B02);
    block_count = ReadReadCdDword(FW_NEAR_UNKNOWN_0B06);
    start_frame_position = (s32)((u32)command_start + 150ul);
    end_frame_position = (s32)((u32)start_frame_position + block_count);
    selection = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B14);
    format_code = ReadCdFormatCodeBySelection[selection];
    SelectNegativeReadCdMode(command_start);

    if(READ_CD_BIT(FW_BIT_UNKNOWN_001C) == 0u && READ_CD_BIT(FW_BIT_UNKNOWN_001D) == 0u && ClassifyReadCdFrameRangeTrackCoverage(start_frame_position, end_frame_position) == 2u)
    {
        (void)ExecuteRawSectorReadRange(start_frame_position, end_frame_position, format_code);
        return;
    }

    ExecuteValidatedReadCdRange(start_frame_position, end_frame_position, format_code);
}
