# Diag22: host packet state at media entry

Diag21 physically reports count8: global interrupts enabled before capture,
RAM:00faeb nonzero, and RAM:000c32 not idle1. It deliberately halts at media
entry and repeats the frozen result. This did not identify the exact packet
state, command opcode, interrupt source enables, or command completion.

Diag22 stops at the same ProcessDriveStateMachine entry (stock 1.04
RAM:f84f6a), masks EA, and reads RAM:000c32 once into R13. It freezes a count
in R14 and repeats it. No earlier diagnostic pauses, controller reads, timer
deadline, compiler XDATA, or writes to firmware state are introduced.

After each long yellow marker, count the slow green pulses:

| Count | Captured RAM:000c32 | Interpretation |
|---|---|---|
| 1 | 01 | Idle at this checkpoint |
| 2 | 10 | State published for an ATA command |
| 3 | 11 | State published for a SCSI packet |
| 4 | 04 | Status4, routed to completion cleanup |
| 5 | 00 | Zero rather than initialized idle1 |
| 6 | Anything else | Unexpected value; exact byte not encoded |

Only one count is needed, not a sequence of fields. It repeats indefinitely;
the user can power off after counting. Normal execution stops deliberately,
so Windows detection is not expected. Recovery remains Eject held at power-on
then stock restore through user-operated redumper. No agent hardware writes,
resets or activation commands are used.

Counts2/3 identify the captured state, not the opcode or a valid/completed
transaction. Count1 on a new run can differ from diag21 due to timing; do not
reinterpret the old observation. Count6 retains an explicit unknown category.

## Firmware evidence

Live canonical /validated/MONT_104.bin disassembly:

- RAM:fdbebd..fdbebf initializes RAM:000c32 to1.
- RAM:fdc0e7..fdc0f0 waits/yields while it equals1.
- RAM:fdc107..fdc188 distinguishes04,11,10;11 reads the 12-byte packet,
  10 proceeds directly, and04/other selects cleanup. It then resets c32 to1.
- RAM:fdd6c8..fdd6d0 publishes c32=11 and protocol c78=1.
- RAM:fdd74a..fdd751 publishes c32=10 and protocol c78=0.

Ghidra metadata unchanged. This is a new diagnostic, not a production fix.

## Build and validation

Use the existing OFF-by-default host-gate mode with snapshot selector packet:

```powershell
cmake -S . -B out/agent-tests/diag22-build -DKEIL_C251_ROOT=C:/Keil_v5/C251 -DBUILD_TESTING=OFF -DPLEXTOR_LED_HOST_GATE_DIAGNOSTICS=ON -DPLEXTOR_HOST_GATE_SNAPSHOT=packet
cmake --build out/agent-tests/diag22-build --config Release --target firmware --parallel 4
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag22_target.py matrix
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag22_target.py baseline
python tools/run_on_private_desktop.py -- python out/agent-tests/check_diag22_target.py frame
```

The selector defaults to flags, retaining the previous snapshot implementation.
Packet mode links the addressless startup_host_packet_diagnostics.a51 adapter
through the same single startup hook. The existing LED frame adapter now uses
32 outer delay passes instead of8: pulse on/off intervals and group pauses
are approximately four times longer at a fixed hardware clock. This follows
the user's request for counting by eye, without phone recording. Absolute
wall-clock durations are not calibrated. Archived diag20/21 stay immutable.

Image size524288, SHA256
`6dc269d75134152c1ce82f6515313b7c1ab3d422e21ee6af7d3113be9ec6f8a5`,
checksum6b94. Format, both checksum expressions and fixed-data checks pass;
final build has zero Keil diagnostics. No linker-region expansion.

Nine actual S251 snapshot inputs pass: 00,01,04,10,11 and unexpected values
02,0f,12,ff. All six report categories are covered. The complete six-pulse
frame passes, then reaches the next report with count6/rawff still held after
the harness changes the live byte to1. IE is zero throughout the frame.
Instructions are not patched. The archived diag21 build supplies the matched
timing control: green-on duration2621456 states versus10485752 in diag22,
ratio3.9999725. Absolute hardware seconds remain unmeasured.

An initial256-input combined harness exceeded60seconds without a result;
the bounded nine-input harness passes. A first timing comparison against the
older diag20 trace measured3.33x and failed the expected4x assertion. The
correct comparison against the user's preceding diag21 measures4x above;
do not infer a hardware clock change from different linked simulator runs.

Normal out/windows firmware rebuilt with all diagnostic modes OFF remains
exact diag18 SHA256
`d21d4c8e347f85b236bef0e7a4ccc97c4fa29a0cd5c201a3f56cbff6c7a5844e`.
Target listings confirm the only startup snapshot call is at media-state
entry. Neither earlier checkpoints nor diagnostic state storage are added.

Artifact:
`out/agent-tests/diag22-host-packet-20260926/PXW5224A_diag22_host_packet.bin`.
Physical result: the user reports **five green blinks**. This means the captured
RAM:000c32 was zero at media-state entry. It does not prove the ordinary host
initialization ran and its value was subsequently lost: the command processor
has a short initialization branch (c88 bit20) that bypasses the idle1 write.
Other possibilities, including an indirect/bulk write or invalid execution/
memory state, remain open. Do not force c32=1 as an assumed repair.
