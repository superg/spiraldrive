#include "timer_debounce.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/timer_debounce_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static void SetPatternOutputLines(void)
{
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) > (s8)10)
    {
        ClearP3Bit0Atomic();
        SetP3Bit1Atomic();
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0x01u) != 0u)
    {
        ClearP3Bit0Atomic();
        SetP3Bit1Atomic();
    }
    else
    {
        SetP3Bit0Atomic();
        ClearP3Bit1Atomic();
    }
}

/* Reconstruction helper; no direct Ghidra function address. */
static void SetPatternOutputLinesHigh(void)
{
    SetP3Bit0Atomic();
    SetP3Bit1Atomic();
}

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/timer_debounce_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u32 ReadBigEndianU32Near(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianU32Near(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:f8e90d */
void UpdateP3Bits0And1PatternState(u8 restart_pattern)
{
    u8 phase;
    u8 remaining;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 0u)
        return;
    if(restart_pattern == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CD, 0u);
        return;
    }

    phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_11CD);
    if(phase == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CE, FirmwareReadNear8(FW_NEAR_UNKNOWN_1009));
        SetPatternOutputLines();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 0x1eu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CD) + 1u));
    }
    else if(phase == 1u)
    {
        SetPatternOutputLines();
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121A) <= (s8)0)
        {
            SetPatternOutputLinesHigh();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CD) + 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 0x1eu);
        }
    }
    else if(phase == 2u)
    {
        SetPatternOutputLinesHigh();
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121A) <= (s8)0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CE) - 1u));
            remaining = FirmwareReadNear8(FW_NEAR_UNKNOWN_11CE);
            if(remaining == 0u || (remaining == 10u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) >= 11u))
            {
                SetPatternOutputLinesHigh();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CD) + 1u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 0x64u);
            }
            else
            {
                SetPatternOutputLines();
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CD, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11CD) - 1u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 0x1eu);
            }
        }
    }
    else if(phase == 3u)
    {
        SetPatternOutputLinesHigh();
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_121A) <= (s8)0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CE, FirmwareReadNear8(FW_NEAR_UNKNOWN_1009));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_121A, 0x1eu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11CD, 1u);
            SetPatternOutputLines();
        }
    }
}

/* RAM:fd1e19 */
void DebounceP0Bit7State(void)
{
    if(FirmwareReadBit0407() != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x01u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1222, 0x0fu);
        else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1222) <= (s8)0)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 0x01u));
    }
    else
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x01u) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1222, 0x0fu);
        else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1222) <= (s8)0)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xfeu));
    }
}

/* RAM:fd1e66 */
void DebounceControllerTransactionStatusBit0(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_80F8) & 0x01u) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x02u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1223, 0x19u);
        }
        else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1223) <= (s8)0)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 0x02u));
            WriteBigEndianU32Near(FW_NEAR_UNKNOWN_11F4, 0ul);
        }
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x02u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1223, 0x19u);
    }
    else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1223) <= (s8)0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0xfdu));
        WriteBigEndianU32Near(FW_NEAR_UNKNOWN_FBED, ReadBigEndianU32Near(FW_NEAR_UNKNOWN_FBED) + ReadBigEndianU32Near(FW_NEAR_UNKNOWN_11F4));
    }
}

/* RAM:fd15e2 */
void ServiceControllerChannel2GateTimeoutTick(void)
{
    u8 active_value;
    u8 tick_count;

    if(FirmwareReadBit0407() != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1225) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));
            WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x02u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) == 1u)
                ConfigureControllerPorts8496Through8499ForOperatingRange();
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1229, 0u);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1229, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1229) + 1u));
    tick_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_1229);
    if(tick_count >= 9u)
    {
        active_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_1225);
        if(active_value != 0u && active_value != 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u || tick_count >= 0x28u))
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 5u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) >> 1) & 0x03u) == 3u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 0u);
                WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, 0u);
                WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0u);
            }
        }
    }
    active_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_1225);
    if(active_value == 0u || active_value == 2u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1229, 0u);
}

/* RAM:fd1d1e */
void PLEXTOR_NEAR_FUNCTION ServiceP1Bit6DriveStateTransition(void)
{
    u8 media_state;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x40u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAED, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAED) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 0x20u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C35) != 0u)
    {
        media_state = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);
        if(media_state == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x04u));
        }
        else
        {
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_122D) > 0)
            {
                FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 0x10u));
                return;
            }

            if(media_state == 1u || media_state == 3u)
            {
                FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x04u));
                if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x01u) != 0u)
                    FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 1u);
            }
            else if(media_state == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS, (u8)(FirmwareReadNear8(FW_NEAR_ACTIVE_OPERATION_FLAGS) | 0x04u));
                FirmwareWrite8(FW_HUGE_ACTIVE_OPERATION_PENDING_LATCH, 1u);
            }
        }
    }

    if(IsIndexedDriveStateActive(0x0cu) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1309) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11E5, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11E5) | 0x04u));
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 0x40u));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0x40u) != 0u)
                SetDriveTransitionOutputLinesHigh();
        }
    }
}
