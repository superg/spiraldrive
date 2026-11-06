#include "raw_sector_completion.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_sector_descriptor_scratch.h"
#include "../platform/status_mapping.h"
#include "fixed_packet_overhead.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_ring_alignment.h"
#include "raw_read_speed_update.h"

extern void EnqueueRawSectorDescriptor(void);
extern u32 DivideS32MagnitudeRemainderNative(s32 dividend, s32 divisor);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_sector_completion_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_sector_completion_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCompletionWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteCompletionWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 ReadCompletionDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteCompletionDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void CompleteWithCountdownStatus(u16 status)
{
    ServiceRawReadCountdownAndEnqueueDescriptor(3u, status);
}

static void PublishTerminalMetadataStatus(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2F, FirmwareReadNear8(FW_NEAR_UNKNOWN_15A0));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F31, FirmwareReadNear8(FW_NEAR_UNKNOWN_15A1));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A3, FirmwareReadNear8(FW_NEAR_UNKNOWN_15A2));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 9u);
    EnqueueRawSectorDescriptorWithTimingGate(0u, 0u);
}

static void CountCompletionAtCurrentOperatingCode(void)
{
    u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    u16 total;

    if(operating_code < 8u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9D) + 1u));
    else if(operating_code < 0x18u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) + 1u));
    else if(operating_code < 0x20u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F) + 1u));
    else if(operating_code < 0x28u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0) + 1u));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA1, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1) + 1u));

    total = (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9D) + (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9E) + (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F9F) + (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA0)
          + (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA1);
    if(total >= 0x7fu)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9D, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9E, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9F, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA0, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA1, 0u);
    }
}

static void UpdateCompletionBoundaryState(u32 target)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
    {
        if(READ_COMPLETION_BIT_001F() != 0u)
        {
            u16 remaining;
            u32 boundary = ReadCompletionDword(FW_NEAR_UNKNOWN_0F00);

            WriteCompletionDword(FW_NEAR_UNKNOWN_0F3D, boundary);
            remaining = (u16)(ReadCompletionWord(FW_NEAR_UNKNOWN_FC42) - 1u);
            WriteCompletionWord(FW_NEAR_UNKNOWN_FC42, remaining);
            if(remaining == 0u)
            {
                WRITE_COMPLETION_BIT_001F(0u);
                WriteCompletionDword(FW_NEAR_UNKNOWN_1598, boundary + 1ul);
                InitializeControllerBufferCursorRegistersIfNeeded();
            }
            else
            {
                WriteCompletionDword(FW_NEAR_UNKNOWN_0F00, boundary + 1ul);
            }
        }
        else
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) == 0u && target == ReadCompletionDword(FW_NEAR_UNKNOWN_0F79))
                InitializeControllerBufferCursorRegistersIfNeeded();
            if(target == ReadCompletionDword(FW_NEAR_UNKNOWN_0F00))
                InitializeControllerBufferCursorRegistersIfNeeded();
        }
    }

    target = ReadCompletionDword(FW_NEAR_UNKNOWN_1598);

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) != 0u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u
        && READ_COMPLETION_BIT_001B() == 0u && (s32)target > 0x575l)
    {
        s32 delta = (s32)(target - ReadCompletionDword(FW_NEAR_UNKNOWN_0F3D));

        if(delta >= 0l && delta < 500l)
        {
            WRITE_COMPLETION_BIT_0020(1u);
        }
        else
        {
            delta = (s32)(ReadCompletionDword(FW_NEAR_UNKNOWN_0F00) - target);
            if(delta >= 0l && delta < 500l)
                WRITE_COMPLETION_BIT_0020(1u);
        }
    }
}

static void PublishCompletionTerminalFlag(void)
{
    u8 auxiliary_kind = FirmwareReadNear8(FW_NEAR_UNKNOWN_12B2);
    u16 auxiliary_word;

    if(auxiliary_kind == 0u || (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) != 0u)
        return;

    auxiliary_word = ReadCompletionWord(FW_NEAR_UNKNOWN_12B0);
    if(auxiliary_word == 0x1105u || auxiliary_word == 0x1106u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 0x10u) == 0u)
            WRITE_COMPLETION_BIT_0009(1u);
    }
    else if(auxiliary_word == 0x1804u || auxiliary_word == 0x1803u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 6u) == 6u)
            WRITE_COMPLETION_BIT_0009(1u);
    }
    else
    {
        WRITE_COMPLETION_BIT_0009(1u);
    }
}

static u8 ServiceCompletionIdleCounter(void)
{
    u16 idle_count;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) >= 5u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44) != 5u)
    {
        WriteCompletionWord(FW_NEAR_UNKNOWN_0F1C, 0u);
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12B2) != 0u)
    {
        WriteCompletionWord(FW_NEAR_UNKNOWN_0F1C, 0u);
        return 0u;
    }

    idle_count = (u16)(ReadCompletionWord(FW_NEAR_UNKNOWN_0F1C) + 1u);
    WriteCompletionWord(FW_NEAR_UNKNOWN_0F1C, idle_count);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44) == 0u && idle_count >= 0x02eeu) || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44) == 5u && idle_count >= 0x05dcu))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 2u);
        InitializeControllerBufferCursorRegistersIfNeeded();
        return 1u;
    }
    return 0u;
}

static void ApplyCompletionSpeedRecovery(void)
{
    u16 progress = ReadCompletionWord(FW_NEAR_UNKNOWN_0F3B);

    if(progress < 9000u)
    {
        WriteCompletionWord(FW_NEAR_UNKNOWN_0F3B, (u16)(progress + 1u));
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u && FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37))
        {
            InitializeControllerBufferCursorRegistersIfNeeded();
            UpdateRawReadSpeedForCurrentPipelineState();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 1u);
        }
        return;
    }

    WriteCompletionWord(FW_NEAR_UNKNOWN_0F3B, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37))
    {
        u8 speed;

        WRITE_COMPLETION_BIT_0027(1u);
        InitializeControllerBufferCursorRegistersIfNeeded();
        if(READ_COMPLETION_BIT_0020() == 0u)
        {
            UpdateRawReadSpeedForCurrentPipelineState();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 1u);
            return;
        }

        speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
        if(speed == 8u)
            speed = 0x18u;
        else if(speed == 0x18u)
            speed = 0x20u;
        else if(speed == 4u)
            speed = 8u;
        else
            speed = 0x28u;
        if(speed > FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A))
            speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, speed);
    }
}

static void ApplyCompletionQueuePressure(void)
{
    u16 high_water = ReadCompletionWord(FW_NEAR_UNKNOWN_129C);
    u16 queued = ReadCompletionWord(FW_NEAR_UNKNOWN_1292);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15E2) >= 10u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x28u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) >= 0x28u && (u16)(high_water - 75u) < queued)
    {
        InitializeControllerBufferCursorRegistersIfNeeded();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 8u);
    }

    if(READ_COMPLETION_BIT_000C() != 0u)
    {
        if((u16)(high_water - 8u) <= queued)
            InitializeControllerBufferCursorRegistersIfNeeded();
    }
    else if(high_water == queued)
    {
        InitializeControllerBufferCursorRegistersIfNeeded();
    }
}

static u8 HandleCompletionPhase(void)
{
    u8 phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F25);

    if(phase == 2u)
    {
        u8 accepted;

        if(READ_COMPLETION_BIT_0006() != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 1u);
            accepted = HandleRawReadDescriptorForCompletionFlags();
        }
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F30) & 0x30u) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
        {
            if(READ_COMPLETION_BIT_000C() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2E) == 3u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 2u);
                InitializeControllerBufferCursorRegistersIfNeeded();
                return 0u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 2u);
            accepted = BuildOrQueueRawSectorDescriptorForReadPhase2();
        }
        else
        {
            if(READ_COMPLETION_BIT_000C() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2E) == 2u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 3u);
                InitializeControllerBufferCursorRegistersIfNeeded();
                return 0u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 3u);
            accepted = HandleRawReadDescriptorForCompletionFlags();
        }
        return accepted;
    }

    if(phase == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 0u);
        if(BuildOrQueueRawSectorDescriptorForReadPhase1() == 0u)
            return 0u;
        if(READ_COMPLETION_BIT_000F() != 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A0) >= 0xacu && ReadCompletionWord(FW_NEAR_UNKNOWN_1292) == 0u)
            {
                CompleteWithCountdownStatus(0x0283u);
                return 0u;
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A0) == 0xadu)
            {
                PublishTerminalMetadataStatus();
                return 0u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B6, FirmwareReadNear8(FW_NEAR_UNKNOWN_15A0));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B7, FirmwareReadNear8(FW_NEAR_UNKNOWN_15A1));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12B4, FirmwareReadNear8(FW_NEAR_UNKNOWN_15A2));
        }
        return 1u;
    }

    if(READ_COMPLETION_BIT_000F() != 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_15A0) == 0xadu || FirmwareReadNear8(FW_NEAR_UNKNOWN_15A3) == 0u))
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A0) >= 0xacu && ReadCompletionWord(FW_NEAR_UNKNOWN_1292) == 0u)
        {
            CompleteWithCountdownStatus(0x0283u);
            return 0u;
        }
        PublishTerminalMetadataStatus();
        return 0u;
    }

    if(READ_COMPLETION_BIT_000C() != 0u)
    {
        u8 metadata_flags = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A3) & 0xe0u);

        if(metadata_flags == 0xe0u)
        {
            u8 remaining = GetFixedPacketOverheadFramesRemaining((s32)ReadCompletionDword(FW_NEAR_UNKNOWN_1598));

            if((s8)remaining <= 0)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 4u);
                CompleteWithCountdownStatus(0x0600u);
                return 0u;
            }
            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
            {
                WRITE_COMPLETION_BIT_0025(0u);
                if(FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u && READ_COMPLETION_BIT_001F() == 0u)
                {
                    u16 table_offset = (u16)DivideS32MagnitudeRemainderNative((s32)(ReadCompletionDword(FW_NEAR_UNKNOWN_1598) - 0x8aul), 39l);
                    fw_addr_t table_address = FirmwareAddressIncrementLow16(ReadCompletionDword(FW_NEAR_UNKNOWN_FAFF), table_offset);

                    if(FirmwareRead8(table_address) == 0u)
                    {
                        WRITE_COMPLETION_BIT_0025(1u);
                        InitializeControllerBufferCursorRegistersIfNeeded();
                        return 0u;
                    }
                }
            }

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 10u);
            {
                u8 index;
                u32 target = ReadCompletionDword(FW_NEAR_UNKNOWN_1598);

                for(index = 0u; index < 7u; ++index)
                {
                    BuildRawSectorDescriptorScratch(0x0600u, 3u);
                    EnqueueRawSectorDescriptor();
                    ++target;
                    WriteCompletionDword(FW_NEAR_UNKNOWN_1598, target);
                }
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1F, 6u);
            return 0u;
        }
        if(metadata_flags != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 4u);
            CompleteWithCountdownStatus(0x0600u);
            return 0u;
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_15A3) & 0xe0u) != 0u && (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) <= (s16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 5u))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 4u);
        EnqueueRawSectorDescriptorWithTimingGate(0x0600u, 3u);
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15A3) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
        CompleteWithCountdownStatus((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 1u) != 0u ? 0x1106u : 0x1105u);
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 0u);
    BuildRawSectorDescriptorScratch(0u, 0u);
    return 1u;
}

static u8 HandleCompletionTerminalBoundary(void)
{
    s32 target = (s32)ReadCompletionDword(FW_NEAR_UNKNOWN_1598);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 4u) != 0u && target >= (s32)(ReadCompletionDword(FW_NEAR_UNKNOWN_12C1) + 100ul))
        {
            EnqueueRawSectorDescriptorWithTimingGate(0x6300u, 5u);
            return 1u;
        }
        return 0u;
    }

    if((s32)ReadCompletionDword(FW_NEAR_UNKNOWN_12C1) > (s32)ReadCompletionDword(FW_NEAR_CURRENT_FRAME_POSITION))
    {
        if(target >= (s32)ReadCompletionDword(FW_NEAR_UNKNOWN_12C1))
        {
            EnqueueRawSectorDescriptorWithTimingGate(0x6300u, 5u);
            return 1u;
        }
    }
    else if(target >= (s32)ReadCompletionDword(FW_NEAR_CURRENT_FRAME_POSITION))
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 1u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x80u) == 0u)
        {
            EnqueueRawSectorDescriptorWithTimingGate(0x6300u, 5u);
            return 1u;
        }
        WRITE_COMPLETION_BIT_000C(0u);
    }
    return 0u;
}

/* RAM:f9199e */
void PLEXTOR_NEAR_FUNCTION ProcessControllerReadSectorCompletion(void)
{
    u32 target;

    if((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 7) != FirmwareReadNear8(FW_NEAR_UNKNOWN_0F75))
    {
        CompleteWithCountdownStatus(0x0200u);
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2F) & 0xc0u) != 0u)
    {
        if(CheckP1Bit3StableHighForCurrentMediaState() != 0u)
        {
            FUN_RAM_f93a5e();
            CompleteWithCountdownStatus(0x0281u);
        }
        else
        {
            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u)
            {
                CompleteWithCountdownStatus(0x0281u);
                return;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
            EnqueueRawSectorDescriptorWithTimingGate(0x2100u, 5u);
        }
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
            WRITE_COMPLETION_BIT_0005(1u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < 0x34u && ReadCompletionDword(FW_NEAR_UNKNOWN_126E) % 0x0930ul != 0ul)
    {
        CompleteWithCountdownStatus((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 1u) != 0u ? 0x1106u : 0x1105u);
        return;
    }

    if(DecodeControllerPositionFields(0u) == 0u)
    {
        FUN_RAM_f93a5e();
        CompleteWithCountdownStatus((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 1u) != 0u ? 0x1106u : 0x1105u);
        return;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, 0x1eu);

    target = ReadCompletionDword(FW_NEAR_UNKNOWN_1598);
    if(target != ReadCompletionDword(FW_NEAR_UNKNOWN_0F6E) && READ_COMPLETION_BIT_0013() == 0u && READ_COMPLETION_BIT_001C() == 0u && READ_COMPLETION_BIT_001D() == 0u)
    {
        FUN_RAM_f93a5e();
        CompleteWithCountdownStatus(0x0283u);
        return;
    }

    if(HandleCompletionPhase() == 0u)
        return;

    WRITE_COMPLETION_BIT_0004(0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F74) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, 0u);
    if(HandleCompletionTerminalBoundary() != 0u)
        return;

    EnqueueRawSectorDescriptor();
    target = ReadCompletionDword(FW_NEAR_UNKNOWN_1598) + 1ul;
    WriteCompletionDword(FW_NEAR_UNKNOWN_1598, target);
    WRITE_COMPLETION_BIT_0020(0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA2, 10u);
        if(READ_COMPLETION_BIT_0027() != 0u)
            CountCompletionAtCurrentOperatingCode();
        UpdateCompletionBoundaryState(target);
    }

    PublishCompletionTerminalFlag();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
    WRITE_COMPLETION_BIT_0024(0u);
    if(ServiceCompletionIdleCounter() != 0u)
        return;
    ApplyCompletionSpeedRecovery();
    ApplyCompletionQueuePressure();
}
