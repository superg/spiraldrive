# Diag21: host state at media entry

Diag20 physically reached its eight-pulse media-state entry checkpoint, with
the tray closed and no Windows enumeration. The paired simulator continuation
through its host-latch wait passes both immediately-ready and two-yield cases.
There is no newly established production defect in that section.

Diag21 removes all earlier checkpoint pauses and takes a terminal snapshot at
ProcessDriveStateMachine entry (canonical stock 1.04 RAM:f84f6a). It reads IE
before masking EA, then reads RAM:00faeb and RAM:000c32. It repeats the frozen
count using the existing register/stack-only diag20 frame adapter. No Timer0
deadline, compiler XDATA storage, or live re-reading selects the count.

One long yellow marker is followed by N green pulses, repeating:

| N | EA at entry | RAM:faeb | Packet RAM:0c32 |
|---|---|---|---|
| 1 | disabled | zero | idle (1) |
| 2 | disabled | zero | other |
| 3 | disabled | nonzero | idle (1) |
| 4 | disabled | nonzero | other |
| 5 | enabled | zero | idle (1) |
| 6 | enabled | zero | other |
| 7 | enabled | nonzero | idle (1) |
| 8 | enabled | nonzero | other |

RAM:faeb is set by the host ISR, but can also be preset by FirmwareMainLoop;
nonzero alone does not prove an interrupt occurred. Packet other than 1 is
reported literally, not proof of a valid received command. EA is the global
enable, not evidence that every individual interrupt source is enabled.
The snapshot intentionally stops firmware execution here, so no Windows
enumeration or subsequent tray behavior is expected from this diagnostic.
No frame is inconclusive: it could mean this point was not reached without
diag20's pauses, among other causes.

`PLEXTOR_LED_HOST_GATE_DIAGNOSTICS` defaults OFF and is mutually exclusive with
the other three LED modes. Only the stage8 hook survives; target listings show
the call at the first instruction of ProcessDriveStateMachine. The assembly
uses explicit EDATA reads for both near RAM bytes. The frozen count is R14,
which the reused frame adapter preserves. This is an addressless diagnostic
exception, not a replacement canonical assembly implementation.

Build:

```powershell
cmake -S . -B out/agent-tests/diag21-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON
cmake --build out/agent-tests/diag21-build --config Release --target firmware --parallel 4
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag21_target.py
```

The image has 524,288 bytes, SHA256
`1a6e49d9b7ffc2554236f52f30d7e2575dab45655b2c0b5b7b29a25a3d8de541`,
checksum `6fa0`. Both checksum expressions, format and stock fixed-data checks
pass. Final build has zero Keil warnings/errors.

## Physical result (2026-09-26)

The user reports eight green blinks on diag21. This decodes to EA enabled at
snapshot entry, RAM:00faeb nonzero, and RAM:000c32 different from idle state1.
The exact packet-state value and command identity remain unknown. This rules
out global interrupt disable and a zero host-wait latch at this boundary; it
does not establish individual interrupt enables, command validity/completion,
or a prior host IRQ (main can preset faeb). Installed bytes were not read back.
Next investigate the non-idle packet state and coroutine0 processing alongside
media recognition. Do not describe count8 as eight sequential startup stages.

The user also asks whether blink speed increased. Diag20 and diag21 archive
the identical startup_foreground_diagnostics.a51 source: pulse delay counters
and frame pauses were not changed. Diag21 removes earlier startup frames and
repeats the terminal frame. Actual wall-clock pulse duration was not measured;
unchanged instruction delays alone do not measure the hardware clock.

After that observation the user requested slower future blinks, without using
a phone recording. The maintained foreground reporter now uses 32 outer delay
passes instead of 8, making pulses, dark intervals and group pauses about four
times longer at the same CPU clock. Archived diag20/21 are unchanged; this
source change applies to future builds. No reflash is requested for timing alone.

All eight snapshot combinations pass in S251. The eight-count case also runs
a complete unmodified LED frame and stops at the next frame call, still
reporting eight after the harness changes the live latch and packet bytes.
P3 low-bit trace is 2,2,3 followed by eight pairs of 1,3; IE remains zero during
the frame. No firmware instructions are patched. Two initial snapshot-3
attempts timed out with a long debugger deadline; the bounded 1000-state
snapshot probes pass. Full-frame validation retains the longer deadline.
These harness timeouts are not firmware-fault evidence.

With every diagnostic option OFF, the rebuilt ordinary image remains exactly
diag18 SHA256 `d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`.

Paired startup-to-recognition cases are maintained in
`tests/simulator/cases/state0-host-gate-continuation.json`. The archived run
used identical cases from `out/agent-tests/post-stage8.json`. Both pass;
synthetic RAM:faeb reads are explicitly part of the model. Stock canonical
RAM:fdd3c3 writes faeb=1 on host-ISR entry; RAM:fdcebd..fdced9 independently
preset it under main-loop configuration conditions. Ghidra metadata unchanged.

Artifact and supporting evidence:
`out/agent-tests/diag21-host-gate-20260926/PXW5224A_diag21_host_gate.bin`.

User-operated redumper flashing and Eject-held-at-power-on recovery followed
by stock restore remain the procedure. No agent flash/reset/activation command
was sent. Ask for one repeating green count; no Bus Hound capture is needed.
