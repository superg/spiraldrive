#include "startup_progress_diagnostics.h"

#if defined(PLEXTOR_HOST_TEST)
extern u8 StartupDiagnosticInterruptEnable;
extern void ObserveStartupDiagnosticHalt(u8 stage);
#else
sbit StartupDiagnosticInterruptEnable = 0xaf;
#endif
extern void LedBootDiagnosticCheckpoint(u8 checkpoint);

/* Memory, not register locals: the coroutine ABI replaces DR12 on resume. */
volatile u8 StartupDiagnosticStage;
volatile u16 StartupDiagnosticTicks;
volatile u8 StartupDiagnosticArmed;

void StartupDiagnosticArm(void)
{
    u8 enabled = StartupDiagnosticInterruptEnable;
    StartupDiagnosticInterruptEnable = 0;
    StartupDiagnosticArmed = 0u;
    StartupDiagnosticStage = 1u;
    StartupDiagnosticTicks = 0u;
    StartupDiagnosticArmed = 1u;
    StartupDiagnosticInterruptEnable = enabled;
}

void StartupDiagnosticMark(u8 stage)
{
    u8 enabled = StartupDiagnosticInterruptEnable;
    StartupDiagnosticInterruptEnable = 0;
    if(StartupDiagnosticStage < stage)
        StartupDiagnosticStage = stage;
    StartupDiagnosticInterruptEnable = enabled;
}

void StartupDiagnosticHalt(u8 stage)
{
    StartupDiagnosticInterruptEnable = 0;
    StartupDiagnosticArmed = 0u;
#if defined(PLEXTOR_HOST_TEST)
    ObserveStartupDiagnosticHalt(stage);
#else
    for(;;)
        LedBootDiagnosticCheckpoint(stage);
#endif
}

void StartupDiagnosticTick(void)
{
    if(StartupDiagnosticArmed == 0u)
        return;
    ++StartupDiagnosticTicks;
    if(StartupDiagnosticTicks >= 8192u)
        StartupDiagnosticHalt(StartupDiagnosticStage);
}
