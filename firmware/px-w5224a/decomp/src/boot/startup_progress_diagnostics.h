#ifndef PLEXTOR_STARTUP_PROGRESS_DIAGNOSTICS_H
#define PLEXTOR_STARTUP_PROGRESS_DIAGNOSTICS_H

#include "../platform/mcs251.h"

/* Diagnostic only: halt and repeat the last startup stage after 8192 Timer0
 * entries. A bounded controller-ready poll reports 9 if interrupts are masked.
 * No code or storage is added to ordinary firmware builds. */
#if defined(PLEXTOR_LED_HOST_PHASE_DIAGNOSTICS)
void StartupHandshakePhaseHalt(u8 count);
#define STARTUP_DIAGNOSTIC_ARM() do { } while(0)
#define STARTUP_DIAGNOSTIC_MARK(stage) do { } while(0)
#define STARTUP_DIAGNOSTIC_TICK() do { } while(0)
#define STARTUP_DIAGNOSTIC_POLL_DECLARE()
#define STARTUP_DIAGNOSTIC_POLL() do { } while(0)
#elif defined(PLEXTOR_LED_HOST_DELAY_DIAGNOSTICS)
void StartupHandshakeDelay59(void);
void StartupDelayOverflowWait(void);
void StartupHandshakeDelayCompleted(void);
#define STARTUP_HANDSHAKE_DELAY_DONE() StartupHandshakeDelayCompleted()
#define STARTUP_DIAGNOSTIC_ARM() do { } while(0)
#define STARTUP_DIAGNOSTIC_MARK(stage) do { } while(0)
#define STARTUP_DIAGNOSTIC_TICK() do { } while(0)
#define STARTUP_DIAGNOSTIC_POLL_DECLARE()
#define STARTUP_DIAGNOSTIC_POLL() do { } while(0)
#elif defined(PLEXTOR_LED_HOST_ROUTE_DIAGNOSTICS)
void StartupRouteInitialEntry(void);
void StartupRouteOtherEntry(void);
#define STARTUP_ROUTE_CHECK() StartupRouteOtherEntry()
#define STARTUP_DIAGNOSTIC_ARM() do { } while(0)
#define STARTUP_DIAGNOSTIC_MARK(stage) do { } while(0)
#define STARTUP_DIAGNOSTIC_TICK() do { } while(0)
#define STARTUP_DIAGNOSTIC_POLL_DECLARE()
#define STARTUP_DIAGNOSTIC_POLL() do { } while(0)
#elif defined(PLEXTOR_LED_HOST_INTERVAL_DIAGNOSTICS)
void StartupHostIntervalCheckpoint(u8 point);
#define STARTUP_INTERVAL_CHECK(point) StartupHostIntervalCheckpoint(point)
#define STARTUP_DIAGNOSTIC_ARM() do { } while(0)
#define STARTUP_DIAGNOSTIC_MARK(stage) do { } while(0)
#define STARTUP_DIAGNOSTIC_TICK() do { } while(0)
#define STARTUP_DIAGNOSTIC_POLL_DECLARE()
#define STARTUP_DIAGNOSTIC_POLL() do { } while(0)
#elif defined(PLEXTOR_LED_HOST_ZERO_DIAGNOSTICS)
void StartupHostZeroCheckpoint(u8 point);
#define STARTUP_HOST_PACKET_CHECK(point) StartupHostZeroCheckpoint(point)
#define STARTUP_DIAGNOSTIC_ARM() do { } while(0)
#define STARTUP_DIAGNOSTIC_MARK(stage) do { if((stage) >= 4u) StartupHostZeroCheckpoint((stage) - 1u); } while(0)
#define STARTUP_DIAGNOSTIC_TICK() do { } while(0)
#define STARTUP_DIAGNOSTIC_POLL_DECLARE()
#define STARTUP_DIAGNOSTIC_POLL() do { } while(0)
#elif defined(PLEXTOR_LED_HOST_GATE_DIAGNOSTICS)
/* No early startup delay. Freeze one snapshot at media-state entry, then
 * report forever using registers/current stack only. */
void StartupHostGateCheckpoint(void);
#define STARTUP_DIAGNOSTIC_ARM() do { } while(0)
#define STARTUP_DIAGNOSTIC_MARK(stage) do { if((stage) == 8u) StartupHostGateCheckpoint(); } while(0)
#define STARTUP_DIAGNOSTIC_TICK() do { } while(0)
#define STARTUP_DIAGNOSTIC_POLL_DECLARE()
#define STARTUP_DIAGNOSTIC_POLL() do { } while(0)
#elif defined(PLEXTOR_LED_FOREGROUND_DIAGNOSTICS)
/* Addressless diagnostic adapter: register/stack-only delay, no timer or
 * compiler XDATA state. Every call emits its literal stage and returns. */
void StartupForegroundCheckpoint(u8 stage);
#define STARTUP_DIAGNOSTIC_ARM() StartupForegroundCheckpoint(1u)
#define STARTUP_DIAGNOSTIC_MARK(stage) StartupForegroundCheckpoint(stage)
#define STARTUP_DIAGNOSTIC_TICK() do { } while(0)
#define STARTUP_DIAGNOSTIC_POLL_DECLARE()
#define STARTUP_DIAGNOSTIC_POLL() do { } while(0)
#elif defined(PLEXTOR_LED_STARTUP_DIAGNOSTICS)
void StartupDiagnosticArm(void);
void StartupDiagnosticMark(u8 stage);
void StartupDiagnosticTick(void);
void StartupDiagnosticHalt(u8 stage);
#define STARTUP_DIAGNOSTIC_ARM() StartupDiagnosticArm()
#define STARTUP_DIAGNOSTIC_MARK(stage) StartupDiagnosticMark(stage)
#define STARTUP_DIAGNOSTIC_TICK() StartupDiagnosticTick()
#define STARTUP_DIAGNOSTIC_POLL_DECLARE() u16 diagnostic_polls = 0u
#define STARTUP_DIAGNOSTIC_POLL() do { if(++diagnostic_polls == 0xffffu) StartupDiagnosticHalt(9u); } while(0)
#else
#define STARTUP_DIAGNOSTIC_ARM() do { } while(0)
#define STARTUP_DIAGNOSTIC_MARK(stage) do { } while(0)
#define STARTUP_DIAGNOSTIC_TICK() do { } while(0)
#define STARTUP_DIAGNOSTIC_POLL_DECLARE()
#define STARTUP_DIAGNOSTIC_POLL() do { } while(0)
#endif

#ifndef STARTUP_HOST_PACKET_CHECK
#define STARTUP_HOST_PACKET_CHECK(point) do { } while(0)
#endif
#ifndef STARTUP_INTERVAL_CHECK
#define STARTUP_INTERVAL_CHECK(point) do { } while(0)
#endif
#ifndef STARTUP_ROUTE_CHECK
#define STARTUP_ROUTE_CHECK() do { } while(0)
#endif
#ifndef STARTUP_HANDSHAKE_DELAY_DONE
#define STARTUP_HANDSHAKE_DELAY_DONE() do { } while(0)
#endif

#endif
