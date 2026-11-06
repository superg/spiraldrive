# Keil 80251 differential simulator tests

This directory contains every version-controlled part of the simulator test
environment. Generated projects, converted images, debugger scripts, logs,
catalogs, traces, and reports belong in `out/simulator/` and must not be
committed.

Normal test execution uses the pinned `metadata/W5224A104.json` snapshot and
does not require Ghidra. The snapshot is bound to the stock image by size and
SHA-256 and contains all 1,717 canonical functions. Source tags and the L251
map associate each canonical entry with its reconstructed symbol and linked
address.

Ghidra is a developer-only metadata authoring dependency.
`emulator-refresh-metadata` exports a disposable candidate under
`out/simulator/`; it never silently overwrites the reviewed snapshot.

## Run every function

From the repository root, supply only the immutable stock firmware image:

```sh
python3 tests/simulator/run_all.py data/W5224A104/MONT_104.bin
```

The command locates the installed Keil C251 toolchain, validates the pinned
offline metadata against the image hash, configures a serial firmware build,
runs the S251 smoke test, and differentially executes the generated case for
each of the 1,717 canonical functions. It also runs the bounded integrated
SCSI command-completion scenario through its real dependency graph. It does
not launch or require Ghidra.
Generated projects, scripts, memory applications, traces, results, and the
aggregate JSON, text, and JUnit reports are written below `out/simulator/`.

The complete direct-function suite needs only the stock image. A separate
64-KiB `RAM:ff0000` runtime-monitor capture is optional and is not silently
assumed to exist under ignored `data/`. Supply one only for an opt-in
monitor-contract case:

```sh
python3 tests/simulator/run_all.py data/W5224A104/MONT_104.bin \
  --monitor data/read_memory_ff0000.debug
```

A mismatch is a successful test execution that found different observable
behavior, so the command returns nonzero until all current cases match. Rerun
the same command to resume completed pass/mismatch cases after an interruption;
the runner rejects stale results if an image, map, metadata snapshot, or case
selection changed.

On native Windows, run long uVision-backed commands on the repository's private
desktop launcher so uVision cannot activate a window on the user's interactive
desktop:

```powershell
python tools/run_on_private_desktop.py -- python tests/simulator/run_all.py `
  data/W5224A104/MONT_104.bin
```

The launcher creates a named non-interactive `WinSta0` desktop, starts the
entire command tree there at below-normal priority, and owns it with a
kill-on-close job. This is stronger than `SW_HIDE`: uVision may create the GUI
it needs for S251 automation, but that GUI cannot take focus from the normal
desktop. Interrupting the launcher also terminates its current uVision child.

For the complete generated catalog on the configured Windows build, use:

```powershell
python tools/run_windows_generated_catalog.py
```

That driver preserves the simulator's normal resume identity, performs the
broad pass with a 10-second uVision limit, and automatically retries only the
remaining errors with 30-, 60-, and 120-second limits. It stops immediately if
a real stock/reconstruction mismatch appears. Each pass also enables the
private launcher's job-scoped descendant watchdog. If uVision survives its
normal runner timeout, the watchdog terminates only that pass's overdue
`UV4.exe` after a 30-second grace period so the catalog can continue.

## Targets

For a focused check that the host restart path releases interrupt-in-service
state before initialization, run:

```powershell
python tools/run_on_private_desktop.py -- python tests/simulator/trace_host_restart_irq_liveness.py
```

This executes the real stock/reconstructed cause-0x10 restart entry, then a
declared second IRQ without resetting the CPU. See
[`HOST_INTERRUPT_RESTART_ANALYSIS.md`](../../docs/HOST_INTERRUPT_RESTART_ANALYSIS.md)
for the debugger-only negative control and physical limits of this test.

Configure a firmware build with `KEIL_C251_ROOT` set, then use:

```sh
cmake --build out/simulator/build --parallel 1 --target emulator-smoke
cmake --build out/simulator/build --parallel 1 --target emulator-catalog
cmake --build out/simulator/build --parallel 1 --target emulator-pilot
cmake --build out/simulator/build --parallel 1 --target emulator-report
cmake --build out/simulator/build --parallel 1 --target emulator-differential
```

`emulator-smoke` compiles and links `smoke.c`, starts the installed S251
debugger through Wine, executes an initialization file, writes a byte through
the debugger, saves it, and verifies the saved marker. This catches a missing
license, broken Wine launch, bad simulator selection, and initialization files
that were accepted by the project but never executed.

Keep Keil targets serial (`--parallel 1`). Parallel Wine invocations compete
for the single compiler installation and can make otherwise valid large
translation units hit the driver timeout. `run_all.py` enforces this itself.
On Windows, the configured CMake uVision targets apply the private-desktop
launcher automatically, including a 90-second job-scoped uVision watchdog for
the targets' default 60-second runner limit. Prefix only direct `simulator.py`
or `run_all.py` invocations as shown above.

The smoke probe also records S251's known inability to expose LC898095
high-HDATA writes such as `RAM:2ba5d8`. Direct differential cases relocate
those operands into S251's modeled window and patch stock and reconstructed
instructions symmetrically. Ordinary near, extended, controller, firmware-ROM
read, and instruction execution probes must still pass without relocation.

`emulator-catalog` validates the committed metadata and supplied stock image.
Despite the historical target name, it does not launch Ghidra.

`emulator-report` accounts for exactly 1,717 canonical source tags. A function
is not upgraded from `untested` merely because another test happened to call
it. Test cases live in `cases/` and conform to `case.schema.json`.

`emulator-pilot` is the quick feasibility target. It builds the arithmetic
translation unit with C251 O5/SIZE, executes `MinU8` from the stock image and
the reconstructed ABS from identical state, and compares its declared ABI
output after both executions reach the corresponding return instruction.
`emulator-differential` runs the small committed feasibility suite against the
complete reconstructed `main.abs` and therefore depends on the full firmware
link. This includes the integrated command-completion path. The one-command
runner additionally invokes `emulator-generated-differential`, which executes
the offline-generated direct case for all 1,717 canonical functions.

The evidence vocabulary is deliberately precise: `smoke`, `branch-covered`,
`differential-fuzzed`, `scenario-tested`, and `exhaustive` are different
claims. Only a test that enumerates its complete declared state space may use
`exhaustive`.

## Direct runner use

```sh
python3 tests/simulator/simulator.py prepare \
  --stock data/W5224A104/MONT_104.bin \
  --output out/simulator/images

python3 tests/simulator/simulator.py report \
  --source-root src --cases tests/simulator/cases \
  --output out/simulator/report
```

Add `--monitor data/read_memory_ff0000.debug` to the low-level `prepare`
command when a case declares `load_monitor: true`. Such a case fails clearly
if the differential runner was not given a monitor image.

The `cases/integration.json` suite includes packet-ingress READ BUFFER
mode-3 capacity and media-dependent READ CAPACITY(10) responses with modeled
DATA IN completion, plus a mode-4 WRITE BUFFER staging overrun rejected
before transfer or programmer entry. The READ CAPACITY case processes an
unsupported second packet in the same coroutine run and checks both command
completions. The full fourteen-case integrated run is retained at
`out/agent-tests/integration-gates/fourteen-controller-xdata-fix-run/results.json`
on the current reconstructed image.

The `cases/integration.json` suite includes a two-call dispatcher scenario:
`second_call_memory` writes the REQUEST SENSE CDB only after the first call
returns to its declared trampoline checkpoint with a balanced stack. The
second call then executes in the same RAM/controller state, with a timed
host-transfer completion and an observed 18-byte sense payload.

The complete 19-case current-image monitor suite passes at
`out/agent-tests/integration-gates/monitor-nineteen-current-run/results.json`.

A monitor case can declare `"fill": {"length": 4096, "value": 255}` in a
symmetric `code_patches` entry to model erased image bytes compactly. The
runner checks the byte and length bounds and applies the fill to each image's
own memory view; the original firmware files remain unchanged.

The optional `monitor-cases/integration.json` suite checks the captured
runtime monitor's main-mode and monitor-mode Timer0 returns, including one
pending-flag interrupt during coroutine-zero startup followed by a scheduler
resume. A separate physical-reset checkpoint verifies the first switch into
`ScsiCommandProcessorCoroutine0` and the saved scheduler/coroutine stack
pointers. That case uses `expected_observations` to require one timer tick and
specific SP/SFR values in each image; the comparator rejects a matched no-op.
It also checks monitor-mode
controller-event and controller-packet returns, main-mode packet short-response and ten-byte-capture/RETI cases, a main SFR-E1 router/RETI
case with an explicit S251 source-vector alias, monitor and no-cause main
host-interface router/RETI cases reached through S251's generic Timer0 source,
valid-image handoff, modeled first-sector-erased and first-block-erased main images routed
to recovery, and
corrupted-image, forced-recovery, and checksum-valid self-loop image
branches. Two bounded operation-5 cases also launch the captured runtime
flash loader with declared `0x2000` and `0xbf00` identification results, verify
the relocated programmer copy, and reach the first unlock-store edge of each
branch. `tests/simulator/trace_runtime_flash.py` then resumes S251 at
each mapped flash-window CPU store in either declared branch. It records
fourteen matching stock/reconstructed `0x2000`-branch store checkpoints
through `RAM:0008c5`, plus six matching `0xbf00`-branch checkpoints through
`RAM:0008f9`. The retained reports are
`out/agent-tests/integration-gates/runtime-operation5-2000-cpu-store-trace/results.json`
and `out/agent-tests/integration-gates/runtime-operation5-bf00-cpu-store-trace/results.json`.
These are CPU stores, not confirmed chip effects. The tool conditionally
replays the first complete six-store command through
`tests/simulator/flash_bus.py` against disposable images and records the
operation-5 allowed-range rejection. Its high-byte command lane and
`+0x80000` execution alias are hypotheses; busy timing and later bare
`0x3000` stores remain unmodeled. Run the model checks with
`python3 -m unittest tests.simulator.test_flash_bus`. Reproduce the CPU trace
from a passing `runtime-operation5-first-unlock-checkpoint` result with:

```sh
xvfb-run -a -s '-screen 0 1024x768x24' \
  python3 -m tests.simulator.trace_runtime_flash \
  --device-code 2000 \
  --source-run out/agent-tests/integration-gates/runtime-operation5-versioned-run \
  --output out/agent-tests/integration-gates/runtime-operation5-cpu-store-trace \
  --uv4 /home/hbrych/.wine/drive_c/Keil_v5/UV4/UV4.exe \
  --stock-image data/W5224A104/MONT_104.bin \
  --reconstructed-image out/simulator/build/firmware/Release/MONT_reconstructed.bin
```

For the alternate branch, use `--device-code bf00` and the passing
`runtime-operation5-bf00-versioned-run` as `--source-run`.

The bootstrap cases launch through a
scratch trampoline so checksum-covered main bytes stay intact. Their `code_patches` use
`stock_image_slice` and `reconstructed_image_slice` to load the two-byte stored
checksum from each image at file offset `0x79ffe` into S251's code view at
`RAM:ff9ffe`; S251 otherwise exposes the stock word to both executions.
Each slice declares `offset` and `length` and is resolved from the current image
file at run time. The forced-recovery case combines state-timed SFR latch writes
with `pause_port_inputs`: the runner stops at the declared monitor PC, records
and validates it, sets external `PORT0/PORT1` pins, then resumes to the recovery
checkpoint. The checksum-valid self-loop cases adjust the current image's
stored checksum word by the declared minus-300 byte-sum delta. `timed_writes`
also accepts `PORT` addresses 0 through 3 for pin stimuli that can run inside
an S251 signal.

The simulator has no LC898095 peripheral model. Tests must therefore declare
all hardware inputs and compare ordered normalized MMIO/SFR events. An absent
or exhausted input sequence is a test error, not an implicit zero.

The executor supports register, SFR, bit, and RAM initialization/observation, execution
limits, normal/`ERET`/`RETI` return-site recognition, normalized intercepted
dependencies, ordered SFR/controller transcripts, and simulator-safe HDATA
bank relocation for functions that declare it in the pinned metadata.
Initial SFR bytes are declared with the `sfr` case field, so platform page
registers do not depend on µVision reset defaults.

Cases whose dynamic buffer pointers cannot use the same physical address in
both S251 execution modes may declare `memory_relocations`. The runner writes
the stock and reconstructed pointer values independently, mirrors the stock
result into the reconstructed comparison window after execution, and excludes
only the physical pointer bytes from the ordinary RAM-delta comparison. The
comparison window must remain inside captured near or controller RAM, so the
full relocated payload is still compared byte-for-byte.
When S251's native-binary pointer encoding differs from its physical result
address, `stock_result_base` (and, if needed, `reconstructed_result_base`)
records that distinction explicitly.

Cases that store relocated firmware entry points in RAM may declare
`code_pointer_memory`. The runner excludes the raw pointer bytes from the RAM
delta and compares the canonical function identity resolved through each
image's code-address map; an unmapped or wrong target remains a mismatch.

When two implementations perform different numbers of hardware reads, cases
may use `stock_hardware_reads` and `reconstructed_hardware_reads` (and the
corresponding write fields). Each image receives enough deterministic input to
terminate, while the normalized transcripts remain observable and therefore
produce a mismatch rather than being hidden by the harness.

A constant hardware read may use `value`, `min_reads`, and `max_reads`
instead of a fixed `values` list. The simulator emits a compact bounded
signal, records every actual read, fails on exhaustion or a count outside the
declared range, and compares the ordered transcript between images. The sum
of declared maxima and other hardware events must fit the selected trace.
Cases needing more than 255 events may set `hardware_trace_capacity` to 1023;
that opt-in uses separate simulator scratch at `RAM:014000..015fff` and a
16-bit event count. It preserves every event and still rejects overflow.
This is useful for repeated controller-ready and SFR:D7 completion polls;
the values still describe an explicit offline hardware model, not observed
drive behavior.

For controller-window read/write signals at `RAM:018000..01ffff`, S251's
`rwatch`/`wwatch` expressions use `X:0x8000..ffff`. A plain 24-bit debugger
watch at `0x018000` does not intercept the firmware's `MOVX @DPTR` access.
The runner records the canonical 24-bit controller address in event traces.
For stock 24-bit `@DR` controller reads, `watch: "RAM24"` on the read item
uses a plain 24-bit `rwatch(0x018015)` while retaining the logical
`CONTROLLER` event identity. Write items accept the same watch option for
diagnostics; S251 may expose an old value through one watch mode, so confirm
write values before using them as acceptance evidence. A case can add
`observe.direct` probes with `space: "XDATA"` and a 16-bit address such as
`0x8046` to read the debugger's XDATA view at its checkpoint; `RAM24` probes
read the plain 24-bit view. These views are simulator observations and need
instruction or bus evidence before they are treated as physical register state.
A signed `reconstructed_nonreturn_checkpoint_offset` can identify an internal
checkpoint in the same linked translation unit before the exported entry.

`repeat_nonreturn_memory` resumes a bounded nonreturning coroutine after its
first declared checkpoint, after verifying that checkpoint PC and its stack.
The runner applies the listed memory changes, lets the same coroutine process
a second packet, and requires a second stop at the same checkpoint.

### Auxiliary mode-5 accepted checksum and pre-flash handoff

The current 1.04 image has a paired conditional continuation from
`write-buffer-mode5-auxiliary-zero-length-finalize-first-yield`. First,
`trace_mode5_aux_clean_scratch.py` moves the runner's observation records out
of the auxiliary checksum range. Then `trace_mode5_aux_clean_acceptance.py`
sets identical candidate bytes and changes only the three checksum source-bank
immediates from the runner's default bank `0x00` to simulator bank `0x01` on
both images. The candidate checksum passes and both reach the auxiliary
configuration staging function. Finally,
`trace_mode5_aux_clean_handoff.py` declares the 96 primary-configuration
bytes from the hash-pinned runtime capture, applies the same simulator bank
substitution to the staging write, checks the full copied region, and stops at
`RAM:ff4000` before executing the flash loader. Reports:
`out/agent-tests/m5auxcleanscratch_final/results.json`,
`out/agent-tests/m5auxcleanaccept_final/results.json`, and
`out/agent-tests/m5auxcleanhandoff_final2/results.json`.

Run these scripts in that order with `--source-run`, `--output`, and `--uv4`.
The first source is `out/agent-tests/i24checksumfix`; each later source is the
previous script's output. The bank substitutions are S251 fixture controls,
not firmware edits or physical controller/flash evidence.

### Main mode-5 checksum bank boundary

`trace_mode5_header_rejection.py` can be rerun on the current 24-case
integrated report. `trace_mode5_main_checksum_entry.py` then replaces its
malformed staged identity with the immutable stock identity and verifies the
native main-checksum call in both images. From that paired entry,
`trace_mode5_main_checksum_boundary.py` records the S251 bank-`0x02` stop.
The retained reports are
`out/agent-tests/m5mainchecksum_entry_probe/results.json` and
`out/agent-tests/m5mainchecksum_boundary_final3/results.json`. The boundary
report verifies progress and source cursors; it does not establish checksum
rejection or accepted main-image programming.
