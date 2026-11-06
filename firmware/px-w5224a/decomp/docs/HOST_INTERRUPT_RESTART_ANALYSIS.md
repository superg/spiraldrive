# Host interrupt restart and subsequent interrupt delivery

Firmware target: Plextor PX-W5224A 1.04. Canonical evidence is the live
`/validated/MONT_104.bin` function at `RAM:f8007f`.

## Confirmed defect

The host interrupt wrapper calls `HandleHostInterfaceInterrupt`. When that
worker requests restart in `RAM:0000fa`, stock rewrites its top stack bytes
for `RestartFirmware` at `RAM:fee819`, clears the request, and executes
`RETI` at `RAM:f800b5`. This releases the interrupt-in-service state before
reinitialization discards the old stack.

The previous C implementation called `RestartFirmware` directly from inside
the ISR. That nonreturning path abandoned the compiler epilogue and its
`RETI`. Clearing interrupt-enable registers and resetting the software stack
do not perform the missing interrupt return.

The [Intel 8XC251Sx manual, RETI description, printed A-115](https://www.keil.com/dd/docs/datashts/intel/8xc251sx_um.pdf)
specifies that `RETI` restores acceptance of interrupts at the serviced
priority. The stock instructions and the S251 reproduction below establish
the relevant firmware behavior; generic Intel documentation does not supply
an LC898095 hardware map.

## Correction

`HostInterfaceInterruptBody` now calls the addressless assembly boundary
`RestartFirmwareAfterInterrupt`. It masks EA, pushes a bank-zero PSW1 byte,
uses a local `ECALL` to obtain the relocated continuation address, swaps that
call's final two return bytes into interrupt-frame order, and executes `RETI`.
The continuation jumps to the ordinary C `RestartFirmware` worker. That worker
clears interrupt control and reinitializes SPX as before. The abandoned frames
are never resumed.

This adapter is necessary because an ordinary C call cannot release the
processor's interrupt-in-service state. No canonical assembly body was added.
The regular no-restart compiler epilogue remains unchanged.

## Reproduction and regression

Run from the repository root after building firmware:

```powershell
python tools/run_on_private_desktop.py -- python tests/simulator/trace_host_restart_irq_liveness.py
```

The regression first executes the real captured monitor and host cause-`0x10`
path to `InitializeAndEnterFirmwareMain` in both images. At that checkpoint,
it substitutes a small simulator-only continuation that enables and requests
a second Timer0 interrupt. A synthetic ISR writes marker `0xa5` and executes
`RETI`. The test never performs CPU RESET between the two interrupts: the
in-service latch must survive the checkpoint. Both sides use the same second
interrupt stimulus and handler. This isolates interrupt delivery from the
unmodeled drive initialization and controller behavior.

| Image | Second IRQ marker | Pending Timer0 flag | Result |
|---|---:|---:|---|
| Stock 1.04 | `0xa5` | clear | delivered |
| Pre-fix reconstruction | `0x00` | set (`TCON=0x20`) | blocked |
| Corrected reconstruction | `0xa5` | clear | delivered |

The maintained negative control patches only the debugger-loaded adapter into
a direct jump to `RestartFirmware`, recreating the defect. It must exit 1:

```powershell
python tools/run_on_private_desktop.py -- python tests/simulator/trace_host_restart_irq_liveness.py --negative-control --output out/agent-tests/host-restart-irq-negative-control
```

Both controls passed their real interrupt-to-restart-entry comparisons; only
the negative control blocked the second IRQ. This demonstrates why the earlier
restart-entry tests could pass while missing the defect.

Evidence directories:

- `out/agent-tests/restart-irq-before/`: first unmodified pre-fix reproduction.
- `out/agent-tests/host-restart-irq-liveness/`: maintained positive regression.
- `out/agent-tests/host-restart-irq-negative-control/`: expected failure.
- `out/agent-tests/interrupt-audit-after/`: restart-entry and ordinary cause-4
  DATA-IN completion comparisons, including PSW1 observation.

The host vector, host interrupt worker, and command processor tests pass. The
vector host test explicitly keeps assertions enabled in Release builds. Keil
compile/link has zero diagnostics, and the packaged image passes checksum and
stock fixed-data verification. LED diagnostics are OFF.

## Physical status and remaining hypotheses

Candidate SHA-256:
`0da270c08f862848d60f420221bf23de11378c389a8a52bdefee19b9d7a40669`.
Checksum `0xdca7`; size 524,288 bytes. Archived as
`out/agent-tests/diag14-interrupt-restart-fix-20260926/PXW5224A_diag14_interrupt_restart_fix.bin`.
This is a normal-running image with the interrupt fix, not a terminal LED build.

Subsequent physical evidence is now recorded in
[`DIAG14_ENUMERATION_AND_TRAY_ANALYSIS.md`](DIAG14_ENUMERATION_AND_TRAY_ANALYSIS.md):
the supplied power-on capture contains successful Plextor INQUIRY payloads/CSWs
and continued optical commands. The tray still moves without a captured host
tray command; a separate debounced-input address defect was then corrected.
Candidate identity is inferred from the user's workflow, not live readback.

Before that capture, the user reported intermittent
brief or full tray-motor actuation only on rebuilt/diagnostic firmware, not on
restored stock under the same power-on conditions. Missing subsequent interrupts
could disrupt command handling and time-dependent control, but responsibility
for the observed tray behavior and enumeration failure is unproved.

Another open physical assumption is the linker's XDATA allocation at
`RAM:010000..017fff`, used for compiler locals and diagnostic state. The hardware
map does not establish writable RAM backing for this range. Simulator writes
cannot establish that backing. If the interrupt fix is insufficient, validate
this region's retention and alias behavior with a diagnostic whose execution
and report do not themselves depend on XDATA, and trace the first tray-control
transition and its input predicates. Do not move allocations to another
unverified region speculatively.

Flashing remains user-operated through redumper. The established recovery is
holding Eject at power-on and restoring known-good stock. This investigation
sent no hardware commands.
