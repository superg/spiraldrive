# Diag24: narrow the pre-media zero-state interval

Physical diag23 reported five green blinks: RAM:000c32 was zero immediately
before InitializeStartupMediaState. It does not prove the earlier drive-init
check executed. ConfigureInitialCoroutines also installs the startup sequence
as the initial coroutine1 entry; the normal command-processor path can replace
it with drive initialization. Unexpected ordering remains a hypothesis.

Live canonical stock1.04 disassembly at RAM:f84b0b and RAM:f84c30 confirms the
local sequence: drive initialization returns, C1 yields and restarts, then
the startup prefix writes flags, two dwords and clears BITS:0017. None of
those prefix instructions directly writes RAM:000c32. Interrupts or other
coroutines may act between checks; a count brackets an observation rather
than proving that the immediately preceding instruction caused it.

One long yellow marker followed by slow green pulses repeats forever:

| Count | Observation |
|---|---|
| 1 | Zero immediately before the drive-init yield |
| 2 | Zero immediately after that yield returns |
| 3 | Zero on entry to the startup sequence, before its writes |
| 4 | Zero after the small flag/status write group |
| 5 | Zero after writing the dword at RAM:001598 |
| 6 | Zero after writing the dword at RAM:001484 |
| 7 | Zero after clearing BITS:0017, just before media initialization |
| 8 | Nonzero at the final check, with no earlier check reporting |

Points1..6 return silently when RAM:000c32 is nonzero. Point7 always stops
and reports7 or8. Point3 may be reached directly, so it does not prove the
yield or restart ran. Count8 does not imply all optional paths executed.

This diagnostic deliberately halts once reporting; Windows detection is not
expected. Only one repeating count is needed. The slow post-diag21 pulse and
group delays are retained. The user can power off after counting. User alone
operates redumper; recovery remains Eject held at power-on, then stock restore.
No agent flash/reset/activation command was sent.

## Implementation and reproduction

PLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON with PLEXTOR_HOST_GATE_SNAPSHOT=interval
selects the addressless startup_host_interval_diagnostics.a51 checker and
defines PLEXTOR_LED_HOST_INTERVAL_DIAGNOSTICS. Earlier diagnostic hooks are
disabled in this mode. There are11 call sites: before/after yield on each of
three drive-init branches, plus five startup-prefix checks. No persistent
diagnostic RAM or forced packet-state writes are introduced. Silent calls
restore IE, PSW, PSW1 and DR8; other registers are untouched. Terminal reports
mask interrupts and freeze the count in R14. Ghidra metadata was not changed.

```powershell
cmake -S . -B out/agent-tests/diag24-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON -DPLEXTOR_HOST_GATE_SNAPSHOT=interval
cmake --build out/agent-tests/diag24-build --config Release --target firmware --parallel 4
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag24_target.py matrix
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag24_target.py frame
python out/agent-tests/run_diag24_continuation_retry.py
```

Artifact:
`out/agent-tests/diag24-host-interval-20260926/PXW5224A_diag24_host_interval.bin`.
Size524288, checksum1fb5, SHA256
`e882a90ce5b1f647bc2a110b88c28d2e4a3ed6b958c5ad464b9adb5e9012e99f`.
Format, both checksum expressions and stock fixed data pass. Build has zero
Keil diagnostics. Normal firmware with LED diagnostics OFF remains exactly
diag18, SHA256
`d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`.

## Validation

- All29 direct S251 cases pass: points1..6 with zero/nonzero and IE0/80,
  plus point7 with packet values0/1/10/11/ff. Silent calls preserve DR4/8/12,
  PSW/PSW1, IE and SP; RAM:000c32 is unchanged. Target instructions are not
  patched. Results are in diag24-target-probes/matrix/validation.json.
- A complete slow four-green frame passes: the expected yellow/green port
  transition sequence is captured, interrupts remain masked, and count4
  repeats even after changing RAM:000c32. Results are in
  diag24-target-probes/frame/validation.json. This validates count retention
  and reporter behavior, not physical pulse duration.
- The paired startup comparison is INCOMPLETE. It uses the existing synthetic
  post-init state0/controller/Timer1 model, stopping stock
  at InitializeStartupMediaState entry and reconstruction at its terminal
  reporter immediately before that call. No code patches or function stubs;
  explicit controller and countdown responses remain. This is not a physical
  cold-boot model. The first60-second attempt timed out before producing
  execution snapshots; retained in diag24-startup-continuation/. On the120s
  retry, reconstruction reached the intended reporter with RAM:000c32=1,
  count8, state08/10 and savedC1SPX00000507. The stock run then timed out before
  producing snapshots. No paired RAM-delta match is claimed. Evidence is in
  diag24-startup-continuation-retry/. Do not rerun unchanged; investigate the
  simulator startup failure separately if further comparison is needed.

Physical result: three green blinks. RAM:000c32 is zero at startup-sequence
entry, before the prefix writes. Subsequent analysis found a legitimate stock
path to this observation: startup mode1 yields in the host handshake before
the idle1 write. Both stock and the ordinary rebuilt image reproduce it in
the simulator. Zero alone is therefore not evidence of a firmware defect.
See DIAG25_STARTUP_ENTRY_ROUTE.md for the paired case and physical discriminator.
