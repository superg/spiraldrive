# Diag28: follow the remaining host-handshake phases

Physical result: both LEDs stayed continuously on for at least one minute,
with no blink group. No terminal report was observed; this does not identify
a particular failed phase. The pass budget cannot expire if the command
coroutine stops being resumed. Further blink/flash diagnostics were stopped
after the user reported excessive flashing effort. See
`SCSI_DEBUG_MONITOR_FEASIBILITY.md` for the replacement investigation.

Physical diag27 reports FOUR green blinks, proving the actual handshake
DelayTimerUnits59 call returned and ran its cleanup. It does not establish
completion of the later controller handshake or enumeration.

Live canonical stock1.04 RAM:fda757 confirms the remaining behavior:

- Phase2 checks RAM:018043 bit0; clear requests RestartFirmware. It then
  reads status again. Bit5 clear advances to phase3 and clears RAM:000c87
  bit1. Otherwise a signed RAM:00121e <=0 ends with c87 bit1 set.
- Phase3 writes6 to RAM:001204 and advances to phase4.
- Phase4 checks status bit0, again requesting restart if clear. A second
  status read with bit4 clear ends with c87 bit2 set. Otherwise a signed
  RAM:001204 <=0 ends with c87 bit2 clear.
- Completion clears RAM:018041 and returns. TimerDispatchPhase2 decrements
  RAM:00121e at RAM:fd16a2..fd16a7 and the slower RAM:001204 countdown at
  RAM:fd192c..fd1931. A successful SFR:91 delay alone does not prove these
  software countdowns advance.

## Diagnostic behavior

The new `phases` selector disables the earlier delay/entry stops. It uses the
original unbounded timer delay, then follows the original handshake branches.
Restart requests are intercepted immediately before the RAM:0000fe write
and reset call. Completion reports after the original RAM:018041 clear.

The existing16-bit local state holds the phase in its low3 bits and a bounded
pending-pass count in its upper13 bits. Phase assignments reset the count.
Only pending phase2/4 passes increment it;8191 is terminal. This includes
the pass entering each phase, so a bound allows8190 status evaluations.
It is a count of coroutine passes, not a calibrated duration or proof of a
permanently stuck timer. No new diagnostic RAM is allocated. Target listing
holds the state in WR14, inside DR12 saved by the actual coroutine boundary.
Scheduler locals are not used. Interrupts are masked only on terminal report.

| Green count | Observation |
|---|---|
| 1 | Phase2 took the controller-status bit0-clear restart branch |
| 2 | Phase4 took the controller-status bit0-clear restart branch |
| 3 | Handshake completed through phase2 countdown expiry |
| 4 | Handshake completed through phase4 status bit4 clear |
| 5 | Handshake completed through phase4 countdown expiry |
| 6 | Phase2 exhausted the pending-pass budget |
| 7 | Phase4 exhausted the pending-pass budget |
| 8 | Unexpected phase/default completion path |

Counts3..5 confirm the handshake's port cleanup, not the later c32=1 write
or host enumeration. Counts6/7 establish the bounded observation only. A
lack of report could mean a block in another coroutine or an earlier wait.
All frames repeat forever at unchanged slow32 timing; one group suffices.
Windows detection is not expected. User alone flashes with redumper;
recovery remains Eject held at power-on, then stock restore. No agent flash,
reset or activation command was issued.

## Build and validation

```powershell
cmake -S . -B out/agent-tests/diag28-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON -DPLEXTOR_HOST_GATE_SNAPSHOT=phases
cmake --build out/agent-tests/diag28-build --config Release --target firmware --parallel 4
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag28_target.py matrix
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag28_target.py stock
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag28_target.py frame
```

Image size524288, checksum/monitor checksumbf12, entry-window sum64975,
fixed data matches stock, zero Keil build warnings/errors. SHA256:
`7d4aa9e8012da7b172f3386f44200ad876b82c8d7dd98ff73e7cdaefc3fba0a8`.
Normal diagnostics-OFF firmware remains exact diag18:
`d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`.

Native S251 scenarios use an auxiliary scheduler outside firmware at
RAM:f70000, real ResumeCoroutine0/YieldCoroutine0 and the actual C handshake.
They supply explicit status/countdown transitions and synthetic SFR:91.7
overflow; no firmware instructions are patched. These isolate branch and
coroutine behavior, not actual hardware timing or full cold boot.
All9 reconstructed native scenarios pass in `matrix-native-final`: both
restart paths, phase2 signed zero/negative completion, immediate/delayed
phase4 status transition, phase4 countdown completion, and both full budgets.
The bound paths halt after8191/8193 actual scheduler returns respectively.
All outcomes preserve the expected c87 flags and port8041 value; restart
interception leaves the sentinel RAM:0000fe unchanged. Full native stack
values are captured through DR60 and the saved scheduler/C0 pointers.

The complete slow frame passes in `frame-native-final`: yellow framing then
four green pulses, frozen count4 on the next frame and interrupts masked
throughout. Normal firmware remains byte-identical as stated above.

Initial scratch harness attempts used ambiguous debugger address spaces for
near RAM/controller inputs. The corrected fixture uses explicit EDATA `E:`
and MOVX `X:` spaces; it also checks a real final-poll breakpoint before
supplying SFR:91.7, rather than a free-running overflow signal. Auxiliary
scheduler ECALL has a3-byte return frame in this configuration: scheduler
DR60=0280 is saved as0283. These are harness corrections, not firmware fixes.
Earlier invalid attempts/timeouts remain separate. Stock comparison PASSES
all7 applicable scenarios in `stock-data-view`: matching yield counts, c87,
port8041 and both countdown bytes. The two added diagnostic pass limits have
no stock terminal equivalent. `diag28-comparison.json` records these matches.
Stock stops before its real restart or immediately after its port cleanup;
the native stock instructions execute, without replacements for yield/delay.

Final stock setup loads its separate `FIRMWARE_MEMORY.ABS` data view before
RESET, as the maintained differential runner does. Earlier stock attempts
omitted that load and timed out; extending the configured ROM range alone
did not resolve them. Some attempts left no execution files. No general
uVision timeout cause is established, and failed harness runs are not firmware
failure evidence. The maintained simulator runner was not changed.

Bundle:
`out/agent-tests/diag28-handshake-phases-20260926/PXW5224A_diag28_handshake_phases.bin`.
Physical result pending. No production fix or Ghidra metadata edits.
