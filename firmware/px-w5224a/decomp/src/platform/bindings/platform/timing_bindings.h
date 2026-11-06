#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveTimer0Dispatch(u8 dispatch_kind);
extern u8 ObserveTimer0IndexedState(u8 state);
extern void ObserveTimer0Sweep(void);
#define WRITE_TIMER0_LOW(value) FirmwareWriteDirectSfr8(8A, value)
#define WRITE_TIMER0_HIGH(value) FirmwareWriteDirectSfr8(8C, value)
#define TIMER0_PHASE0() ObserveTimer0Dispatch(0u)
#define TIMER0_ODD_PHASE() ObserveTimer0Dispatch(1u)
#define TIMER0_PHASE2() ObserveTimer0Dispatch(2u)
#define TIMER0_PHASE4() ObserveTimer0Dispatch(4u)
#define TIMER0_NO_OP() ObserveTimer0Dispatch(0xffu)
#define TIMER0_INDEXED_STATE(state) ObserveTimer0IndexedState(state)
#define TIMER0_SWEEP() ObserveTimer0Sweep()
#else
#include "platform/leaf_workers.h"
#include "platform/noop_handlers.h"
#include "platform/status_mapping.h"
sfr Timer0LowRegister = 0x8a;
sfr Timer0HighRegister = 0x8c;
extern void PLEXTOR_NEAR_FUNCTION ServiceControllerWord0251SweepSampleTick(void);
#define WRITE_TIMER0_LOW(value) (Timer0LowRegister = (value))
#define WRITE_TIMER0_HIGH(value) (Timer0HighRegister = (value))
#define TIMER0_PHASE0() TimerDispatchPhase0()
#define TIMER0_ODD_PHASE() TimerDispatchOddPhase()
#define TIMER0_PHASE2() TimerDispatchPhase2()
#define TIMER0_PHASE4() TimerDispatchPhase4()
#define TIMER0_NO_OP() NoOpTimerDispatch()
#define TIMER0_INDEXED_STATE(state) IsIndexedDriveStateActive(state)
#define TIMER0_SWEEP() ServiceControllerWord0251SweepSampleTick()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
/* Target builds use timing_target.c for these timer/SFR primitives. */

/* RAM:f8f11e */
void StopAndClearTimer1(void)
{
    FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) & 0xbfu));
    FirmwareWriteDirectSfr8(8D, 0);
    FirmwareWriteDirectSfr8(8B, 0);
}

/* RAM:fa7661 */
void StartTimer1AndClearMeasurementStateBit4(void)
{
    FirmwareWriteDirectSfr8(88, (u8)(FirmwareReadDirectSfr8(88) | 0x40u));
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
void DelayTimerUnits(u16 delay_units)
{
    u8 status;
    u32 scaled_delay = ScaleTimerUnitsBy60(delay_units);
    u8 shift_count = 16;
    s16 overflow_count;
    s16 reload_value;

    status = FirmwareReadDirectSfr8(91);
    FirmwareWriteDirectSfr8(91, (u8)(status & 0xbfu));
    status = FirmwareReadDirectSfr8(91);
    FirmwareWriteDirectSfr8(91, (u8)(status & 0x7fu));
    do
    {
        scaled_delay >>= 1;
        shift_count--;
    } while(shift_count != 0);
    overflow_count = (s16)scaled_delay;
    FirmwareWriteDirectSfr8(94, 0);
    FirmwareWriteDirectSfr8(96, 0);
    status = FirmwareReadDirectSfr8(91);
    FirmwareWriteDirectSfr8(91, (u8)(status | 0x40u));
    while(overflow_count != 0)
    {
        while((s8)FirmwareReadDirectSfr8(91) >= 0)
        {
        }
        status = FirmwareReadDirectSfr8(91);
        FirmwareWriteDirectSfr8(91, (u8)(status & 0x7fu));
        overflow_count = (s16)(overflow_count - 1);
    }
    reload_value = (s16)(u16)(0u - (u16)(ScaleTimerUnitsBy60(delay_units) + 1ul));
    FirmwareWriteDirectSfr8(94, (u8)reload_value);
    FirmwareWriteDirectSfr8(96, (u8)((u16)reload_value >> 8));
    while((s8)FirmwareReadDirectSfr8(91) >= 0)
    {
    }
    status = FirmwareReadDirectSfr8(91);
    FirmwareWriteDirectSfr8(91, (u8)(status & 0xbfu));
    status = FirmwareReadDirectSfr8(91);
    FirmwareWriteDirectSfr8(91, (u8)(status & 0x7fu));
}

/* RAM:fda690 */
void DelayTimerUnits59(void)
{
    DelayTimerUnits(0x003bu);
}
#endif
#endif
