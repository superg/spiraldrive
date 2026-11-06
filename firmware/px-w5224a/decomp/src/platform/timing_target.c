#include "firmware_memory.h"
#include "mcs251.h"
#include "../boot/startup_progress_diagnostics.h"

extern void PLEXTOR_NEAR_FUNCTION ClearTimer1RunAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetTimer1RunAtomic(void);

sfr DelayTimerStatus = 0x91;
sbit DelayTimerRunBit = DelayTimerStatus ^ 6;
sbit DelayTimerOverflowBit = DelayTimerStatus ^ 7;

/* RAM:f8f11e */
void StopAndClearTimer1(void)
{
    ClearTimer1RunAtomic();
    FirmwareWriteDirectSfr8(8D, 0u);
    FirmwareWriteDirectSfr8(8B, 0u);
}

/* RAM:fa7661 */
void StartTimer1AndClearMeasurementStateBit4(void)
{
    SetTimer1RunAtomic();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xefu));
}

/* RAM:fa7cb6 */
void BusyWaitAccumulatorTimes2300(u8 scale)
{
    volatile u16 index = 0;
    u16 limit = (u16)((u16)scale * 0x08fcu);

    while(index < limit)
        index = (u16)(index + 1u);
}
/* Reconstruction helper; no direct Ghidra function address. */
static u32 ScaleTimerUnitsBy60(u16 delay_units)
{
    return ((u32)delay_units << 5) + ((u32)delay_units << 4) + ((u32)delay_units << 3) + ((u32)delay_units << 2);
}

/* RAM:fdbda2 */
#if defined(PLEXTOR_LED_HOST_DELAY_DIAGNOSTICS)
/* Select the probe per call, without persistent state across coroutines. */
static void DelayTimerUnitsWorker(u16 delay_units, u8 probe_handshake)
#else
void DelayTimerUnits(u16 delay_units)
#endif
{
    u32 scaled_delay = ScaleTimerUnitsBy60(delay_units);
    u8 shift_count = 16;
    s16 overflow_count;
    s16 reload_value;

    DelayTimerRunBit = 0;
    DelayTimerOverflowBit = 0;
    do
    {
        scaled_delay >>= 1;
        shift_count--;
    } while(shift_count != 0);
    overflow_count = (s16)scaled_delay;
    FirmwareWriteDirectSfr8(94, 0);
    FirmwareWriteDirectSfr8(96, 0);
    DelayTimerRunBit = 1;
    while(overflow_count != 0)
    {
#if defined(PLEXTOR_LED_HOST_DELAY_DIAGNOSTICS)
        if(probe_handshake != 0u)
            StartupDelayOverflowWait();
        else
#endif
        while(DelayTimerOverflowBit == 0)
        {
        }
        DelayTimerOverflowBit = 0;
        overflow_count = (s16)(overflow_count - 1);
    }
    reload_value = (s16)(u16)(0u - (u16)(ScaleTimerUnitsBy60(delay_units) + 1ul));
    FirmwareWriteDirectSfr8(94, (u8)reload_value);
    FirmwareWriteDirectSfr8(96, (u8)((u16)reload_value >> 8));
#if defined(PLEXTOR_LED_HOST_DELAY_DIAGNOSTICS)
    if(probe_handshake != 0u)
        StartupDelayOverflowWait();
    else
#endif
    while(DelayTimerOverflowBit == 0)
    {
    }
    DelayTimerRunBit = 0;
    DelayTimerOverflowBit = 0;
}

#if defined(PLEXTOR_LED_HOST_DELAY_DIAGNOSTICS)
void DelayTimerUnits(u16 delay_units)
{
    DelayTimerUnitsWorker(delay_units, 0u);
}

/* Only RunControllerStartupStatusHandshake selects the bounded wait. */
void StartupHandshakeDelay59(void)
{
    DelayTimerUnitsWorker(0x003bu, 1u);
}
#endif

/* RAM:fda690 */
void DelayTimerUnits59(void)
{
    DelayTimerUnits(0x003bu);
}
