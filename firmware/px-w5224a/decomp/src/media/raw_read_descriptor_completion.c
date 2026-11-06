#include "raw_read_descriptor_completion.h"
#include "../platform/firmware_memory.h"
#include "../platform/raw_sector_descriptor_scratch.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_descriptor_completion_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_descriptor_completion_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadBigEndianU32ForDescriptorCountdown(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u16 ReadBigEndianU16ForDescriptorCountdown(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteBigEndianU32ForDescriptorCountdown(fw_addr_t address, u32 value)
{
    FirmwareWrite8(address, (u8)(value >> 24));
    FirmwareWrite8(address + 1u, (u8)(value >> 16));
    FirmwareWrite8(address + 2u, (u8)(value >> 8));
    FirmwareWrite8(address + 3u, (u8)value);
}

static void WriteBigEndianU16ForHostDescriptorCompletion(fw_addr_t address, u16 value)
{
    FirmwareWrite8(address, (u8)(value >> 8));
    FirmwareWrite8(address + 1u, (u8)value);
}

/* RAM:fde8d8 */
/* The interrupt dispatcher supplies B, WR2 and DR4 as inherited state.  The
 * two WR8 loads immediately before the comparison are behaviorally unused,
 * but remain volatile reads and therefore stay in the C reconstruction. */
void HandleRawReadHostTransferDescriptorCompletion(void)
{
    u8 inherited_b = CaptureBForRawReadHostTransferDescriptorCompletion();
    u32 inherited_dr4 = CaptureDR4ForRawReadHostTransferDescriptorCompletion();
    u16 inherited_wr2 = CaptureWR2ForRawReadHostTransferDescriptorCompletion();
    u16 descriptor_seed = (u16)(((u16)inherited_b << 8) | 0x14u);
    u16 descriptor_address;
    u16 cursor;
    u16 comparison_pointer;
    u32 comparison_value;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018047, 0x14u);
    descriptor_address = ReadBigEndianU16ForDescriptorCountdown((u16)(descriptor_seed + 0x0eu));
    cursor = (u16)(descriptor_address + ReadBigEndianU16ForDescriptorCountdown(FW_NEAR_UNKNOWN_1298));
    cursor &= ReadBigEndianU16ForDescriptorCountdown(FW_NEAR_UNKNOWN_129A);
    WriteBigEndianU16ForHostDescriptorCompletion(FW_NEAR_UNKNOWN_1298, cursor);
    FirmwareWriteNear8((u16)(descriptor_address + 0x0du), 0u);

    comparison_pointer = ReadBigEndianU16ForDescriptorCountdown((u16)(inherited_wr2 + 2u));
    comparison_value = ((u32)ReadBigEndianU16ForDescriptorCountdown(comparison_pointer) << 16) | inherited_wr2;
    (void)ReadBigEndianU16ForDescriptorCountdown((u16)((u16)inherited_dr4 + 6u));
    (void)ReadBigEndianU16ForDescriptorCountdown((u16)((u16)(inherited_dr4 >> 16) + 4u));

    if(inherited_dr4 == comparison_value)
    {
        u8 completion_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1467);

        completion_state |= FirmwareReadNear8(FW_NEAR_UNKNOWN_1466);
        completion_state |= FirmwareReadNear8(FW_NEAR_UNKNOWN_1468);
        if(completion_state == 0u)
        {
            DispatchSetNoSenseForRawReadHostTransferDescriptorCompletion();
            DispatchPrepareAtaStatusForRawReadHostTransferDescriptorCompletion();
            DispatchResetControllerForRawReadHostTransferDescriptorCompletion();
        }
        DispatchLimitBacklogForRawReadHostTransferDescriptorCompletion();
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1468) == 0u)
    {
        DispatchLimitBacklogForRawReadHostTransferDescriptorCompletion();
    }
}

/* RAM:f93429 */
void PLEXTOR_NEAR_FUNCTION ServiceParsedQDescriptorCountdownAndPositionAdvance(void)
{
    u8 countdown;

    DispatchInitializeControllerBufferCursorForParsedQCountdown();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) - 1u));
    if(DispatchCheckP1Bit3StableForParsedQCountdown() == 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 22)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1245) & 0x40u) == 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 19)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0u);

    if(FirmwareReadBit000E() != 0u)
    {
        if(ReadBigEndianU16ForDescriptorCountdown(FW_NEAR_UNKNOWN_1292) <= 75u)
        {
            u8 media_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);

            countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23);
            if((media_mode == 2u && (s8)countdown <= 23) || (media_mode != 2u && (s8)countdown <= 15))
            {
                WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598, ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598) + 2ul);
                WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F36, ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F36) - 2ul);
            }
        }
        else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 25u);
        }
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 0)
    {
        DispatchEnqueueParsedQDescriptorForPositionAdvance();
        FirmwareWriteBit0009(1u);
    }
}

/* RAM:f92296 */
u8 PLEXTOR_NEAR_FUNCTION HandleRawReadDescriptorForCompletionFlags(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 1u) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2E) == 1u && FirmwareReadBit0004() == 0u)
            DispatchBuildRawSectorDescriptorScratch(0u, 0u);
        else
            DispatchBuildRawSectorDescriptorScratch(0x1803u, 1u);
        return 1u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F27) & 0x40u) != 0u)
    {
        DispatchServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x1106u);
        return 0u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F26) & 0x40u) == 0u)
    {
        DispatchBuildRawSectorDescriptorScratch(0u, 0u);
        return 1u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F2E) == 3u && FirmwareReadBit000B() != 0u)
    {
        DispatchBuildRawSectorDescriptorScratch(0u, 0u);
        return 1u;
    }

    DispatchServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x1105u);
    return 0u;
}

/* RAM:f921f0 */
u8 PLEXTOR_NEAR_FUNCTION BuildOrQueueRawSectorDescriptorForReadPhase1(void)
{
    u16 countdown_limit;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 1u) != 0u)
    {
        if(FirmwareReadBit0004() != 0u)
            DispatchBuildRawSectorDescriptorScratch(0x1803u, 1u);
        else
            DispatchBuildRawSectorDescriptorScratch(0u, 0u);
        return 1u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) & 8u) != 0u)
    {
        DispatchServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x1105u);
        return 0u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F27) & 0x40u) != 0u)
    {
        countdown_limit = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 10u);
        if((s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) >= (s16)countdown_limit)
        {
            DispatchServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x1106u);
            return 0u;
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) & 0xc0u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) & 0x20u) == 0u)
    {
        DispatchPhase1DescriptorCountdownResetProbe();
        DispatchServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x1105u);
        return 0u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) & 0x10u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F30) & 0x40u) != 0u)
        DispatchBuildRawSectorDescriptorScratch(0x1804u, 1u);
    else
        DispatchBuildRawSectorDescriptorScratch(0u, 0u);
    return 1u;
}

/* RAM:f9230d */
u8 PLEXTOR_NEAR_FUNCTION BuildOrQueueRawSectorDescriptorForReadPhase2(void)
{
    u16 countdown_limit;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3C) & 1u) != 0u)
    {
        if(FirmwareReadBit0004() != 0u)
            DispatchBuildRawSectorDescriptorScratch(0x1803u, 1u);
        else
            DispatchBuildRawSectorDescriptorScratch(0u, 0u);
        return 1u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) & 8u) != 0u)
    {
        DispatchServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x1105u);
        return 0u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F27) & 0x40u) != 0u)
    {
        countdown_limit = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 10u);
        if((s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) >= (s16)countdown_limit)
        {
            DispatchServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x1106u);
            return 0u;
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) & 0xc0u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) & 0x20u) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u)
        {
            u8 index;
            u8 port_sum = 0u;

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, 5u);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 5u);
            for(index = 0u; index != 4u; ++index)
            {
                u8 value = FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018004 + index);

                FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A4 + index, value);
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FCA, 4u);
            FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018002, 4u);
            for(index = 0u; index != 4u; ++index)
                port_sum = (u8)(port_sum + FirmwareReadNear8(FW_NEAR_UNKNOWN_15A4 + index));
            if(port_sum == 0u)
            {
                DispatchBuildRawSectorDescriptorScratch(0u, 0u);
                return 1u;
            }
        }

        DispatchServiceRawReadCountdownAndEnqueueDescriptor(3u, 0x1105u);
        return 0u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0F31) & 0x10u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0F30) & 0x40u) != 0u)
        DispatchBuildRawSectorDescriptorScratch(0x1804u, 1u);
    else
        DispatchBuildRawSectorDescriptorScratch(0u, 0u);
    return 1u;
}

/* RAM:f9241f */
void PLEXTOR_NEAR_FUNCTION ServiceParsedQRawReadCountdownAndEnqueueDescriptor(u16 base_descriptor_status)
{
    u8 countdown;

    DispatchInitializeControllerBufferCursorForParsedQCountdown();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) - 1u));

    if((FirmwareReadBit001B() != 0u || (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u) && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 20)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
    {
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F8F, 0ul);
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F8B, 0ul);
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F87, 0ul);
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F83, 0ul);
    }

    if(FirmwareReadBit0011() != 0u)
    {
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 20)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0u);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= 8u)
            return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F07) == 0u && FirmwareReadBit000B() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F08, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F08) - 1u));
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F08) <= 0)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F23, 0u);
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 0)
    {
        if(FirmwareReadBit0020() != 0u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u)
        {
            countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA2);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA2, (u8)(countdown - 1u));
            if((s8)countdown > 0)
            {
                DispatchInitializeRawReadOperatingSpeedForParsedQCountdown();
                return;
            }
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA2, 10u);
        if(DispatchCheckP1Bit3StableForParsedQCountdown() != 0u)
        {
            DispatchBuildParsedQDescriptorForCountdown(base_descriptor_status);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (s32)ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598) >= 0
                && (s32)ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598) <= 450 && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1245) & 0x40u) == 0u)
            {
                FirmwareWriteBit0016(1u);
                FirmwareWriteBit0017(1u);
            }
        }
        else
        {
            if((s32)ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598) >= 0 && (s32)ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598) <= 450)
            {
                FirmwareWriteBit0017(1u);
                FirmwareWriteBit0016(0u);
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
            DispatchBuildRawSectorDescriptorScratch(0x2100u, 5u);
        }

        DispatchEnqueueDescriptorAtScratchAddressForParsedQCountdown();
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
        {
            FirmwareWriteBit0009(1u);
        }
        else
        {
            WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598, ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598) + 1ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9C, 0u);
        }
        return;
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 16)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 3u);
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) <= 5)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 1u);
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x20u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, 0x20u);
    }
}

/* RAM:f928ae */
void PLEXTOR_NEAR_FUNCTION EnqueueRawSectorDescriptorWithTimingGate(u16 base_descriptor_status, u8 descriptor_mode)
{
    u8 threshold;

    DispatchInitializeControllerBufferCursorForTimingGate();
    if(FirmwareReadBit0013() == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u && FirmwareReadBit001B() == 0u)
    {
        if(FirmwareReadBit0020() != 0u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u)
        {
            u8 capacity_countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA2);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA2, (u8)(capacity_countdown - 1u));
            if((s8)capacity_countdown > 0)
            {
                DispatchInitializeRawReadOperatingSpeedForTimingGate();
                return;
            }
        }

        threshold = 10u;
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u)
            threshold = 0u;
        if((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) >= (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - (u16)threshold))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) - 1u));
            return;
        }
    }

    DispatchBuildRawSectorDescriptorScratch(base_descriptor_status, descriptor_mode);
    DispatchEnqueueDescriptorAtScratchAddressForTimingGate();
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
    {
        FirmwareWriteBit0009(1u);
    }
    else
    {
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598, ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598) + 1ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9C, 0u);
    }
}

/* RAM:f925be */
void PLEXTOR_NEAR_FUNCTION ServiceRawReadCountdownAndEnqueueDescriptor(u8 raw_sector_source_kind, u16 base_descriptor_status)
{
    u8 countdown;

    DispatchInitializeControllerBufferCursorForGeneralCountdown();
    FirmwareWriteBit0004(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) - 1u));
    FirmwareWriteBit0026(0u);

    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u && (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) <= (s16)(u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 5u))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
    {
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F8F, 0ul);
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F8B, 0ul);
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F87, 0ul);
        WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_0F83, 0ul);
    }

    if(FirmwareReadBit0011() != 0u)
    {
        if((s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) <= (s16)(u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 10u))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, 0u);
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= 8u)
        {
            FirmwareWriteBit0004(1u);
            FirmwareWriteBit0026(0u);
            return;
        }
    }

    if(FirmwareReadBit0024() == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
    {
        if((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D) - 1u) == FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24))
            FirmwareWriteBit0024((u8)(raw_sector_source_kind == 3u && base_descriptor_status == 0x1105u));
    }
    else if(raw_sector_source_kind == 3u && base_descriptor_status == 0x1105u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) >= 5u)
        {
            u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);

            if(operating_code != FirmwareReadNear8(FW_NEAR_UNKNOWN_0F3A) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= 8u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) <= 19u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, 0u);
                FirmwareWriteBit0024(0u);
            }
        }
    }
    else
    {
        FirmwareWriteBit0024(0u);
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) <= 0)
    {
        if(FirmwareReadBit0020() != 0u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u)
        {
            countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA2);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA2, (u8)(countdown - 1u));
            if((s8)countdown > 0)
            {
                DispatchInitializeRawReadOperatingSpeedForGeneralCountdown();
                return;
            }
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA2, 10u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
        DispatchBuildRawSectorDescriptorScratch(base_descriptor_status, raw_sector_source_kind);
        DispatchEnqueueDescriptorAtScratchAddressForGeneralCountdown();
        if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 0u)
        {
            FirmwareWriteBit0009(1u);
            FirmwareWriteBit0004(0u);
        }
        else
        {
            WriteBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598, ReadBigEndianU32ForDescriptorCountdown(FW_NEAR_UNKNOWN_1598) + 1ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F24, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3D));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2C, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F9C, 0u);
        }
        return;
    }

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) <= 16)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 3u);
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F24) <= 5)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F05, 1u);
    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x20u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 0x20u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, 0x20u);
    }
}
