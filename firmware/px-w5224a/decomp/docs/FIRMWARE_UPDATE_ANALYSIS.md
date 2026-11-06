# Firmware update and runtime flash path

This report applies to PX-W5224A firmware 1.04 and the 64-KiB runtime capture
`data/read_memory_ff0000.debug`. Main-firmware addresses are qualified as
`RAM:`. Runtime routines absent from the downloadable image are proven by the
capture and must remain a separate reconstruction/build target.

## READ/WRITE BUFFER CDB contract

`HandleWriteBufferCommand` at `RAM:fd2741` and `HandleReadBufferCommand` at
`RAM:fd2e21` parse the standard ten-byte fields as:

```text
mode      = CDB[1] & 7
buffer_id = CDB[2]
offset    = big_endian_u24(CDB[3..5])
length    = big_endian_u24(CDB[6..8])
```

WRITE BUFFER accepts modes 0, 1, 2, 4, and 5. The update contract is:

- mode 4, buffer ID 0: transfer ordinary candidate-image chunks into the
  controller staging buffer;
- mode 5, buffer ID 0: validate, program, and activate the main firmware;
- mode 5, buffer ID `0xff`: validate, program, and activate the auxiliary,
  recovery, and configuration portion.

Other mode-5 IDs do not form the normal firmware-update path. In particular,
values `0x80..0x84` observed elsewhere in an updater trace are not buffer IDs
accepted by these branches; they likely came from CDB byte 1.

The retained stock updater `PXFirm3.exe` (SHA-256
`a564786456d9cd1b16ab9c5e0177ab768d949ec923a78be91b996647a4dd50cd`)
provides independent host-side confirmation. Its hash-specific x86 routine at
VA `0x00401790` constructs the ten-byte WRITE BUFFER CDB and submits it with
`IOCTL_SCSI_PASS_THROUGH_DIRECT` (`0x0004d014`). The update loop at VA
`0x00406320` divides the configured 524,288-byte image by the configured
16,384-byte buffer size. With `Use Update After Cmd=Yes`, it sends 31 buffer-ID
zero mode-4 chunks at offsets `0x00000..0x78000`, then sends the final
16,384-byte chunk at offset `0x7c000` as buffer-ID zero mode 5. The generic
pass-through wrapper assigns a 10-second timeout to ordinary commands and a
600-second timeout to WRITE BUFFER mode 5. This proves the stock host command
sequence, not that a reconstructed payload is safe to program.

The same hash-pinned updater does not reject the monitor's `0.00` revision in
its later-version gate. Its check at VA `0x00405cb0` tests the first byte of
the candidate revision at `0x00405dd7` and the selected drive revision at
`0x00405de7`; if either begins with ASCII `0`, control reaches the accepted
successor at `0x00405ead` without the floating-point later-version comparison.
The offline plan generator now maps those virtual addresses through the PE32
section table and requires the exact branch and success-return instruction
bytes before accepting the updater, in addition to requiring its whole-file
hash.
This is static updater compatibility evidence, not proof that a recovery-mode
drive enumerates or completes a transfer. The positive `0.00` read-only
preflight remains required.

`tools/firmware_update_plan.py` reproduces that 32-command sequence without
opening a device. It hash-checks the retained updater and INI, applies the
strict image and stock-fixed-data validator, and records each CDB and payload
hash in a JSON plan. It can bind the existing normal-mode full backup and an
optional future `--recovery-report`. The recovery binding requires a passing
revision-`0.00` profile with exactly nine fixed probes plus the 32-chunk full
backup, all 41 commands recorded as DATA IN, the current helper hash, all 15
recovery checks, and cryptographically verified captures. No positive recovery
report exists yet. The generated plan is preparation evidence only; it has no
DATA OUT backend.
The plan also partitions all 512 KiB by the recovered operation-3 contract:
candidate code/constants at offsets `0x00000..0x6ffff`, stock-identical fixed
data at `0x78000..0x79ffd`, and the candidate checksum at `0x79ffe..0x79fff`
are intended programmer inputs; the `0x70000..0x77fff` recovery hole and
`0x7a000..0x7ffff` erased tail are required to equal stock and are marked
preserved. Per-range candidate and stock hashes are recorded. This is a CPU/
staging range contract, not proof of physical address aliasing or chip effects.

READ BUFFER accepts modes 0–3. Mode 0 returns a capacity descriptor followed
by data, mode 1 returns a descriptor plus offset data and has a special buffer
ID 2 reserved-region case, mode 2 reads data directly, and mode 3 returns the
capacity descriptor.

`ExecuteBufferDataTransfer` at `RAM:fd3124` receives transfer length in `DR0`,
buffer address in `DR4`, and SCSI opcode in `ACC`. Its ACC status is an enum:
4 means failure, while 5 means success or an empty transfer.

## Vendor F5 read windows

Vendor opcode F5 is distinct from standard READ BUFFER. Mode zero reads the
caller-supplied 32-bit address and 16-bit transfer length. Four internal fixed
window selectors are now instruction- and ABI-validated:

| F5 mode | Source | Available length |
|---:|---:|---:|
| 1 | `RAM:00fb6a` | 12 bytes |
| 5 | `RAM:00dd2f` | `TrackTableHighIndex * 8 + 63` bytes |
| 6 | `RAM:00fb7a` | 18 bytes |
| 7 | `RAM:00116e` | 92 bytes |

The helpers at `RAM:fcb861..fcb8c2` write the chosen source and length through
the `DR0` and `DR4` output addresses owned by `HandleScsiOpcodeF5`. These are
exact software windows; the physical meanings of the underlying state blocks
remain only partially resolved.

The captured runtime monitor separately routes opcode F5 through the
four-byte dispatch entry at `RAM:ff7dd4` (`00 ff 2a 3e`) to
`HandleMonitorReadMemoryF5Command` at `RAM:ff2a3e`. Its mode-zero path reads
CDB[3] at `RAM:000f05`, source CDB[4..7] at `RAM:000f06`, and length
CDB[8..9] at `RAM:000f0a`; the DATA IN worker reads the CPU source at
`RAM:ff32ed` and `RAM:ff32fa`. This confirms the read-only command contract
exists in both main and monitor modes. The same captured dispatch table has
opcode `0x3b` at `RAM:ff7aec` (`00 ff 21 0d`) and opcode `0x3c` at
`RAM:ff7af0` (`00 ff 25 ae`), entering
`HandleMonitorWriteBufferCommand` at `RAM:ff210d` and
`HandleMonitorReadBufferCommand` at `RAM:ff25ae`. The monitor WRITE BUFFER
mode-4 branch at `RAM:ff23cd` calls the DATA OUT transfer worker
`RAM:ff28a6` at `RAM:ff2427`; the mode-5 branch at `RAM:ff243c` can also
call it at `RAM:ff249f` for a nonzero payload before validation. Both
mode dispatches and transfer calls are checked as canonical control-flow
edges. In that mode-5 branch, checksum calls at `RAM:ff2508`/`RAM:ff2526` precede clears of
`BITS:0580`/`BITS:0581` at `RAM:ff2542..ff2544`; the two branches then call
the runtime `RAM:ff4000` programmer with operation 3 at `RAM:ff2554` or
operation 5 at `RAM:ff2566`. These are direct canonical instruction and
flow checks in `tools/ghidra_scripts/ValidatePlextorCanonicalProject.java`;
the full read-only listing is `out/agent-tests/monitor_buffer_commands_headless.log`.
This establishes a static monitor DATA OUT staging and programming path
that could reprogram an invalid main image. It does not prove a live recovery-mode host transfer,
valid staging, flash chip effects, or successful reflashing. Exact read-only
CDBs and immutable response-byte checks are in `docs/HARDWARE_PLATFORM.md`.

F5 mode `0x0a` is a separate constructed-response path, not a direct memory
window. `BuildVendorF5MediaStrategyResponse` at `RAM:fbeb4c` clears and builds
an 82-byte response from current media-strategy selection, fixed or
recordable-media controller-profile values, write-speed capability flags, and
three 32-bit state values, then clamps the transfer to the CDB allocation
length. It has a no-argument `void` contract and terminates with `ERET`.

## PLEXTORPREMIUM on-disc update acquisition

The main firmware also contains an on-disc update path. Firmware-1.04
`ValidatePlextorPremiumDiscSignatureSector` at `RAM:fd6936` requires the
literal fourteen-byte `PLEXTORPREMIUM` marker, verifies the 1,024-word additive
sector checksum, and requires the adjacent sixteen bytes to match the runtime
INQUIRY product-identification field at `RAM:00dcd5`. The earlier
"erased marker" interpretation was incorrect.

After that check succeeds, `AcquirePlextorPremiumFirmwarePayloadFromDisc` at
`RAM:fd6a30` uses `AcquirePlextorPremiumRawSectorBatch` at `RAM:fc691d` to
request 256 raw sectors beginning at internal frame value `0x17`. The batch
helper applies the firmware's normal plus-150 acquisition conversion. The
payload routine stages the `0x100 * 0x930 = 0x93000` raw bytes, then performs
256 copies of `0x400` words: the first `0x800` bytes of each 2352-byte sector
become the contiguous 512-KiB candidate image at `RAM:100000..17ffff`.

This acquisition routine returns Boolean success but does not itself validate
the image revision/checksum or program flash. Its caller performs those later
steps through the same staged-image and runtime programming machinery
described below.

## Staged-image validation

The named validation path consists of:

- `ValidateStagedFirmwareChecksum` at `RAM:fd2c93`: start offset in `DR0`,
  exclusive checksum-word offset in `DR4`, Boolean result in `ACC`;
- `ValidateStagedMainFirmwareHeader` at `RAM:fd2d40`: compares staged offset
  `0x30` with the installed `PLEXTOR MONT    000` identity and performs the
  firmware's weak initial-dword/24-bit check;
- `ValidateStagedFirmwareRevisionPolicy` at `RAM:fd3173`: buffer ID in `ACC`,
  Boolean result in `ACC`; reads configuration offsets `0x3a/0x3b` and parses
  staged decimal revision digits without validating that they are digits;
- `StagePrimaryDriveConfigurationForAuxiliaryUpdate` at `RAM:fd68ba`: copies
  0x60 bytes of the current primary configuration into auxiliary staging.

The 1.04 `CopyFromControllerBufferWindow` call in the revision policy takes destination `RAM:000b2a` in DR0 and controller offset zero in DR4; its first source read is `RAM:100000`. The reconstructed C signature/callers were corrected to that native register order. The unmodified S251 run matches both images through the first read but stops at that high-bank access. With an explicit debugger-only relocation of the copy helper’s `0x10` bank add to `0x01` in both images and staged ASCII `00` at `X:001a..001b`, the reproducible offline continuation completes the 48-byte copy and reaches `SetSenseData(4, 0x40f5)` on both sides (`tests/simulator/trace_mode5_revision_rejection.py`, `out/agent-tests/m5-revision-rejection-final-xvfb/results.json`). This checks the revision-rejection selection under that simulator memory model. A continuation verifies the written `70/04/40/F5` sense and command idle in both images (`tests/simulator/trace_mode5_rejection_completion.py`, `out/agent-tests/m5-rejection-completion-final-xvfb/results.json`). A separate staged revision `09` with declared media state 1 reaches the main-header validator and rejects a zero first staged identity byte against installed `P`, then writes the same sense and returns to idle (`tests/simulator/trace_mode5_header_rejection.py`, `out/agent-tests/m5-header-rejection-final-xvfb/results.json`). Physical controller-window access, main-image checksum rejection, and programming remain unverified. On the current image SHA-256 `8af8470e99fce51f27347f81ecb6d5d0eb30c317b701e656ebbdc712974a5a0c`, a versioned auxiliary mode-5 zero-length case reaches the first yield, and paired continuations reach the checksum function, `SetSenseData(4, 0x40f6)`, written `70/04/40/F6` sense, and command idle (`out/agent-tests/i24checksumfix/results.json`, `out/agent-tests/m5auxentry_final/results.json`, `out/agent-tests/m5auxreject_final/results.json`, `out/agent-tests/m5auxidle_final/results.json`). The trace declares media state, minimum revision, and XDATA staging bytes. The native checksum call takes start in DR0 and end in DR4. The previous C251 formal order emitted the arguments in reverse; `ValidateStagedFirmwareChecksum(end_offset, start_offset)` with inverted direct-call arguments and target binding now emits DR0=0 and DR4=`0xfffe`, matching stock at the auxiliary entry. The S251 observation scratch shares `X:6000..7fff` with the auxiliary checksum candidate, so the rejected path is conditional evidence and that original harness layout cannot provide a clean accepted-image control (`out/agent-tests/m5auxview_final/results.json`). A case-local run relocates trace scratch to `X:a000`, `X:d000`, and `X:e000..efff`, outside the auxiliary checksum range. The runner otherwise rewrites this function's bank-`0x10` reads to bank `0x00`; explicitly selecting simulator bank `0x01` for the three checksum reads on both images makes their native reads see the identical declared candidate. Both accept its additive checksum `0x0001` and enter `StagePrimaryDriveConfigurationForAuxiliaryUpdate` with `ACC=1` (`out/agent-tests/m5auxcleanscratch_final/results.json`, `out/agent-tests/m5auxcleanaccept_final/results.json`). With the 96-byte primary configuration from the immutable runtime capture declared at `RAM:ffc000` and the same bank substitution for the staging write, both copy all 96 bytes to `RAM:10c000` (87 bytes change from the pre-copy snapshot) and reach `RAM:ff4000` with the operation-5 register tuple before executing the loader (`out/agent-tests/m5auxcleanhandoff_final2/results.json`). These are conditional S251 results; physical controller staging, flash effects, and activation remain unverified.

On the current image, a paired main-image mode-5 continuation seeds the stock identity after the passing revision policy and reaches `ValidateStagedFirmwareChecksum` with DR0=0, DR4=`0x79ffe` in both stock and reconstruction (`out/agent-tests/m5mainchecksum_entry_probe/results.json`). The bounded continuation reaches source cursor `RAM:020000` on both sides (stock DR4, reconstruction DR0), where S251 halts before a checksum decision; reconstructed PC reporting varies at that invalid-bank stop (`out/agent-tests/m5mainchecksum_boundary_final3/results.json`). Thus main-image checksum rejection and accepted programming are still unverified. A read-only canonical disassembly of the stock `ValidateStagedFirmwareChecksum` body at `RAM:fd2c93..fd2d3f` confirms the current C loop bounds and source pointers: main-image sum reads `RAM:100000..16ffff` then `RAM:178000..179ffd` and compares the big-endian word at `RAM:179ffe`; auxiliary sum reads `RAM:100000..107fff` then `RAM:10ff00..10fffd` and compares at `RAM:10fffe`. The first loops use start offset zero at the caller, and the sum is 16-bit. The listing is `out/agent-tests/staged_checksum_headless.log`. This static comparison does not establish physical staging or the S251 bank reads, and it revealed no source correction. A focused host regression now fills both complete stock-defined checksum ranges with distinct patterns, checks the independent 16-bit expected sum and total read counts (`0x72000` main, `0x8100` auxiliary including the stored word), and verifies rejection after changing a covered byte. `ctest -R ^staged_checksum_tests$` passes in `out/agent-tests/simulator-no-ghidra`; this validates full-range C behavior, not the controller bank mapping or a stock dynamic decision.

Main-image checksum validation covers staging `0x100000..0x16ffff` plus
`0x178000..0x179ffd`, with the stored checksum word at `0x179ffe`.
Auxiliary validation covers `0x100000..0x107fff` plus
`0x10ff00..0x10fffd`, with its stored word at `0x10fffe`.

Revision-policy failure reports sense `04/40/F5`; checksum failure reports
`04/40/F6`. A nonzero return from the runtime entry has an `04/40/F3` path,
although successful programming operations 3 and 5 normally do not return.

## Runtime image and interrupt ownership

The immutable 64-KiB capture `data/read_memory_ff0000.debug` has SHA-256
`7ad6513ecb74f6107b3abdbdbb59b29b00334adb79cbf48ba1322d0442a42e80`
and maps at `RAM:ff0000..ffffff`. Its separate canonical program is
`/validated/read_memory_ff0000.debug` in `ghidra/gplextor`.

The ROM owns the physical interrupt vectors even while main firmware runs.
Five fixed routers preserve `ACC`, inspect
`MainFirmwareInterruptVectorSelect` at `RAM:0000fc`, and tail-jump to either a
monitor or main-firmware handler:

| Runtime router | Entry |
|---|---:|
| `RouteControllerEventInterrupt` at `RAM:ff3f84` | controller event |
| `RouteHostInterfaceInterrupt` at `RAM:ff3f9b` | host interface |
| `RouteControllerPacketInterrupt` at `RAM:ff3fb2` | controller packet |
| `RouteSfrE1Bit7Interrupt` at `RAM:ff3fc9` | neutral SFR-E1 bit-7 source |
| `RouteTimer0Interrupt` at `RAM:ff3fe0` | Timer 0 |

`SelectMonitorInterruptHandlers` at `RAM:ff4dc3` clears the selector; main
firmware sets it after installing its runtime contexts. Preserve this ROM/main
ownership boundary in separate reconstruction targets.

## Runtime loaders and relocated code

The downloadable image contains erased bytes at these targets. The runtime
capture supplies their implementations:

| Entry | Loader behavior | Role |
|---|---|---|
| `RAM:ff2000` | direct runtime routine | hardware/register initialization; not a memory reader or flash writer |
| `RAM:ff4000` | copy `0x40e` bytes from `RAM:ff40ed` to `RAM:000800`, then jump | main/auxiliary flash programmer loader |
| `RAM:ff4500` | copy `0x28e` bytes from `RAM:ff3c8a` to `RAM:000800`, then jump | configuration flash programmer loader |
| `RAM:ff5000` | copy `0x6a` bytes from `RAM:ff5a5d` to `RAM:000800`, then jump | flash-identification loader |

Before the main or auxiliary WRITE BUFFER programmer handoff, stock
`HandleWriteBufferCommand` clears `BITS:0580` and `BITS:0581` at
`RAM:fd2b79..fd2b7b`, then invokes runtime `RAM:ff4000` with operation
3 or 5 (`out/agent-tests/write_buffer_p3_headless.log`). The C path
performs the same two bit clears. Physical P3 wiring is unknown, so a
normal-command F5 data-read alias cannot alone validate the update-state
flash mapping.

The relocated `RAM:000800..000c0d` main programmer is exactly `0x40e` bytes
and directly emits unlock, erase, byte-program, and polling sequences. The
canonical runtime program represents it as one contiguous function, including
the erase sequence at `RAM:0008c7..0008f8` and epilogue at
`RAM:000bfd..000c0d`; the canonical project validator enforces that complete
body.

The flash-identification routine recognizes device codes `0xbf00`, `0x6200`,
or a normalized `0x2000` result. The relocated programmer branches on the
stored result at `RAM:0000f8`: its `0x2000` path starts at `RAM:000824` and
emits `0x3000` erase stores; another path starts at `RAM:0008c7`, includes a
`0x5000` store, and later emits `0x3000` stores. These values and address
cadences correspond to the [SST39LF400A manufacturer's sector/block erase
commands](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST39LFX00A-SST39VFX00A-2-Mbit-4-Mbit-8-Mbit-x16-Multi-Purpose-Flash-DS20005001.pdf).
On 2026-09-25, an identified live PX-W5224A 1.04 returned `bf00` from
`RAM:0000f8` through the firmware's read-only F5 DATA-IN path, so this drive
selects the alternate branch. Read-only canonical runtime disassembly verifies the relocated
loader’s device-word read at `flash_identification_relocated::000833`, store to
`RAM:0000f8` at `::000836`, and normalization tests/stores at
`::00084b..000863` (`out/agent-tests/flash_id_selector_headless.log`). In particular, the `0x2000` branch's early `0x30` stores advance by
64 KiB across `RAM:f00000..f60000`; that cadence alone does not cover every
4-KiB sector of the inspected SST part. It would be unsound to treat the
isolated `0x2000` probe as the physical board's complete erase algorithm. In an isolated offline S251 operation-5 probe with a declared
`0x2000` result, both 1.04 images copy the `0x40e`-byte programmer to
`RAM:000800` and stop at `RAM:00082b`, the successor of the first unlock store
(`RAM:000828`, `DR16=0xf0aaaa`, `WR24=0xaa00`), after 12,166 states. An
initial single-run `RAM:0008b1` checkpoint also stops at `RAM:00082b` because
S251 pauses after a mapped flash-window store. Controlled resumes now record
14 matching CPU stores through post-store `RAM:0008c5` on the current image;
the retained result is
`out/agent-tests/integration-gates/operation5-2000-cpu-store-ring-order-pc-pair/results.json`.
A second bounded case declares `RAM:0000f8=0xbf00` and reaches the alternate
branch's first unlock-store successor at `RAM:0008d6` in both images after
12,176 states. Controlled resumes record six matching stores through
`RAM:0008f9` at
`out/agent-tests/integration-gates/operation5-bf00-cpu-store-ring-order-pc-pair/results.json`.
The declared result now matches the `bf00` value measured on the connected
drive. These are still CPU stores, not observed
flash-chip effects; neither branch's later programming or poll outcome is
validated.

The SST39LF400A manufacturer-ID word is `0x00bf`, whereas the live CPU-visible
selector is `0xbf00`. That exact byte swap agrees with the programmer's CPU
words `0xaa00`, `0x5500`, `0x8000`, and `0x5000`: their high byte becomes the
SST command byte `DQ7..DQ0`. The CPU byte-address offsets `0xaaaa` and `0x5554`
also divide by two to the manufacturer's x16 unlock word addresses `0x5555`
and `0x2aaa`. Together with the complete live `RAM:f00000..f7ffff` /
`RAM:f80000..ffffff` read alias, this is strong independent evidence for the
stock programmer's address and command-byte mapping. It still does not claim
that an erase or program cycle has been physically observed.

## Programming maps

Runtime operation 3 programs:

```text
staging 0x100000..0x16ffff -> programming 0xf00000..0xf6ffff
staging 0x178000..0x179fff -> programming 0xf78000..0xf79fff
```

Runtime operation 5 programs:

```text
staging 0x100000..0x107fff -> programming 0xf70000..0xf77fff
staging 0x10c000..0x10ffff -> programming 0xf7c000..0xf7ffff
```

The retained stock full-image updater does **not** select this internal
operation 5. Its final SCSI WRITE BUFFER mode-5 command uses buffer ID zero,
which the firmware finalizes through internal operation 3. Buffer ID `0xff`
selects the separate auxiliary internal operation 5. Consequently the
operation-5 first-erase/range concern below is relevant to auxiliary updates,
not to the 31-mode-4-plus-one-mode-5 full-image plan prepared for the current
reconstruction.

The operation-5 byte-program loop skips `RAM:f78000..f7bfff`, but the
preceding erase sequence does not skip the entire region. In the captured
runtime monitor's relocated programmer, `monitor_service_relocated::0008a9`
sets `DR28` to programming address `RAM:f78000`, and the instruction at
`monitor_service_relocated::0008b1` writes the erase-sector value `0x3000`
there on the `0x2000` device-code path for both operations 3 and 5. This follows the
`0xaa00`, `0x5500`, `0x8000`, `0xaa00`, `0x5500` unlock/erase setup at
`monitor_service_relocated::000824..000847`. Therefore the programming map
alone does not establish preservation of `RAM:f78000..f7bfff` during operation
5. The inspected SST39LF400A has uniform 4-KiB erase sectors and 64-KiB erase
blocks, according to its [manufacturer datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST39LFX00A-SST39VFX00A-2-Mbit-4-Mbit-8-Mbit-x16-Multi-Purpose-Flash-DS20005001.pdf).
If this `0x30` write reaches that chip and programming address `RAM:f78000`
aliases execution address `RAM:ff8000`, it would erase
`RAM:ff8000..ff8fff`. A read-only calculation on the immutable 1.04 image
finds 3,730 non-`0xff` bytes in that 4-KiB sector; changing those bytes to
`0xff` would change the computed main checksum from `0xb3bd` to `0x8ef0`
while leaving its stored word at `RAM:ff9ffe` equal to `0xb3bd`. For an earlier reconstructed image (SHA-256
`8178fc3c8f3189c9665fed5bfa33e234b3d56ec6da27715bbbada83bea23760a`),
the analogous calculation changes 3,730 bytes and the computed checksum
from `0x782d` to `0x5360`, while the stored word remains `0x782d`. These are conditional image calculations, not observed flash
outcomes. They make the physical branch selected at `RAM:0000f8`, the
LC898095 address/byte-lane mapping, and any flash protection decisive open
questions before operation 5 can be accepted as safe.

The main-firmware function at `RAM:fd68ba` copies 0x60 bytes from the
captured runtime monitor at `RAM:ffc000` into auxiliary staging at
`RAM:10c000`. Operation 5's byte-program loop then targets programming
address `RAM:f7c000` for that staging region. This is the configuration copy
that needs an explicit before/after preservation assertion; the earlier claim
that the configuration copy resided at `RAM:f78000` was incorrect.

The exact agreement between these maps, the checksum layouts, and the normal
executable mapping suggested that programming addresses
`0xf00000..0xf7ffff` alias read/execution addresses `0xf80000..0xffffff`.
Read-only F5 observations on an identified live 1.04 drive now confirm the
normal-command CPU data-read relationship at representative pairs
`RAM:f00050`/`RAM:f80050`, `RAM:f70000`/`RAM:ff0000`,
`RAM:f72a3e`/`RAM:ff2a3e`, and `RAM:f78000`/`RAM:ff8000`; returned bytes match
the immutable stock image and runtime-monitor capture. Full-window captures
then proved `RAM:f00000..f7ffff` byte-identical to
`RAM:f80000..ffffff` (SHA-256
`c3f6dff082319e1e3ac29554c3954be4a9a05d6b2345a5114fee8a65c9f5a65c`),
with the high view exactly equal to stock through `RAM:feffff` plus the
protected runtime capture at `RAM:ff0000..ffffff`
(`out/agent-tests/windows-readonly-w5224a-full-backup-20260925/`).
The main WRITE BUFFER path clears `BITS:0580/0581`, and the configuration
programmer uses a different P3 transition. Their physical pin roles remain
undocumented. For the factory full-image path, however, those writes and the
subsequent operation-3 handoff are the stock implementation being reused; the
live ID byte order and exact SST command addresses above close the practical
address/lane inference. Only the actual chip effect remains unobserved before
the first authorized update.

## Activation and recovery handoff

Successful operation 3 or 5 writes `0x99` to `RAM:0000fd` and jumps to
`RAM:ff0200`. The captured relocated programmer does this at
`monitor_service_relocated::000bf2..000bf9`; the bootstrap calls the
forced-recovery predicate at `RAM:ff021e` before main-image handoff. The
read-only canonical validator checks those exact instructions and flow
(`out/agent-tests/reset_handoff_validation.log`). Bootstrap reinitializes
hardware, evaluates forced recovery, validates the installed main image, and
transfers to either main entry `RAM:f80050` or monitor loop `RAM:ff0300`.

Mode-5 WRITE BUFFER is therefore self-activating. The firmware trace does not
support START STOP UNIT as the activation mechanism for this update path.

The current-image 34-case monitor suite includes both conditional first-erase interruption decisions. With either `RAM:f80000..f80fff` or `RAM:f80000..f8ffff` filled with `0xff` and the stored checksum left unchanged, stock and reconstruction enter recovery loop `RAM:ff0300` with `ACC=0` (`out/agent-tests/m34blockfull/results.json`). The 4-KiB and 64-KiB spans correspond to the two declared operation-5 device-code branches under the explicit SST command-lane and `+0x80000` alias assumptions. No physical erase or reflashing was observed.

The monitor protects against an invalid installed checksum by refusing to boot
the main image. A checksum-valid but semantically broken main image instead
requires forced recovery. The user reports that the `RAM:ff0000` rescue
firmware is protected and never flashed, and that holding Eject during
power-on enters it. Therefore the inferred high-address operation-5 writes
cannot be equated with destruction of the rescue firmware: the runtime rescue
view and flash-backed main-image view must be distinguished. In the immutable
1.04 download, the entire `RAM:ff0000..ff7fff` recovery hole is `0xff`, while
the runtime capture contains 28,717 non-`0xff` bytes there. The next
`RAM:ff8000..ff9fff` 8,192 bytes match the download exactly. These are
file-level observations consistent with the protected rescue code premise;
they do not resolve the flash bus alias or which main-image cells are written.

`tools/probe_readonly_w5224a.py --recovery` provides a non-writing recovery
preflight. It requires the captured monitor's `PLEXTOR` / `W5224A` / `0.00`
INQUIRY identity before issuing any fixed vendor F5 DATA-IN request. Against
the normal 1.04 drive, an elevated Windows run issued only INQUIRY, received
GOOD status and the expected 1.04 identity, then rejected the mode mismatch
before a vendor command. The user subsequently entered recovery by holding
Eject at power-on. The positive run identified revision `0.00`, completed all
41 DATA-IN commands, passed all 15 required checks, and captured two identical
512-KiB views whose bytes equal stock main firmware plus the protected monitor
capture (`out/agent-tests/windows-recovery-full-backup-current-20260925/`).
The tool itself did not reset or alter the drive. This confirms recovery entry
and read access, not WRITE BUFFER transfer or flash effects.

The retained 1.04 updater configuration `data/W5224A104/PlexFirm.ini`
(SHA-256 `0824164c8eb37838ae2bb2c6ee260350ffcb9bcd8a3e58e6cac328b45f6a6e4f`)
selects `MONT_104.bin`, declares a 524,288-byte image and 16,384-byte
buffer, and enables `Use Update After Cmd`. Those settings do not document
a forced-recovery procedure or demonstrate that the updater can communicate
with the monitor after an interrupted flash operation. The Eject-at-power-on
procedure is supplied by the user. The captured monitor WRITE BUFFER route
supplies static command-path evidence; live monitor transfer is outside the
current recovery-entry gate.

No reconstructed or patched image should be flashed merely because its
checksum matches. For a hardware write, retain the original image and the
user-provided Eject-at-power-on rescue procedure, resolve the intended flash
write range,
and obtain explicit authorization for the exact experiment. The reset-to-rescue
selection has passing offline cases; a live monitor reflashing test is not a
precondition for the current reconstruction gate.

`tools/flash_w5224a.py` is the non-GUI Windows execution boundary for that exact
experiment. Its default mode only regenerates and matches the normal/recovery-
bound plan. Live mode additionally requires the full candidate SHA-256,
re-identifies the revision-`0.00` monitor, logs each command before submission,
and never retries the self-activating final command. `tools/verify_w5224a_flash.py`
is the corresponding DATA-IN-only post-flash verifier. It expects candidate
bytes in the two internal-operation-3 programmed ranges, the protected monitor
in image offsets `0x70000..0x77fff`, and the captured configuration/runtime tail
at `0x7a000..0x7ffff`. Both tools have passed validation-only runs for current
candidate SHA-256 `f374052001e719e619fb0f35204d51dfc2bda5c60bc4c471e796278c2afba5f6`;
neither validation run opened the drive.
