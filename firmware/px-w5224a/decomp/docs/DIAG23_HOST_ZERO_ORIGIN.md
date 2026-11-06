# Diag23: locate the zero host-packet state

The physical diag22 count5 establishes RAM:000c32=0 at media-state entry.
This is earlier than recognition, so the issue cannot be attributed to
recognition itself based on this observation. It does not establish whether
normal host initialization occurred or when the byte became zero.

Canonical stock1.04 cross-references to c32 contain explicit writes of idle1
and ready10/11. This is not an exhaustive audit of indirect/bulk writes.
Cold initialization clears a range containing c32 before coroutine execution.
Live fdbe7d disassembly confirms c88 bit20 selects a path through fdc28c that
jumps to fdbfa7 and bypasses the c32=1 write at fdbebf. That branch may be
legitimate in context; no production change to it is justified yet.

Diag23 reports the short branch, or the first instrumented boundary observing
zero. Passing checks briefly mask interrupts, restore the entry context and
return without LED delays. No persistent diagnostic memory is used. The final
checkpoint always reports so an all-nonzero path is distinguishable.

One long yellow marker followed by slow green pulses repeats forever:

| Count | Observation |
|---|---|
| 1 | Host-init short branch selected, before its preparation call; c32 may have any value |
| 2 | Zero immediately after the ordinary host initialization wrote1 |
| 3 | Zero at drive-initialization coroutine entry |
| 4 | Zero after drive-control initialization returned |
| 5 | Zero immediately before startup-media initialization |
| 6 | Zero immediately after startup-media initialization |
| 7 | Zero at media-state-machine entry |
| 8 | Nonzero at media-state-machine entry; no earlier check reported |

This is localization to checkpoints, not the exact instruction that changed
the byte. The checks are revisited if firmware restarts. Counts3..7 alone do
not prove the normal host-init write executed: unexpectedly skipped execution
is still possible. ISR publication of10/11 is treated as nonzero and allowed.
Count1 identifies branch selection, not a fault in that branch.

This diagnostic deliberately stops once reporting; Windows detection is not
expected. Only one repeating count is needed. Pulses and group gaps keep the
slower post-diag21 timing. The user can power off after counting. User operates
redumper; Eject held at power-on then stock restore remains recovery. No agent
flash, reset or activation command was sent.

## Implementation

Use PLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON and PLEXTOR_HOST_GATE_SNAPSHOT=zero.
This adds PLEXTOR_LED_HOST_ZERO_DIAGNOSTICS and selects the addressless
startup_host_zero_diagnostics.a51 adapter. Other modes and normal builds remove
the new hooks. Existing stages4..8 map to report points3..7. Two new command-
processor hooks check the short branch and immediately after the idle1 write.
No stock state is forced or changed by the checker. Silent returns preserve
DR8, IE, PSW and PSW1; all other general registers are untouched. Terminal
reports hold the count in R14 and reuse StartupForegroundCheckpoint.

The first target validation exposed missing PSW1 preservation in the new
diagnostic adapter. It was corrected before release; preservation checks
compare captured caller flags rather than assuming debugger assignments leave
parity/flag aliases unchanged. This was a diagnostic defect, not evidence of
the normal firmware's physical failure.

Build and target checks:

```powershell
cmake -S . -B out/agent-tests/diag23-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON -DPLEXTOR_HOST_GATE_SNAPSHOT=zero
cmake --build out/agent-tests/diag23-build --config Release --target firmware --parallel 4
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag23_target.py matrix
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag23_target.py frame
python out/agent-tests/run_diag23_continuation.py
```

Candidate size524288, SHA256
`c90fb134c091699a70d8cabeaa07db6989f8ffb15bdfcbbcb46862759647ce3e`,
checksumd9f8. Package format/both checksum expressions/stock fixed data pass.
Normal out/windows image remains exact diag18 d21d4c8e...5844e. No Ghidra
metadata changes or newly established production fix.

## Validation outcome and limits

- All27 final direct target cases pass: short-branch reports, zero/nonzero
  checks at every boundary, both IE0/80 for silent returns, and final counts7/8.
  Caller DR4/8/12, PSW/PSW1, IE and SP are preserved on silent returns; c32 is
  unchanged. The final isolated harness uses SP40. Earlier SP60 combined
  attempts timed out; a single SP60 silent-return case passed. No causal
  conclusion about SP is established.
- The maintained paired state0-to-media-entry continuation passes with the
  diag23 image: all captured RAM deltas and state08/30, savedC1SPX00000507
  match stock. The first45-second attempt timed out on reconstruction; the60-second
  retry passes. It retains explicit countdown/controller responses, and does
  not prove physical cold boot/interrupt delivery. Results:
  out/agent-tests/diag23-startup-continuation-retry/.
- Three complete seven-pulse reporter attempts exceeded60seconds without
  producing a final snapshot. No new full-frame pass is claimed. Do not repeat
  these unchanged. The frame adapter source is unchanged from diag22's passed
  slow-frame validation; final diag23 direct cases reach its correct entry
  with the expected count. This limit is retained, not interpreted as a
  physical firmware stall.
- Final image builds with zero Keil diagnostics. Ordinary firmware remains
  byte-identical to diag18 with diagnostic options OFF.

Artifact:
`out/agent-tests/diag23-host-zero-20260926/PXW5224A_diag23_host_zero.bin`.
Physical result: the user reported five green blinks. RAM:000c32 was zero
immediately before InitializeStartupMediaState. This does not prove point4
was visited: the startup coroutine also has a direct initial entry. Diag24
checks the drive-init yield/restart and each startup prefix write group; see
DIAG24_HOST_ZERO_INTERVAL.md. No exact writer or memory fault is established.
