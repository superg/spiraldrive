#include "write_speed_reduction.h"
#include "firmware_memory.h"
#include "status_mapping.h"

static u16 ReadWriteSpeedFlags(void)
{
    return (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_FAD7);
}

static void WriteWriteSpeedFlags(u16 flags)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD7, (u8)(flags >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD8, (u8)flags);
}

static void SaveAndApplyReducedWriteSpeed(u8 speed)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD3, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, speed);
}

/* RAM:f89dda */
u8 TryReduceWriteSpeedForCurrentMediaState(u8 enabled)
{
    u8 current_speed;
    u8 replacement_speed;
    u8 media_strategy;
    u16 flags;

    if(enabled == 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 16u)
                return 0u;
            flags = (u16)(ReadWriteSpeedFlags() | 0x0004u);
            WriteWriteSpeedFlags(flags);
            flags = (u16)(ReadWriteSpeedFlags() | 0x1000u);
            WriteWriteSpeedFlags(flags);
            SaveAndApplyReducedWriteSpeed(8u);
            return 1u;
        }

        replacement_speed = SelectLowestEnabledWriteSpeed();
        current_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
        if(current_speed <= replacement_speed)
            return 0u;
        flags = ReadWriteSpeedFlags();
        if((flags & 0x0004u) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD3, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C38));
        WriteWriteSpeedFlags((u16)(flags | 0x0004u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, replacement_speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, replacement_speed);
        return 1u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u)
    {
        current_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
        if(current_speed < 24u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
            return 0u;
        WriteWriteSpeedFlags((u16)(ReadWriteSpeedFlags() | 0x0004u));
        media_strategy = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D);
        if(media_strategy == 0xd7u || media_strategy == 0xd4u)
        {
            replacement_speed = 20u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x7fu));
        }
        else if(media_strategy == 0xd8u)
        {
            replacement_speed = 16u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x7fu));
        }
        else
        {
            replacement_speed = 24u;
        }
        SaveAndApplyReducedWriteSpeed(replacement_speed);
        return 1u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
    {
        current_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
        if(current_speed < 32u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
            return 0u;
        WriteWriteSpeedFlags((u16)(ReadWriteSpeedFlags() | 0x0004u));
        media_strategy = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D);
        if(media_strategy == 0xe3u || media_strategy == 0xe4u || media_strategy == 0xe2u)
        {
            replacement_speed = 24u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x7fu));
        }
        else
        {
            replacement_speed = 32u;
        }
        SaveAndApplyReducedWriteSpeed(replacement_speed);
        return 1u;
    }

    return 0u;
}
