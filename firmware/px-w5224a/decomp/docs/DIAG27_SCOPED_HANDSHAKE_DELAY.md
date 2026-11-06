# Diag27: scope the delay probe to the host handshake

Physical diag26 reports five green blinks. That diagnostic mistakenly
validated every DelayTimerUnits call against59. Stock1.04 legitimately uses
12 and9 during InitializeControllerAsic (RAM:fdb87c and fdb895), and2470
in configuration/input validation (RAM:fdc75c and fdc95c). Live canonical
Ghidra xrefs and instructions confirm all four calls. The first cold-init12
call likely explains the result; the observed count does not identify the
argument or caller. It is not evidence of firmware corruption.

## Scope correction

With the existing `delay` diagnostic selector, timing_target.c compiles a
shared C worker with a per-call probe argument. Ordinary DelayTimerUnits
calls select0, including ordinary DelayTimerUnits59 calls. Only handshake
phase1 in RunControllerStartupStatusHandshake selects1 through the diagnostic
StartupHandshakeDelay59 wrapper. The timer arithmetic, setup and cleanup
remain shared. No persistent diagnostic state is allocated. The compiler
holds the selector in R15 and saves/restores it on the current stack.

The generated listing confirms both overflow checks branch on this selector.
Ordinary calls retain unbounded SFR:91.7 polling; the handshake uses the
diag26 register/stack helper with32*65535 polls. This limit is not a calibrated
physical time. Successful waits preserve context and allow original cleanup.
The completion hook then halts with4. There is no argument validator or count5.

| Green count | Observation |
|---|---|
| 1 | Handshake poll budget exhausted; SFR:91 bits7/6 both clear |
| 2 | Budget exhausted; bit7 clear, bit6 set, global interrupts disabled |
| 3 | Budget exhausted; bit7 clear, bit6 set, global interrupts enabled |
| 4 | The actual handshake delay returned and cleared bits6/7 |
| 6 | Overflow appeared between the final poll and terminal snapshot |

A lack of a report means this diagnostic has not reached a terminal
observation; it cannot locate an earlier block. Count4 is only delay
completion, not the rest of host initialization. Reports repeat forever at
unchanged slow32 timing. Windows detection is not expected. Record one group.
User alone flashes with redumper. Recovery: hold Eject at power-on, then
restore stock. No agent flash, reset or activation command was issued.

## Reproduction

```powershell
cmake -S . -B out/agent-tests/diag27-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON -DPLEXTOR_HOST_GATE_SNAPSHOT=delay
cmake --build out/agent-tests/diag27-build --config Release --target firmware --parallel 4
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag27_target.py matrix
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag27_target.py calls
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag27_target.py frame
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag27_target.py handshake
python out/agent-tests/run_diag27_timeout.py
```

Build passes with zero Keil warnings/errors. Image size524288, checksum8eb4,
monitor checksum8eb4, entry-window sum65899, fixed data matches stock.
SHA256: `7bf565ba1f98082adc146621fce3149975d5382e2d8794df2cce5617de2f6f92`.
Ordinary firmware rebuilt with diagnostics disabled remains byte-identical
to diag18: `d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`.
Twelve direct assembly-helper cases pass (immediate/delayed overflow, bounded
snapshots1/2/3, late flag6, completion4). Eight ordinary C delay cases pass
for9/12/59/2470 with global interrupts enabled/disabled. A breakpoint at the
bounded helper proves these calls bypass it; modeled repeated overflow lets
the real C loops complete. Four actual handshake-wrapper cases pass: no
overflow reports2/3, modeled overflow returns with reload2b/f2 and cleared
timer bits. C returns preserve DR12, IE and fullDR60=0520; assembly helper
returns additionally preserve DR4/DR8 and flags. A complete slow four-green
frame passes with frozen repeated count4 and interrupts masked throughout.

Paired synthetic mode1 continuation passes: stock's first final poll at
RAM:fdbdf4 versus the rebuilt terminal report. Both have packet0, state00/30,
SFR:91/94/96=40/2b/f2, and matching captured remaining RAM deltas after
explicit coroutine stack/savedSPX exclusions. The post-init seed skips cold
hardware initialization and has global interrupts disabled. These simulated
overflow events do not establish real hardware timer behavior.

Initial matrix launch timed out60s without execution files. An owned-window
inspection launcher ran the same project successfully in about5s; validation
of that result passes all12 cases. No launch-timeout cause is established.
All uVision processes ran on a private desktop. The maintained runner was
not changed.

Bundle:
`out/agent-tests/diag27-scoped-handshake-delay-20260926/PXW5224A_diag27_scoped_handshake_delay.bin`.
Physical result: FOUR green blinks. The actual host-handshake delay returned
and its original timer cleanup ran. This establishes progress through that
delay under diag27 timing, not the later controller handshake, timer frequency,
or host enumeration. Diag28 follows the remaining phases. No production fix
or Ghidra metadata edits. Archived diag27 evidence remains immutable.
