# Diag25: distinguish the initial startup entry and host handshake yield

Physical diag24 produced three green blinks: RAM:000c32 was zero at
RunStartupSequenceCoroutine1 entry. Earlier counts did not prove host
initialization had completed or drive initialization had run.

Live stock1.04 Ghidra instructions establish a legitimate path to this state:

- RAM:fdbe7d invokes ConfigureControllerStartupModeRegisters at RAM:fdbeb5,
  before writing idle1 to RAM:000c32 at RAM:fdbebf.
- For startup mode1 (RAM:000c79=1), when RAM:0000fd masked by f9 is not99,
  RAM:fda020 calls RunControllerStartupStatusHandshake at RAM:fda757.
- The handshake first writes controller RAM:018046=0, then yields C0 at
  RAM:fda832. The saved return address is RAM:fda836. The normal initial C1
  entry has not yet been replaced with drive initialization at RAM:fdbfc4.
- The scheduler resumes C1 at RAM:fdcf80, entering initial RAM:f84c30 while
  RAM:000c32 can still be zero. No corruption or skipped C0 is required.

## Paired simulator evidence

Maintained case: `tests/simulator/cases/startup-mode1-initial-entry.json`.
`out/agent-tests/diag25-mode1-continuation-checked/` passes stock versus the
ordinary diag18-equivalent image. Real scheduler, C0 initialization and first
handshake yield reach initial C1 entry with packet0, state00/00 and saved
C1SPX00000507. All other captured RAM deltas match after explicit stack
exclusions. Stock/rebuilt state counts are893/950.

This synthetic post-init case sets RAM:0000fb=1, RAM:000c79=1,
RAM:00faef=80, RAM:0011ef=08, debounced P1 bits84 and controller RAM:018043=1.
A timed write expires the initial RAM:001214 countdown. It uses no function
stubs or instruction patches. It does not establish the drive's physical
startup mode or hardware timing. The existing mode0 continuation did not
exercise this early handshake yield.

The initial raw comparison differs only at saved C0 SPX: stock0310 versus
rebuilt0316, consistent with different native/rebuilt call-frame depths.
C0-C3 stack storage and the four-byte C0 SPX slot are explicitly excluded from
byte equality in the checked case. Initial C1 checkpoint, its saved SPX and
all remaining captured deltas remain checked. Raw evidence is retained in
diag25-mode1-continuation/. No production fix follows from this comparison.

## Physical diagnostic

Diag25 tags only ConfigureInitialCoroutines' default startup C1 entry with an
addressless terminal reporter. Later initializations/restarts retain the real
startup entry, whose first diagnostic call selects the other-entry reporter.
The host handshake's YieldCoroutine0 call goes through a register-neutral
adapter adding one four-byte return frame. At C1 entry, the reporter masks
interrupts and inspects the suspended C0 frame: saved SPX at RAM:00115e,
return address at SPX-7, ignoring its leading mode byte. A relocated symbol
comparison identifies the adapter's exact resume label. No persistent scratch
RAM or timing pauses are inserted before reporting. No packet state is forced.

One long yellow marker and the following slow green count repeat forever:

| Count | Observed route and packet state |
|---|---|
| 1 | Tagged initial C1 entry, packet0, C0 suspended at the handshake yield |
| 2 | Tagged initial C1 entry, packet0, a different C0 saved return address |
| 3 | Tagged initial C1 entry, packet nonzero |
| 4 | Other startup C1 entry, packet0, C0 suspended at the handshake yield |
| 5 | Other startup C1 entry, packet0, a different C0 saved return address |
| 6 | Other startup C1 entry, packet nonzero |
| 7 | Packet0, saved C0 SPX outside the diagnostic's supported0307..04ff range |

Other entry does not uniquely prove restart; another initializer can install
the same entry. Count7 is a bounds classification, not a diagnosis of damage.
The tag/wrapper are diagnostic perturbations; physical timing equivalence is
not claimed. The classifier reads only a bounded saved frame and leaves it
unchanged. Report entries deliberately never return; Windows detection is not
expected. Slow delay32 is retained. One repeating count is enough.

User alone flashes with redumper. Recovery remains Eject held at power-on,
then stock restore. No agent flash/reset/activation command was sent.

## Build and validation

```powershell
cmake -S . -B out/agent-tests/diag25-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON -DPLEXTOR_HOST_GATE_SNAPSHOT=route
cmake --build out/agent-tests/diag25-build --config Release --target firmware --parallel 4
python out/agent-tests/run_diag25_mode1_checked.py
python out/agent-tests/run_diag25_route.py
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag25_target.py matrix
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag25_target.py frame
```

Diag25 size524288, checksum5e58, SHA256
`2ef2fdeca8b97c7958ec862da3b4af4c77a68b5b1e6dbd7b60361440aabdc925`.
Format, both checksum expressions and stock fixed data pass. Final build has
zero Keil diagnostics. An initial assembly immediate-address fixup failed at
link; using WORD2/WORD0 relocations corrected it before packaging.

The diagnostic image also passes the paired mode1 case, reaching its terminal
reporter with packet0/count1 and matching other captured state. The wrapper's
saved return address is thus tested through a real coroutine yield/switch.
All50 direct S251 classifier cases pass: both entry routes, packet values
0/1/10/11/ff, IE0/80, matching/other C0 return addresses, ignored leading
return mode byte and invalid-SP guards. RAM:000c32 is unchanged. Initial
matrix and single-case harness attempts omitted explicit PSW/PSW1 setup and
timed out; the final harness initializes D0=80/D1=40 and passes. No physical
firmware conclusion follows from those test-launch failures.

A complete slow one-green frame passes:
port sequence2,2,3,1,3; IE stays masked; count1 repeats after changing c32 to1.
The initial SP40 full-frame attempt timed out without a final snapshot. The
alternate harness run passes; no root cause for the first timeout is
established. Correction established during diag26 testing: debugger SP is
eight bits, so the assignment SP=0520 used here actually selected SP=20,
not SPX0520. The blink/frame result stands; the earlier high-stack claim does
not. Future full-stack probes must set and capture DR60. All50 classifier
cases used SP40 and passed. Frame adapter source
is unchanged from diag24. Results: diag25-target-probes/{matrix,frame}/validation.json.

Normal out/windows rebuild with diagnostics OFF remains byte-identical to
diag18, SHA256
`d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`.

Artifact:
`out/agent-tests/diag25-startup-route-20260926/PXW5224A_diag25_startup_route.bin`.
Physical result: the user reports one green blink. Tagged initial C1 entry
observed packet0 and C0 suspended at the host startup handshake yield. This
matches the stock mode1 route, not proof of corruption. Diag26 investigates
the handshake's subsequent timer wait. No production fix established.
