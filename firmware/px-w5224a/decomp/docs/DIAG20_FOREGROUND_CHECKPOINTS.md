# Diag20: direct startup checkpoints

## Observation and scope

After flashing diag19 the user reports no reaction and both LEDs continuously
on. No repeating diagnostic frame was observed. Installed bytes were not read
back. This does not establish whether Timer0 stopped, execution stalled before
the deadline, diagnostic storage was invalid, or another path was taken.
Diag19's simulated counter was accelerated; its timer-dependent reporter did
not resolve the physical startup location.

Diag20 retains diag18's reconstruction and replaces the diagnostic strategy.
It emits a frame directly at each instrumented foreground boundary. The first
call is the first operation in `FirmwareMainLoop`, before cold initialization.
The adapter uses only registers, the current stack, and the established P3 LED
bits. It requires neither timer operation nor compiler-allocated XDATA for its
count or delay. No new production firmware fix is claimed.

## Frames

Each frame consists of a long yellow marker, then N short green pulses:

| N | Boundary reached |
|---|---|
| 1 | Main-loop entry, before cold initialization |
| 2 | Initialization returned or was skipped |
| 3 | Initial coroutine configuration returned |
| 4 | Drive-initialization coroutine entered |
| 5 | Drive-control initialization returned |
| 6 | Startup-media initialization about to run |
| 7 | Startup-media initialization returned |
| 8 | Media state machine entered |

Every call displays its literal number and then returns. There is no monotonic
memory latch and no timer-triggered final report; numbers may recur as firmware
restarts or revisits a boundary. Record the sequence and the last complete
frame before it goes steady. Count green pulses, not the yellow marker.
Allow about a minute after normal power-on with the tray empty. If it remains
completely steady, say whether there was even one brief transition at power-on.

Each frame masks interrupts while it runs, then restores the previous interrupt
enable byte. These pauses change startup/host timing. This is a diagnostic,
not an enumeration or physical timing equivalence test. The adapter does not
disable a motor or force a controller output beyond the two LED lines.

Recovery remains holding Eject during power-on, then restoring stock with
redumper. The user performs flashing. No agent flash/reset/activation command
was sent during this work.

## Implementation and validation

`PLEXTOR_LED_FOREGROUND_DIAGNOSTICS` is OFF by default and mutually exclusive
with the two existing LED modes. The existing startup hooks select the new
addressless assembly adapter; normal builds compile them out. The adapter is
a documented diagnostic exception, not a canonical function restored as
assembly. It preserves DR4, DR8, DR12, PSW, PSW1 and IE; other general registers
are untouched. All saves and delay counters use the current stack/registers.
The 108-byte adapter fits the ordinary linker classes, with no diag19-style
code/constant boundary adjustment.

S251 checks execute the complete unmodified adapter:

- One-pulse frame returns to its caller with registers, flags, SP and IE=0
  preserved. Trace observes P3 low-bit states 2,2,3,1,3.
- Eight-pulse frame emits exactly eight green pulses and returns with the same
  saved state and IE=80 restored. IE is zero throughout the pulse writes.
- Reset veneer RAM:f80050 reaches the one-pulse frame, returns from it, and
  reaches the actual `InitializeDriveFirmware` entry (linked RAM:fe6ff7).
  This is a synthetic reset-entry check, not a complete monitor/cold-start run.

The checks use explicit simulator mappings and generic timer/interrupt seeds.
Initial standalone harness attempts hit 45-second wall limits; one short
probe intentionally stopped inside the delay. These incomplete attempts are
not physical firmware failure evidence. Completed traces and results are
preserved under `out/agent-tests/diag20-target-probes/` and
`out/agent-tests/diag20-reset-probe/`. Runners are
`out/agent-tests/check_diag20_target.py` (optional stage argument 1 or 8) and
`out/agent-tests/check_diag20_reset.py`; run each on the private desktop.

The ordinary firmware rebuild remains byte-identical to diag18:
`d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`.
The new diagnostic builds with zero Keil warnings/errors and passes both
checksum expressions, image-format validation and immutable fixed-data checks.
The earlier actual-Timer0 stock/rebuild controller differences at RAM:018087
and RAM:01808a remain unresolved; this diagnostic does not explain them.

```powershell
cmake -S . -B out/agent-tests/diag20-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_FOREGROUND_DIAGNOSTICS=ON
cmake --build out/agent-tests/diag20-build --config Release --target firmware --parallel 4
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag20_target.py 8
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag20_reset.py
```

Artifact:
`out/agent-tests/diag20-foreground-checkpoints-20260926/PXW5224A_diag20_foreground_checkpoints.bin`.
SHA-256 `5eca42d41aaf065c7823ebabedd8b5465d0d9b3d7f5bbc5c0c6cb9b84004309a`;
524,288 bytes; checksum e347.

## Physical result (2026-09-26)

The user reports the final complete frame had **eight green blinks**. Windows
does not detect the drive and the tray stays closed. A successful read-only
Get-PnpDevice query also finds neither PX-W5224A nor bridge VID_093B/PID_0042;
snapshot: `out/agent-tests/diag20-pnp.json`. An earlier denied query was not
valid absence evidence; the saved snapshot is the successful elevated query.

Eight proves entry to ProcessDriveStateMachine (stock RAM:f84f6a), the last
instrumented boundary. It does not locate a halt there, prove cold init was
taken rather than skipped, or establish host-command completion. Diag20's
interrupt-masked frames substantially perturb host timing.

Live canonical inspection confirms the following wait at RAM:f84fb5..f84fbd
yields coroutine1 while RAM:00faeb is zero. The host ISR sets that byte at
entry; FirmwareMainLoop can also preset it. Two extended paired S251 cases
reach RunDiscRecognitionAndTocInitialization at RAM:fb9d58: one with a ready
latch and one with synthetic read sequence 0,0,1, exercising two real yields.
Both pass all captured comparisons and expected state/host-packet observations.
They retain explicit countdown/controller responses from the maintained
startup case, and do not simulate host interrupt arrival or cold boot.
Reproduction: `python out/agent-tests/run_post_stage8.py`; results/cases under
`out/agent-tests/post-stage8/` and `out/agent-tests/post-stage8.json`.
