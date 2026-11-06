#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include "boot/startup_progress_diagnostics.h"

u8 StartupDiagnosticInterruptEnable;
extern volatile u8 StartupDiagnosticStage;
extern volatile u16 StartupDiagnosticTicks;
extern volatile u8 StartupDiagnosticArmed;
static unsigned reports;
static u8 reported_stage;

void ObserveStartupDiagnosticHalt(u8 stage)
{
    assert(StartupDiagnosticInterruptEnable == 0u);
    assert(StartupDiagnosticArmed == 0u);
    ++reports;
    reported_stage = stage;
}

int main(void)
{
    unsigned i;
    STARTUP_DIAGNOSTIC_POLL_DECLARE();

    StartupDiagnosticInterruptEnable = 1u;
    STARTUP_DIAGNOSTIC_ARM();
    assert(StartupDiagnosticInterruptEnable == 1u);
    STARTUP_DIAGNOSTIC_MARK(4u);
    STARTUP_DIAGNOSTIC_MARK(3u);
    assert(StartupDiagnosticStage == 4u);
    StartupDiagnosticInterruptEnable = 0u;
    STARTUP_DIAGNOSTIC_MARK(7u);
    assert(StartupDiagnosticInterruptEnable == 0u);
    for(i = 0; i < 8191u; ++i)
        STARTUP_DIAGNOSTIC_TICK();
    assert(reports == 0u);
    STARTUP_DIAGNOSTIC_TICK();
    assert(reports == 1u && reported_stage == 7u);
    STARTUP_DIAGNOSTIC_TICK();
    assert(reports == 1u);
    STARTUP_DIAGNOSTIC_ARM();
    assert(StartupDiagnosticStage == 1u && StartupDiagnosticTicks == 0u);
    for(i = 0; i < 65534u; ++i)
        STARTUP_DIAGNOSTIC_POLL();
    assert(reports == 1u);
    STARTUP_DIAGNOSTIC_POLL();
    assert(reports == 2u && reported_stage == 9u);
    puts("Startup progress interrupt-state, deadline, restart, and masked-poll checks passed");
    return 0;
}
