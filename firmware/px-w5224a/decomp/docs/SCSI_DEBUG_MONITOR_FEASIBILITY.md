# SCSI debug monitor: feasibility and next work

2026-09-26. The user reports that flashing is slow and rejects further
one-observation-per-flash LED diagnostics. Investigate an interactive memory
interface offline before proposing another hardware image.

## Confirmed existing capability

Stock 1.04 has a CPU-memory DATA IN command:

```
F5 00 00 00 AA AA AA AA LL LL
```

Address is big-endian CDB[4..7]; length is big-endian CDB[8..9]. Selector
CDB[3] is zero. The maintained host helper restricts addresses to 24 bits.
This reads the CPU data view; it is not a generic SFR/bit-space debugger.

Live canonical inspection on this date reconfirmed:

- `/validated/MONT_104.bin`, RAM:fcb61f: source loaded at RAM:fcb730,
  length at RAM:fcb73c, DATA IN helper called at RAM:fcb832.
- `/validated/read_memory_ff0000.debug`, RAM:ff2a3e: selector at
  RAM:ff2a44, source/length at RAM:ff2a4c/ff2a58, DATA IN at RAM:ff2aad.
- Captured monitor RAM:ff5026 dispatches through the fixed table at
  RAM:ff7a00 + opcode*4 (RAM:ff5105..ff5123). This is not an observed
  writable RAM dispatch table or an existing arbitrary-code execution hook.
- Captured monitor RAM:ff57bc installs RAM:ff5026 as its command coroutine
  and runs its own scheduler. Its operation does not depend on the rebuilt
  main firmware completing media startup.

Earlier physical DATA IN captures prove F5 works in stock normal and
protected recovery modes. See `docs/HARDWARE_PLATFORM.md` and
`out/agent-tests/windows-recovery-full-backup-current-20260925/`.
`tools/probe_readonly_w5224a.py` already has Windows SCSI pass-through and
identity checking, but deliberately supports fixed probes rather than an
interactive arbitrary-memory console.

Standard WRITE BUFFER modes 2/4 write controller-buffer offsets. They are
not arbitrary CPU-memory writes. Mode 5 enters firmware programming and
activation; it is not a RAM upload/run command. No existing general CPU RAM
write or uploaded-code execution command has been established by this work.

## Proposed debug environment (not implemented)

Use a dedicated main-image diagnostic environment with a small command
service based on the working monitor transport. Preserve the protected
recovery image. Establish host reachability before starting media/tray code.
Calling the stock monitor loop unchanged would provide reads, but its fixed
dispatch table would not supply new write/run commands.

The useful interface would provide:

1. Identity/protocol version and status, then repeated memory reads.
2. Writes to explicitly reserved RAM, with readback and a distinct operation
   for any controller or SFR access. MMIO is not ordinary scratch memory.
3. A RAM trace ring with sequence numbers and overflow reporting, drained
   through DATA IN. Record multiple events per experiment instead of LED counts.
4. Bounded test operations that return to command service. Upload/run of
   small native routines is a later capability, conditional on proving the
   executable RAM mapping, available size, ABI, and return path.

This could allow many experiments after one installation. It does not mean
the whole firmware can run from RAM, nor that a monitor can respond while
the CPU is trapped in arbitrary test code. Shared interrupts, controller
registers and buffers can also destroy the transport. Cooperative checkpoints
or a proven independent timeout/recovery mechanism are required for tests
that can wait indefinitely. A reset into recovery can reinitialize RAM, so
it cannot be assumed to preserve the failed program's state.

## Offline acceptance before any hardware proposal

- Trace monitor initialization, packet ingress, command admission, DATA
  IN/OUT completion, interrupt routing and scheduler dependencies together.
- Locate and verify a reserved RAM region; do not assume compiler XDATA
  RAM:010000..017fff has physical backing.
- Demonstrate repeated commands, bounded RAM write/readback, trace draining,
  and return from a test using the actual native transport/scheduler in S251.
  State all simulated controller inputs and distinguish these from hardware.
- Keep flash programming out of the debug protocol. Package only after the
  above results, with the existing Eject-held stock-restore recovery strategy.

No device command, flash, reset, or new hardware image was issued for this
feasibility investigation. The next step is transport/dependency validation,
not another blink image.
