#include "primary_raw_read_pipeline_step.h"
#include "../platform/controller_record_bank_scan.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/raw_sector_descriptor_scratch.h"
#include "../platform/status_mapping.h"
#include "fixed_packet_overhead.h"
#include "primary_raw_read_capture_batch.h"
#include "primary_raw_read_operating_code.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_position_probe.h"
#include "raw_read_runtime.h"
#include "read_acquisition_request.h"

extern void YieldCoroutine2(void);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);

#define PRIMARY_RAW_READ_DESCRIPTOR 0x0f04u

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/primary_raw_read_pipeline_step_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/primary_raw_read_pipeline_step_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPipelineWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadPipelineDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WritePipelineDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u16 GetPipelineFreeSlotCount(void)
{
    return (u16)((u16)(ReadPipelineDword(FW_NEAR_UNKNOWN_156A) - 6ul) - ReadPipelineWord(FW_NEAR_UNKNOWN_1292));
}

static u8 ReadControllerRecordState(s32 frame_position, s32 base)
{
    s16 remainder = (s16)((frame_position - base) % 39L);
    fw_addr_t table = ReadPipelineDword(FW_NEAR_UNKNOWN_FAFF);

    return FirmwareRead8(FirmwareAddressIncrementLow16(table, (u16)remainder));
}

static void BuildAndEnqueuePipelineDescriptor(u16 status, u8 mode)
{
    BuildRawSectorDescriptorScratch(status, mode);
    EnqueueRawSectorDescriptor();
}

static void AdvancePipelineTarget(void)
{
    WritePipelineDword(FW_NEAR_UNKNOWN_1598, ReadPipelineDword(FW_NEAR_UNKNOWN_1598) + 1ul);
}

static void WaitForFixedPacketCapacity(void)
{
    while(GetPipelineFreeSlotCount() < 7u)
    {
        YieldCoroutine2();
        if(ReadPipelineWord(FW_NEAR_UNKNOWN_1292) == 0u)
            break;
    }
}

static void WaitForCaptureBatchCapacity(u16 required)
{
    while(GetPipelineFreeSlotCount() < required)
        YieldCoroutine2();
}

static void WaitForScannedDescriptorCapacity(u16 required)
{
    do
    {
        YieldCoroutine2();
        if(ReadPipelineWord(FW_NEAR_UNKNOWN_1292) == 0u)
            break;
    } while(GetPipelineFreeSlotCount() < required);
}

static void CopyPipelineDescriptorProfile(u16 descriptor)
{
    u8 mode = FirmwareRead8((fw_addr_t)(descriptor + 2u));
    fw_addr_t profile = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9574) + (fw_addr_t)mode * 4ul;

    WritePipelineDword((fw_addr_t)(descriptor + 0x0cu), ReadPipelineDword(profile));
    FirmwareWrite8((fw_addr_t)(descriptor + 0x1cu), FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9588) + mode));
}

static void QueueAcquisitionResultDescriptor(u8 result)
{
    s32 signed_result = (s32)(s8)result;
    fw_addr_t status_address = (fw_addr_t)(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + signed_result * 2L);
    u16 status = ReadPipelineWord(status_address);

    if(status == 0x0200u)
    {
        BuildAndEnqueuePipelineDescriptor(0x0280u, 3u);
    }
    else
    {
        fw_addr_t mode_address = (fw_addr_t)(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + signed_result);

        BuildAndEnqueuePipelineDescriptor(status, FirmwareRead8(mode_address));
    }
}

/* RAM:f9019b */
u8 PLEXTOR_NEAR_FUNCTION ProcessPrimaryRawReadPipelineStep(void)
{
    u8 retry = CAPTURE_PIPELINE_RETRY();

    FirmwareWrite8(PRIMARY_RAW_READ_DESCRIPTOR + 0x1ful, 0x19u);
    if(READ_PIPELINE_BIT_000C() == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F3B, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F3C, 0u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
    {
        if(READ_PIPELINE_BIT_001F() == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
        {
            s32 target = (s32)ReadPipelineDword(FW_NEAR_UNKNOWN_1598);

            if((FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u || (s32)ReadPipelineDword(FW_HUGE_UNKNOWN_2BA58D) < target) && READ_PIPELINE_BIT_0025() == 0u
                && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && READ_PIPELINE_BIT_001B() == 0u)
            {
                if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u)
                {
                    WRITE_PIPELINE_BIT_0009(1u);
                    return 0u;
                }
                if(ReadControllerRecordState(target, 145L) == 0u)
                    WRITE_PIPELINE_BIT_0025(1u);
            }
            if(READ_PIPELINE_BIT_0025() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 3u)
            {
                DispatchReadAcquisitionRequest5ForPipelineTarget();
                YieldCoroutine2();
            }

            {
                u8 overhead = GetFixedPacketOverheadFramesRemaining(target);

                if((s8)overhead > 0)
                {
                    u8 index;

                    if(GetPipelineFreeSlotCount() < 7u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 3u)
                        DispatchReadAcquisitionRequest5ForPipelineTarget();
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 10u);
                    WaitForFixedPacketCapacity();
                    for(index = 0u; (s8)index < (s8)overhead; ++index)
                    {
                        BuildAndEnqueuePipelineDescriptor(0x0600u, 3u);
                        AdvancePipelineTarget();
                        AdvanceRawReadRingCursors();
                    }
                    return 0u;
                }
            }

            if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) != 0xb9u && READ_PIPELINE_BIT_0021() == 0u)
            {
                if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >= 0)
                {
                    u8 index = SelectDriveNumericTable0IntervalForReadPipelineTarget();

                    if(index != 0u)
                    {
                        u32 selected_target = ReadPipelineDword(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84690) + (fw_addr_t)index * 4ul);

                        WRITE_PIPELINE_BIT_0025(0u);
                        if(ReadControllerRecordState((s32)selected_target, 145L) == 0u)
                        {
                            WRITE_PIPELINE_BIT_0025(1u);
                            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 3u)
                            {
                                DispatchReadAcquisitionRequest5ForPipelineTarget();
                                YieldCoroutine2();
                            }
                            WritePipelineDword(FW_NEAR_UNKNOWN_1598, selected_target);
                            return 0u;
                        }
                        WritePipelineDword(FW_NEAR_UNKNOWN_1598, selected_target);
                    }
                }
                if(READ_PIPELINE_BIT_001B() != 0u)
                    WRITE_PIPELINE_BIT_0025(0u);
                if(READ_PIPELINE_BIT_0025() != 0u)
                {
                    u32 batch_target = ReadPipelineDword(FW_NEAR_UNKNOWN_1598);
                    u16 remainder = (u16)((batch_target - 150ul) % 39ul);
                    u16 batch_count = (u16)(32u - remainder);
                    u8 index;

                    WaitForCaptureBatchCapacity(batch_count);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 2u);
                    PrepareAndSubmitPrimaryRawReadCaptureBatch((u8)batch_count);
                    for(index = 0u; index < batch_count; ++index)
                    {
                        BuildAndEnqueuePipelineDescriptor(0u, 0u);
                        AdvancePipelineTarget();
                        AdvanceRawReadRingCursors();
                    }
                    WRITE_PIPELINE_BIT_0025(0u);
                    if(ReadControllerRecordState((s32)ReadPipelineDword(FW_NEAR_UNKNOWN_1598), 138L) == 0u)
                        WRITE_PIPELINE_BIT_0025(1u);
                    return 0u;
                }

                target = (s32)ReadPipelineDword(FW_NEAR_UNKNOWN_1598);
                if(target >= (s32)ReadPipelineDword(FW_NEAR_UNKNOWN_0F00) && target <= (s32)ReadPipelineDword(FW_NEAR_SCAN_POSITION_SECOND))
                {
                    u16 found = ReadPipelineWord(FW_NEAR_UNKNOWN_0F7D);
                    u32 first_position = ReadPipelineDword(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT);

                    if(found == 0u || (u32)target != first_position)
                    {
                        u32 count = ReadPipelineDword(FW_NEAR_SCAN_POSITION_SECOND) - (u32)target + 1ul;

                        WritePipelineDword(FW_NEAR_UNKNOWN_0F00, (u32)target + count);
                        found = ScanControllerRecordBanksForFrameRange(0u, target, count);
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F7D, (u8)(found >> 8));
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F7E, (u8)found);
                        if(found != 0u)
                        {
                            first_position = ReadPipelineDword(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT);
                            WritePipelineDword(FW_NEAR_UNKNOWN_0F00, first_position);
                            if((u32)target == first_position)
                            {
                                WritePipelineDword(FW_NEAR_UNKNOWN_1598, ReadPipelineDword(FW_HUGE_UNKNOWN_2B9700));
                                WRITE_PIPELINE_BIT_001F(1u);
                            }
                            if((s32)(first_position - ReadPipelineDword(FW_NEAR_UNKNOWN_1598)) < 500L)
                                WRITE_PIPELINE_BIT_0020(1u);
                        }
                    }
                    else
                    {
                        WritePipelineDword(FW_NEAR_UNKNOWN_1598, ReadPipelineDword(FW_HUGE_UNKNOWN_2B9700));
                        WRITE_PIPELINE_BIT_001F(1u);
                    }
                }
            }
            else
            {
                target = (s32)ReadPipelineDword(FW_NEAR_UNKNOWN_1598);
                if(target >= (s32)ReadPipelineDword(FW_NEAR_UNKNOWN_0F00))
                {
                    u32 count;
                    u16 found;

                    if((u32)target == ReadPipelineDword(FW_NEAR_UNKNOWN_0F00) && ReadPipelineWord(FW_NEAR_UNKNOWN_0F7D) != 0u)
                        count = 31ul;
                    else
                        count = ReadPipelineDword(FW_NEAR_CURRENT_FRAME_POSITION) - (u32)target + 1ul;
                    WritePipelineDword(FW_NEAR_UNKNOWN_0F00, (u32)target + count);
                    found = ScanControllerRecordBanksForFrameRange(0u, target, count);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F7D, (u8)(found >> 8));
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F7E, (u8)found);
                    if(found != 0u)
                    {
                        if((u32)found == count)
                        {
                            u16 index;

                            WaitForScannedDescriptorCapacity(31u);
                            for(index = 0u; index < found; ++index)
                            {
                                BuildAndEnqueuePipelineDescriptor(0u, 0u);
                                AdvancePipelineTarget();
                            }
                            WritePipelineDword(FW_NEAR_UNKNOWN_0F00, ReadPipelineDword(FW_NEAR_UNKNOWN_1598));
                            return 0u;
                        }
                        WritePipelineDword(FW_NEAR_UNKNOWN_0F00, ReadPipelineDword(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT));
                        if((u32)target == ReadPipelineDword(FW_HUGE_CONTROLLER_SCAN_SNAPSHOT))
                        {
                            WaitForScannedDescriptorCapacity(16u);
                            BuildAndEnqueuePipelineDescriptor(0u, 0u);
                            AdvancePipelineTarget();
                            WritePipelineDword(FW_NEAR_UNKNOWN_0F00, ReadPipelineDword(FW_NEAR_UNKNOWN_1598));
                            return 0u;
                        }
                    }
                }
            }
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) != 0u)
        {
            u16 found;

            WRITE_PIPELINE_BIT_0020(1u);
            if(ReadPipelineWord(FW_NEAR_UNKNOWN_1292) >= 32u)
            {
                WRITE_PIPELINE_BIT_0009(1u);
                return 0u;
            }
            found = ScanControllerRecordBanksForFrameRange(0u, (s32)ReadPipelineDword(FW_NEAR_UNKNOWN_1598), 1ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F7D, (u8)(found >> 8));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F7E, (u8)found);
            if(found != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 0x0bu);
                BuildAndEnqueuePipelineDescriptor(0x0200u, 3u);
                AdvancePipelineTarget();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9C, 0u);
                return 0u;
            }
        }
    }

    for(;;)
    {
        u8 result;
        s32 adjustment;

        if(READ_PIPELINE_BIT_0009() != 0u)
            return 0u;
        ThrottleRawReadProducerForQueueCapacity();
        CopyPipelineDescriptorProfile(PRIMARY_RAW_READ_DESCRIPTOR);

        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u && FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u)
        {
            adjustment = 0L;
        }
        else if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u)
        {
            u8 delta = (u8)(0x1bu - FirmwareRead8(PRIMARY_RAW_READ_DESCRIPTOR + 0x1fu));

            adjustment = (s32)(ReadPipelineDword(FW_NEAR_UNKNOWN_159C) - (u32)delta);
        }
        else
        {
            adjustment = 0L;
        }
        WritePipelineDword(FW_NEAR_UNKNOWN_159C, (u32)adjustment);

        if(READ_PIPELINE_BIT_0007() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 4)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 4u);
        }

        if(READ_PIPELINE_BIT_0013() != 0u || READ_PIPELINE_BIT_001C() != 0u || READ_PIPELINE_BIT_001D() != 0u)
        {
            result = RequestReadAcquisitionAndWait(0x10000000L, 0L);
        }
        else
        {
            u32 profile_adjustment = ReadPipelineDword(PRIMARY_RAW_READ_DESCRIPTOR + 0x0cul);

            result = RequestReadAcquisitionAndWait((s32)ReadPipelineDword(FW_NEAR_UNKNOWN_1598), (s32)((u32)adjustment + profile_adjustment));
        }

        if(result != 0u)
        {
            if(READ_PIPELINE_BIT_0007() == 0u)
            {
                WRITE_PIPELINE_BIT_0007(1u);
                return 0u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
            QueueAcquisitionResultDescriptor(result);
            if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
                WRITE_PIPELINE_BIT_0009(1u);
            else
                AdvancePipelineTarget();
            return 0u;
        }

        InitializeRawReadControllerForPositionProbe();
        do
        {
            if(((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F76) <= 0 && READ_PIPELINE_BIT_000F() == 0u) || (READ_PIPELINE_BIT_000F() != 0u && CheckP1Bit3StableHighForCurrentMediaState() == 0u))
            {
                s8 old_retry = (s8)retry;

                retry = (u8)(retry - 1u);
                if(old_retry <= 0)
                {
                    if(FirmwareRead8(PRIMARY_RAW_READ_DESCRIPTOR + 0x1ful) <= 0x14u)
                        FirmwareWrite8(PRIMARY_RAW_READ_DESCRIPTOR + 0x1ful, 0u);
                    ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0082u);
                }
            }
            else
            {
                retry = 8u;
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F76) <= 0)
                ServiceParsedQRawReadCountdownAndEnqueueDescriptor(0x0081u);
            YieldCoroutine2();
        } while(FirmwareRead8(PRIMARY_RAW_READ_DESCRIPTOR) > 0u && FirmwareRead8(PRIMARY_RAW_READ_DESCRIPTOR) <= 3u);

        if(FirmwareRead8(PRIMARY_RAW_READ_DESCRIPTOR) != 0u)
            return 1u;
        UpdatePrimaryRawReadOperatingCodeSelector();
    }
}
