#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveTimer1ScaledBegin(u16 duration_units, u8 yield_while_waiting);
extern void ObserveTimer1ScaledProgram(u16 reload_value);
extern u8 ObserveTimer1ScaledOverflow(void);
extern void ObserveTimer1ScaledYield(void);
#define Timer1ScaledClearOverflow() ((void)0)
#define Timer1ScaledSetRun() ((void)0)
#define Timer1ScaledClearRun() ((void)0)
#define Timer1ScaledOverflow() ObserveTimer1ScaledOverflow()
#define Timer1ScaledYield() ObserveTimer1ScaledYield()
#else
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1OverflowAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetTimer1RunAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION ClearTimer1RunAtomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadTimer1OverflowAtomic(void);
extern void YieldCoroutine1(void);
#define Timer1ScaledClearOverflow() ClearTimer1OverflowAtomic()
#define Timer1ScaledSetRun() SetTimer1RunAtomic()
#define Timer1ScaledClearRun() ClearTimer1RunAtomic()
#define Timer1ScaledOverflow() ReadTimer1OverflowAtomic()
#define Timer1ScaledYield() YieldCoroutine1()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
ObserveTimer1ScaledBegin(duration_units, yield_while_waiting);
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 3
#if defined(PLEXTOR_HOST_TEST)
ObserveTimer1ScaledProgram(reload_value);
#endif
#endif
