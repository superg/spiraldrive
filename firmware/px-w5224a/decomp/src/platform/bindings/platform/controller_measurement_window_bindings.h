#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveBeginReadRangeClearGlobalInterrupt(void);
extern void ObserveBeginReadRangeClearTimerOverflow(void);
extern void ObserveBeginReadRangeSetTimerRun(void);
extern void ObserveBeginReadRangeClearTimerRun(void);
extern u8 ObserveBeginReadRangeTimerOverflow(void);
extern void ObserveBeginReadRangeSetGlobalInterrupt(void);
extern void ObserveBeginReadRangeSetControllerInterrupt(void);
extern void ObserveBeginReadRangeSetSfrA9Bit2(void);
extern void ObserveBeginReadRangeWriteScaledProfile(u8 scale);
extern void ObserveBeginReadRangeWriteEaEb(u16 value);
extern void ObserveBeginReadRangeWriteParameterWord(u16 value);
#define BEGIN_RANGE_CLEAR_GLOBAL() ObserveBeginReadRangeClearGlobalInterrupt()
#define BEGIN_RANGE_CLEAR_TIMER_OVERFLOW() ObserveBeginReadRangeClearTimerOverflow()
#define BEGIN_RANGE_SET_TIMER_RUN() ObserveBeginReadRangeSetTimerRun()
#define BEGIN_RANGE_CLEAR_TIMER_RUN() ObserveBeginReadRangeClearTimerRun()
#define BEGIN_RANGE_TIMER_OVERFLOW() ObserveBeginReadRangeTimerOverflow()
#define BEGIN_RANGE_SET_GLOBAL() ObserveBeginReadRangeSetGlobalInterrupt()
#define BEGIN_RANGE_SET_CONTROLLER() ObserveBeginReadRangeSetControllerInterrupt()
#define BEGIN_RANGE_SET_SFR_A9_BIT2() ObserveBeginReadRangeSetSfrA9Bit2()
#define BEGIN_RANGE_WRITE_SCALED_PROFILE(scale) ObserveBeginReadRangeWriteScaledProfile(scale)
#define BEGIN_RANGE_WRITE_EA_EB(value) ObserveBeginReadRangeWriteEaEb(value)
#define BEGIN_RANGE_WRITE_PARAMETER(value) ObserveBeginReadRangeWriteParameterWord(value)
#else
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1OverflowAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetTimer1RunAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1RunAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadTimer1OverflowAtomic(void);
#define BEGIN_RANGE_CLEAR_GLOBAL() ClearBit0547Atomic()
#define BEGIN_RANGE_CLEAR_TIMER_OVERFLOW() ClearTimer1OverflowAtomic()
#define BEGIN_RANGE_SET_TIMER_RUN() SetTimer1RunAtomic()
#define BEGIN_RANGE_CLEAR_TIMER_RUN() ClearTimer1RunAtomic()
#define BEGIN_RANGE_TIMER_OVERFLOW() ReadTimer1OverflowAtomic()
#define BEGIN_RANGE_SET_GLOBAL() SetBit0547Atomic()
#define BEGIN_RANGE_SET_CONTROLLER() SetBit0546Atomic()
#define BEGIN_RANGE_SET_SFR_A9_BIT2() SetSfrA9Bit2Atomic()
#define BEGIN_RANGE_WRITE_SCALED_PROFILE(scale) WriteScaledOperatingProfileWordViaSfr8B8D(scale)
#define BEGIN_RANGE_WRITE_EA_EB(value) WriteControllerParametersEAEBFromScaledValue(value)
#define BEGIN_RANGE_WRITE_PARAMETER(value) WriteControllerParameterWord(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadBit003AForProcessedSfrDbWindow(void);
#define READ_OFFSET_ENABLE_BIT() ObserveReadBit003AForProcessedSfrDbWindow()
#else
extern u8 ReadBit003AForProcessedSfrDbWindow(void);
#define READ_OFFSET_ENABLE_BIT() ReadBit003AForProcessedSfrDbWindow()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveClearReadMeasurementBit0028(void);
extern void ObserveClearReadMeasurementWindowBits(void);
extern u8 ObserveReadBit0032ForReadMeasurementReset(void);
extern void ObserveClearBit0032ForReadMeasurementReset(void);
extern void ObserveClearGlobalInterruptForReadMeasurementReset(void);
extern void ObserveSetGlobalInterruptForReadMeasurementReset(void);
extern void ObserveSetControllerInterruptForReadMeasurementReset(void);
extern void ObserveSetSfrA9Bit2ForReadMeasurementReset(void);
#define CLEAR_READ_MEASUREMENT_BIT0028() ObserveClearReadMeasurementBit0028()
#define CLEAR_READ_MEASUREMENT_WINDOW_BITS() ObserveClearReadMeasurementWindowBits()
#define READ_READ_MEASUREMENT_BIT0032() ObserveReadBit0032ForReadMeasurementReset()
#define CLEAR_READ_MEASUREMENT_BIT0032() ObserveClearBit0032ForReadMeasurementReset()
#define CLEAR_READ_MEASUREMENT_GLOBAL_INTERRUPT() ObserveClearGlobalInterruptForReadMeasurementReset()
#define SET_READ_MEASUREMENT_GLOBAL_INTERRUPT() ObserveSetGlobalInterruptForReadMeasurementReset()
#define SET_READ_MEASUREMENT_CONTROLLER_INTERRUPT() ObserveSetControllerInterruptForReadMeasurementReset()
#define SET_READ_MEASUREMENT_SFR_A9_BIT2() ObserveSetSfrA9Bit2ForReadMeasurementReset()
#else
extern void PLEXTOR_NEAR_FUNCTION ClearReadMeasurementBit0028Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearReadMeasurementWindowBitsAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0032ForReadMeasurementReset(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0032ForReadMeasurementResetAtomic(void);
extern void ClearBit0547Atomic(void);
extern void SetBit0547Atomic(void);
extern void SetBit0546Atomic(void);
extern void SetSfrA9Bit2Atomic(void);
#define CLEAR_READ_MEASUREMENT_BIT0028() ClearReadMeasurementBit0028Atomic()
#define CLEAR_READ_MEASUREMENT_WINDOW_BITS() ClearReadMeasurementWindowBitsAtomic()
#define READ_READ_MEASUREMENT_BIT0032() ReadBit0032ForReadMeasurementReset()
#define CLEAR_READ_MEASUREMENT_BIT0032() ClearBit0032ForReadMeasurementResetAtomic()
#define CLEAR_READ_MEASUREMENT_GLOBAL_INTERRUPT() ClearBit0547Atomic()
#define SET_READ_MEASUREMENT_GLOBAL_INTERRUPT() SetBit0547Atomic()
#define SET_READ_MEASUREMENT_CONTROLLER_INTERRUPT() SetBit0546Atomic()
#define SET_READ_MEASUREMENT_SFR_A9_BIT2() SetSfrA9Bit2Atomic()
#endif
#endif
