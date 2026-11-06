# Diag26: bound the host-handshake timer wait

Physical diag25 reports one green blink: initial startup C1 entry, packet0,
C0 suspended at the mode1 host-handshake yield. This explains the earlier
zero-state observations without implying corruption. It does not prove the
handshake later resumes or completes.

Live canonical stock1.04 RAM:fda757 shows the next handshake pass invokes
DelayTimerUnits59 at RAM:fda78a. That calls RAM:fdbda2 with WR6=003b.
The delay operates on SFR:91 bits6/7 and SFR:94/96, not the conventional
SFR:88 timer controls used by a different delay worker. The physical role of
these extended SFRs is not independently established here. Do not infer that
the simulator's TMOD10 setup covers this timer.

For59 units, the firmware clears SFR:91 bits6/7, writes zeros to SFR:94/96,
sets bit6, writes reload2b/f2, then waits for bit7. Live stock instructions and
the rebuilt timing_worker listing agree on those accesses. No production
timer-address or control-bit change is justified by current evidence.

## Diagnostic behavior

Selector delay with PLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON defines
PLEXTOR_LED_HOST_DELAY_DIAGNOSTICS. Earlier diagnostic stops/tags compile out.
Original timer setup/calculation/cleanup remain. Only overflow polling is
replaced by an addressless helper that allows32*65535=2097120 bit7 checks,
using registers and the current stack. It does not mask interrupts while
waiting, supply an overflow flag, or allocate persistent scratch RAM.

A validator preserves context and returns for argument59; another argument
reports5, avoiding attribution of an unexpected delay to the handshake.
Immediately after the handshake's real delay returns, a terminal hook reports4.
On poll exhaustion the helper snapshots current IE, masks interrupts, reads
SFR:91 and reports the following. Passing waits preserve registers, flags,
stack and interrupt enables and return to the original cleanup.

| Green count | Observation |
|---|---|
| 1 | Poll limit reached; SFR:91 bit7=0 and bit6=0 |
| 2 | Poll limit reached; bit7=0, bit6=1, global interrupts disabled at snapshot |
| 3 | Poll limit reached; bit7=0, bit6=1, global interrupts enabled at snapshot |
| 4 | The host-handshake delay59 returned |
| 5 | DelayTimerUnits received an argument other than59 |
| 6 | Bit7 became set between the last poll and the terminal snapshot |

Counts1..3 establish a bounded observation, not a permanently broken timer.
The poll budget is not calibrated physical seconds. Count4 proves progress
through this delay, not host initialization completion. A lack of report may
mean execution did not reach the instrumented delay; do not assign a count.
All terminal frames repeat forever with unchanged slow delay32. Windows
detection is not expected; one repeating count is enough.

User alone flashes with redumper; recovery remains Eject held at power-on,
then stock restore. No agent flash/reset/activation was sent.

## Build and verification

```powershell
cmake -S . -B out/agent-tests/diag26-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON -DPLEXTOR_HOST_GATE_SNAPSHOT=delay
cmake --build out/agent-tests/diag26-build --config Release --target firmware --parallel 4
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag26_target.py matrix
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag26_target.py frame
python out/agent-tests/run_diag26_timeout.py
```

Image size524288, checksum76b1, SHA256
`b6e3e9602828c331759f892ec788bb0c6e6ddc478f28ad9013f216de20508fb6`.
Format, both checksum expressions and stock fixed data pass. Normal build
with diagnostics OFF remains exact diag18, SHA256
`d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`.

The first broad synthetic continuation reached the rebuilt final SFR:91.7
poll and exhausted five million simulator states. That alone is not evidence
of a physical failure or a stock/rebuild mismatch. A narrower paired delay
entry attempt and a low-state-budget timer snapshot attempt timed out in
uVision; no paired pass is claimed for them. Evidence remains in the
diag26-mode1-continuation/, diag26-delay-entry/ and diag26-timer-snapshot/
scratch directories. Final focused probe outcomes are recorded below.

- All19 direct S251 cases pass: argument59 context preservation, other
  arguments reporting5, immediate/delayed overflow returns, timeout counts1/2/3,
  late flag count6, and completion count4. Silent returns preserve DR4/8/12,
  PSW/PSW1, IE and full DR60=0520. No firmware instruction patches. Result:
  diag26-target-probes/matrix-private-root/validation.json.
- A complete slow four-green frame passes, with port sequence2,2,3 followed
  by four1,3 pairs, IE masked throughout and frozen count4 on the next frame.
  Final run uses actual DR60=0520. Initial60s attempt timed out;120s retry
  completed in about3s. No causal timing claim. Result:
  diag26-target-probes/frame-retry120/validation.json.
- Paired mode1 continuation PASSES comparing stock's first final overflow
  poll (RAM:fdbdf4) with diag26's bounded terminal report. Both have packet0,
  state00/30 and SFR:91/94/96=40/2b/f2, with matching remaining captured RAM
  deltas after explicit coroutine-stack/saved-SPX exclusions. Diag26 reports2
  because global interrupts are disabled in this post-init simulation seed.
  This validates the path and snapshot; it is not hardware timer evidence.
  Result: diag26-delay-timeout/results.json.
- The passing direct matrix and paired run use an alternate scratch launcher
  on the private desktop, invoking uVision from the workspace root. Prior
  launches timed out intermittently. Several launch details changed together;
  no root cause of those tool timeouts is established and the maintained
  simulator launcher is unchanged.
- With a synthetic overflow injected at five million simulator states,
  reconstruction reaches the handshake's completion reporter/count4 and
  clears SFR:91 bits6/7. The stock half timed out in uVision, so this additional
  paired comparison is INCOMPLETE. An earlier one-million-state injection
  did not produce completion; the retained final state was still the wait.
  Do not treat either simulation as a measurement of the real timer.
  Evidence: diag26-delay-overflow/ and diag26-delay-overflow5m/.

Harness correction: S251 debugger SP is eight bits. A diag25 frame test
assigned SP=0520, actually selecting low SP20; its frame passed but the claimed
high stack was incorrect. Diag26 direct probes use DR60=0520 and capture DR60
to check full SPX. Earlier diag26 SP-only attempts are retained separately.

Artifact:
`out/agent-tests/diag26-handshake-delay-20260926/PXW5224A_diag26_handshake_delay.bin`.
Physical result: FIVE green blinks. This only establishes a non59 argument;
it does not identify the caller or prove a fault. The validator was scoped
too broadly: live stock1.04 RAM:fdb878/fdb87c and fdb891/fdb895 call the same
delay with12 and9 during InitializeControllerAsic. RAM:fdc758/fdc75c and
fdc958/fdc95c also legitimately use2470. The first cold-init12 call is a
likely explanation, not a measured argument. The generated diag26 listing
reloads the original units into WR6 before validation.

Diag27 removes this global argument trap and selects the bounded check at
the handshake call site. See DIAG27_SCOPED_HANDSHAKE_DELAY.md. The archived
diag26 image, source, and tests remain unchanged. No production fix or Ghidra
metadata edits.
