# Diag18: controller reset addressing

2026-09-26; stock PX-W5224A 1.04 and the reconstructed firmware.

## Confirmed defects

The real early startup continuation exposed a controller-state difference:
stock cleared RAM:018046, while reconstruction left its startup value 2.
Separate S251 write watches identified stock RAM:f8b5d8 as the writer. This
is inside ConfigureControllerPorts8046And804EAnd80C9Through80CB (f8b58c).
Live canonical Ghidra instructions establish:

- MOVX accesses controller RAM:018002 and, for operating code 10, RAM:01806a.
- MOV @DPX,R12/R13 at f8b5d8/f8b5de writes RAM:018046/01804e in this path.
- Table calculations construct bank ff plus bases 84fe and 850a, then add
  twice the profile index. Each word is emitted low byte then high byte.

The C implementation used near RAM:008002/008046/00804e/00806a, table bank
00, and an erroneous minus-one table offset. All three defects are corrected
in `src/platform/controller_parameters.c`. The fixed data itself is unchanged.
The canonical plate comment was corrected and the intended main program saved.

This function is reached during the early drive-control reset. The evidence
does not yet establish that these defects caused the physical failure.

## Validation

Three discriminating S251 cases in
`tests/simulator/cases/controller-reset-addressing.json` cover disabled mode,
enabled ordinary mode (profile-one stub), and enabled operating code 10.
Distinct table bytes establish the bank, index and byte order: profile zero
emits b2/a1/f6/e5, profile one emits d4/c3/89/67. They also check controller
port output and preservation of a low-RAM sentinel. All three mismatch before
the change and pass afterward. Evidence directories:

- `out/agent-tests/controller-reset-addressing-before/`
- `out/agent-tests/controller-reset-addressing-final-values/`

The host test now checks the correct controller and ROM addresses across all
512 enabled/operating-code combinations, with low-RAM sentinels. Assertions
are enabled in Release. `controller_reset_addressing_tests` passes.
The previous host expectations repeated the faulty address calculation and
were corrected; the stock-instruction/simulator comparison is independent.

`tests/simulator/cases/early-drive-control-continuation.json` executes real
scheduler resumes, controller reset, operating-state reset, signal probe,
state-10 yield, result propagation and coroutine restart to startup. Its
state-8 seed selects the reset's fast branch; Timer1 delay f8f127 is an explicit
return stub, and bounded controller/countdown responses are supplied. It now
passes (`out/agent-tests/early-drive-reset-state8-fixed/`). Before the fix,
the same flow differed at RAM:018046. Earlier expected state 08/00 was also
corrected to the observed stock 08/10 after parameter writes were restored.

All three previously accepted downstream startup cases pass on the new image
(`out/agent-tests/p1-startup-continuation-controller-fix/`). These seven paired
cases do not establish full cold boot, pickup homing, real timer interrupts,
physical sensor input, or Windows detection.

Reproduction uses the existing Windows/private-desktop catalog runner through
`out/agent-tests/run_controller_reset_addressing.py`,
`run_early_drive_control.py`, and `run_p1_startup_continuation.py`; each takes a
new output-directory name. Host/build checks:

```powershell
cmake --build out/windows --config Release --target firmware reconstruction_foundation_tests --parallel 1
ctest --test-dir out/windows -C Release -R '^controller_reset_addressing_tests$' --output-on-failure
```

An initial host link was blocked by an agent-started stale test executable;
that identified process was terminated, the target relinked successfully, and
the focused test passed. Firmware generation had already completed normally.

## Candidate and hardware boundary

Artifact:
`out/agent-tests/diag18-controller-reset-addressing-20260926/PXW5224A_diag18_controller_reset_addressing.bin`.
SHA-256 `d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`;
524288 bytes; stored and monitor checksum afa4. LED boot diagnostics OFF.
Format and immutable stock fixed-data verification pass. The archive contains
linked files, source snapshots, before/after results and a manifest.

This is a rebuilt image, not a fixed-offset patch like diag16. It retains the
P1 input, timer-state, host RETI and IDENTIFY corrections. Physical result:
the user reports no Windows enumeration and the tray stays closed. The PnP
snapshot `out/agent-tests/diag18-pnp-20260926-135121.json` finds neither the
target nor its 093b:0042 bridge. Installed bytes have not been read back.
The user flashes with redumper; recovery remains Eject held during
power-on followed by stock restore. No agent flash/reset/activation was sent.

The subsequent state-zero/Timer1 continuation passes; the exploratory real
Timer0 run reaches the media-state entry but mismatches. Diag19 adds a physical
startup-progress reporter; see `DIAG19_STARTUP_PROGRESS.md` for exact scope.
Do not convert model exhaustion into a firmware fix.
