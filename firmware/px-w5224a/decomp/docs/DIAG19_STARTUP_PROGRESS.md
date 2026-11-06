# Diag19: report startup progress without Windows detection

## Physical result motivating this diagnostic

The user reports diag18 does not enumerate in Windows and the tray stays
closed. The read-only PnP snapshot
`out/agent-tests/diag18-pnp-20260926-135121.json` contains neither PX-W5224A nor
USB bridge 093b:0042; the two other Plextor drives in that snapshot are unrelated.
Installed diag18 bytes have not been read back. No new flash/reset/activation
command was sent by the agent.

## Additional simulation evidence

`tests/simulator/cases/state0-startup-timer1-continuation.json` reaches the real
`ProcessDriveStateMachine` entry from reset with the post-initialization flag
RAM:0000fb=1. It starts drive state zero and executes the reset/homing,
drive-control sequence, coroutine yield/restart, and startup-media initialization.
Timer1 waits execute against simulated Timer1 mode 1. No function stubs or
code patches are used. Timer0 countdown values and controller responses remain
explicit model inputs. It compares all captured near/controller RAM deltas
except compiler-specific coroutine stack frames; drive state 08/30 and saved
coroutine1 SPX 00000507 are asserted. Evidence:
`out/agent-tests/state0-real-startup-media/` and
`out/agent-tests/state0-timer1-maintained-retry/`. An initial replay exceeded
its 30-second uVision wall limit; the sequential retry passes.

The exploratory real-Timer0 run also reaches this checkpoint on both images
(stock 37,798,047 simulator states; rebuilt 37,969,473), with the same drive
state and saved SPX. It supplies persistent external PORT1=7b, controller
responses, TMOD=11, and a debugger-only vector patch from generic S251 Timer0
at RAM:ff000b to the captured router RAM:ff3fe0 (selector RAM:0000fc=1).
It no longer substitutes countdown RAM. This is simulator wiring, not an
image correction or proof of the physical interrupt source.

This run **mismatches** and is not accepted as equivalent. Differences include
timer/counter bytes and RAM:018087/01808a (stock 00/00, rebuilt 80/01).
Their cause is unresolved. Stock write tracing confirms resets to zero at
RAM:f9c4b5 between ready-port writes at RAM:f8a7f9. Do not dismiss the
controller differences as timing without a discriminating follow-up.

The scratch aggregate reader logs each response identity once and separately
counts reads, capped at 100,000 per address; it does not compare every read
event. Counts for 018015/D7/0180ac/0180a9 are stock 1921/100/2/1, rebuilt
15897/100/2/1. Reproducer and preserved evidence:
`out/agent-tests/run_timer0_aggregate_probe.py`, `state0-homing.json`,
`state0-timer0-aggregate-long/`, and `state0-timer0-port-writes/`.
Run the probe script on `tools/run_on_private_desktop.py`.

## Diagnostic behavior

Enable only `PLEXTOR_LED_STARTUP_DIAGNOSTICS`; ordinary firmware and the older
INQUIRY-stop diagnostic remain separate. Progress resides in explicitly
initialized volatile memory, not scheduler register locals. After 8,192 main
Timer0 entries the reporter disables interrupts and repeats the latest stage.
The two controller-ready helpers also report 9 after 65,535 unsuccessful
polls, covering their interrupt-masked wait. This timeout is diagnostic
instrumentation, not a stock behavior change accepted for normal firmware.

Each frame is a long yellow marker followed by N short green flashes:

| N | Latest reached boundary |
|---|---|
| 1 | Main loop entered; initialization not yet returned |
| 2 | Initialization path returned or was skipped; coroutine configuration not yet returned |
| 3 | Initial coroutines configured |
| 4 | Drive-initialization coroutine entered; drive-control result not yet returned |
| 5 | Drive-control initialization returned |
| 6 | Startup-media initialization entered |
| 7 | Startup-media initialization returned |
| 8 | Media state machine entered |
| 9 | Controller-ready helper exceeded its diagnostic poll bound |

Stages 1..8 are monotonic within a main-loop entry. They establish that a
boundary was reached, not which function is executing at the later deadline.
Stage 9 overrides the displayed count. If interrupts stop in a different wait,
the reporter may never run. No pulses are not proof of a particular crash.
The reporter intentionally stops firmware operation; Windows detection is not
the pass criterion for this image. Leave the tray empty, power on normally,
allow up to a minute, and record the repeating green count and tray behavior.
Recovery remains Eject held during power-on, then redumper stock restore.

The diagnostic borrows 256 bytes of unused constant-region capacity for code:
ECODE ends RAM:feb5ff, HCONST begins RAM:feb600. The monitor, fixed data,
vectors, CODE helpers, and XDATA bounds retain their normal boundaries.
This remains a normal-size flashable diagnostic, not the expanded non-flashable
layout option. Normal firmware retains its original linker classes.

## Validation and reproduction

Host checks cover the exact 8,192-tick boundary, monotonic stages, preservation
of the caller's interrupt-enable state, restart initialization, disarming, and
the masked-poll limit. Assertions remain enabled in Release.

Two target S251 probes reach `LedBootDiagnosticCheckpoint` with interrupts
disabled and the reporter disarmed: the timer path reports stage 4, and the
forced ready-port stall reports 9. The timer probe sets the diagnostic counter
to 8,191 after two million simulator states; it checks the next real ISR and
reporting path, not the full elapsed deadline. A first unaccelerated probe
exceeded its 45-second wall limit and is not a firmware failure.
Probe: `out/agent-tests/check_diag19_target.py`; results under
`out/agent-tests/diag19-target-probes/`. Run the entire script on the private
desktop. The regular build with both LED options OFF is byte-identical to
diag18 (SHA-256 d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e).

```powershell
cmake -S . -B out/agent-tests/diag19-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_STARTUP_DIAGNOSTICS=ON -DPLEXTOR_LED_BOOT_DIAGNOSTICS=OFF
cmake --build out/agent-tests/diag19-build --config Release --target firmware --parallel 4
ctest --test-dir out/windows -C Release -R '^startup_progress_diagnostics_tests$' --output-on-failure
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag19_target.py
python out/agent-tests/run_state0_timer1_continuation.py state0-timer1-new-run
```

Artifact:
`out/agent-tests/diag19-startup-progress-20260926/PXW5224A_diag19_startup_progress.bin`.
SHA-256 `f409e2f218d7b8f0df927e132bc49db2938267552e7e07f66ef1bc73d7955c87`;
524,288 bytes; both checksum expressions 324d; stock fixed-data validation passes.
Physical result: the user reports no reaction and both LEDs continuously on;
no repeating frame was observed. Installed bytes were not read back. This does
not identify the stopping point. Diag20 emits foreground checkpoints without
timer or diagnostic-XDATA dependencies; see `DIAG20_FOREGROUND_CHECKPOINTS.md`.
