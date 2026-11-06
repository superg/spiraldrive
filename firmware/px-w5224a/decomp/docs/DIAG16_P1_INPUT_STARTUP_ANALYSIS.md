# P1.7 isolation and timer input-state correction

Target: Plextor PX-W5224A firmware 1.04. Date: 2026-09-26.

## Physical evidence

The user restored diag14: tray opens, closes, then opens and stays open.
Windows PnP again lists the bridge093b:0042 and optical revision1.04. A new
readback stalled, so this does not establish reliable command handling.
Diag16 changes only the P1.7 input-address operand and checksum from diag14.
The user reports it flashed and Windows no longer sees the drive. Agent PnP
at 13:06:12 confirms no target bridge/optical child and no USB error device.
Evidence: `out/agent-tests/diag16-pnp-20260926-130612-4379232.json`. Diag16 tray behavior
is awaiting the user's answer. Installed diag16 identity is user-reported;
unlike diag15, it has not been read back in recovery.

This isolates a startup failure associated with correcting P1.7 or the paths
it enables. It does not identify the failing caller or invalidate the stock
input address. Stock RAM:fb01d0 reads RAM:0011ed; the previous RAM:001300
read was erroneous. Three bounded asserted-P1.7 caller comparisons passed
earlier, with declared dependencies; they do not model complete hardware
startup. Keep all proven input corrections in maintained C.

## Confirmed additional defect

Live canonical `/validated/MONT_104.bin`, TimerDispatchPhase0 RAM:fd1b3d:

- RAM:fd1b85 loads RAM:001300, then compares the drive state with 0x0a.
- This controls calling RAM:fd1d1e on a rising debounced P1.6 transition
  when RAM:00faed bit0 is set.
- The previous C tested RAM:000c43 instead. That address remains the correct
  destination for request-code writes at RAM:fd1bed and RAM:fd1c4c.

Only the erroneous condition in `src/platform/timer_phase0.c` is changed.
The host direct-SFR macro already token-pastes `90` into `0x90`; no SFR
binding correction is needed. Physical causation of the P1.7-associated
failure remains unproven.

## Validation and candidate

Release assertions are now explicitly enabled in timer_phase0_tests. A new
test covers all 65,536 independent drive-state/request-code pairs with the
edge condition selected. It fails before the source correction and passes
after it, along with the existing timer phase-zero tests. MSVC assertion
failures report on stderr without opening an interactive dialog.

`tests/simulator/cases/timer-phase0-drive-state.json` supplies drive10/request0
and drive0/request10 cases, with a declared transition stub marking RAM:001700.
Both mismatch before and pass afterward; expected marker, timer and flag
values are checked on both sides. Reproducer:
`python out/agent-tests/run_timer_phase0_audit.py <new-output-name>`.
Evidence: `out/agent-tests/timer-phase0-before-fix/` and
`out/agent-tests/timer-phase0-after-fix/`.

Build and host check:

```powershell
cmake --build out/windows --config Release --target firmware timer_phase0_tests --parallel 1
ctest --test-dir out/windows -C Release -R '^timer_phase0_tests$' --output-on-failure
```

Normal-running diag17 retains all three corrected P1 helpers and the earlier
interrupt-return/IDENTIFY corrections. LED diagnostics are OFF. Artifact:
`out/agent-tests/diag17-timer-input-state-fix-20260926/PXW5224A_diag17_timer_input_state_fix.bin`.
SHA-256 `9f1d465d658f4cefa400de6ed27249cdb1b9a81bd3d12f811382fb3a89c90ff9`,
checksum df2c, 524,288 bytes. Relative to diag15, only RAM:fe3630..fe3631
changes 0c 43->13 00, and checksum RAM:ff9fff changes 68->2c. No layout
changes. Strict format, both checksum expressions and fixed data checks pass;
the archive contains map/ABS/HEX/listing, tests, source and simulator evidence.
Canonical timer plate comment was amended and only the main program saved.

User test: established Eject-at-power-on recovery, redumper flash, normal
power-on with a closed empty tray; report tray behavior and Windows detection.
Recovery remains Eject-at-power-on and stock restore. No agent flash/reset
or activation command was sent. User subsequently reports diag17 flashed with "nothing good"; present-device
query again found no bridge/optical child. Installed bytes and exact tray
behavior are not established for that run. See DIAG17_STARTUP_CONTINUATION_ANALYSIS.md.
