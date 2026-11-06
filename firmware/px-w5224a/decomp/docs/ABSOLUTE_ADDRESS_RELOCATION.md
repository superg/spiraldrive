# Absolute-address ownership and relocation

The reconstruction must not treat a successful link as proof that every
numeric address remains valid. Numeric constants do not create linker
relocations, so a stale reference to original ROM can survive compilation and
linking without a diagnostic.

The canonical address, object-boundary, type, and function evidence remains in
Ghidra program `/validated/MONT_104.bin`. The source audit is reproducible with
`tools/audit_absolute_addresses.py`; its JSON, CSV, address list, Ghidra TSV,
and generated Markdown views belong under `out/agent-tests/` and are not a
second analysis authority.

## Address policy

| Source address class | Reconstruction policy |
|---|---|
| SFR, bit, controller MMIO, flash alias | Keep fixed and volatile; verify the hardware contract |
| Shared runtime RAM and controller buffers | Keep fixed initially; use named typed accessors and reserve ownership |
| Runtime monitor entry | Keep fixed as an external overlay ABI and verify the exact entry |
| Internal ROM data | Emit an owned data object and refer to its symbol |
| Function or coroutine entry | Use a linker-resolved function symbol |
| Pointer-bearing dispatch table | Rebuild with relocations or replace with direct symbolic dispatch |
| Code bytes read as data | Replace with an explicit constant object or prove fixed code placement |
| Checksum, erased bytes, pointer sentinels | Treat as container fields/constants, not ordinary objects |

Canonical `RAM:` address comments remain source identity metadata. They do not
require reconstructed functions to retain their original addresses.

## Reproducible baseline

The current source census contains 24,343 address candidates at 2,748 unique
RAM addresses across C, headers, and retained assembly. Most are fixed runtime
RAM/MMIO contracts. The relocation-first
subset originally contained 576 occurrences at 292 unique addresses in main
ROM, fixed image data, flash aliases, or the runtime-monitor overlay.

Correlation with the canonical Ghidra map classified that initial subset as:

| Canonical owner | Unique addresses |
|---|---:|
| Defined data | 262 |
| Exact function entries | 25 |
| Function interiors read as data | 3 |
| Undefined at the literal address | 2 |

The 25 exact function-entry dependencies have been converted to
linker-resolved symbols through `src/platform/firmware_code_addresses.h`.
Host tests retain the original numeric values for observable ABI assertions;
target C251 builds emit `WORD0`/`WORD2` relocations for far functions and
`WORD0`/`?C?CODESEG` relocations for near functions. Representative C251 O5/SIZE
builds of `coroutine_initialization.c`, `raw_read_queue_boundaries.c`,
`indexed_drive_state_transitions.c`, and `status_mapping.c` complete with zero
diagnostics and show those relocations in their listings.

After that conversion, 418 priority occurrences at 267 unique addresses
remained. The first pointer-table pass then replaced the SCSI dispatch table
dependency at `RAM:f83ef7` and the unsupported-handler pointer comparison at
`RAM:ff9691` with `src/scsi/command_dispatch_map.inc`. The map contains the 78
supported opcodes and routes the other 178 opcodes to
`HandleUnsupportedScsiCommand`. `tools/verify_scsi_dispatch_map.py` checks
every opcode and handler address against the immutable 1.04 image. The same
verified source model now replaces the 256-byte media-requirement table at
`RAM:f842f7` with 50 symbolic opcode cases. The C251 O5/SIZE dispatcher
compiles with zero diagnostics.

The remaining ROM dependencies have now been converted as follows:

| Disposition | Source occurrences |
|---|---:|
| Linker-resolved ROM-data symbols | 410 |
| Linker-resolved code symbols | 25 |
| Verified fixed configuration-flash aliases | 34 |
| Verified fixed monitor/overlay addresses | 21 |

`tools/generate_firmware_rom_data.py` validates the immutable 1.04 SHA-256 and
emits two relocatable constant objects: the contiguous
`RAM:f82320..f848ff` main-ROM data range (9,696 bytes) and a 73-byte explicit
replacement for the three function-interior/code-as-data reads at
`RAM:f894e3..f8952b`. The 80-byte header and `RAM:ff8000..ff9ffd` fixed-data
range are packer-owned at their canonical addresses and are not duplicated in
HCONST. Target references use linker relocations or canonical fixed-data
addresses through `firmware_rom_data.h`; host references retain canonical
numeric values for tests and traceability.

The flash configuration window remains fixed through
`PLEXTOR_CONFIGURATION_FLASH_ADDRESS`. The separately mapped recovery/runtime
monitor remains fixed through `PLEXTOR_RUNTIME_MONITOR_ADDRESS` and named A251
entry constants. The only tail literals outside those wrappers are verified
24-bit masks, wrap increments, or sentinels (`0xfffffd..0xffffff`).

The current audit reports **zero unresolved priority occurrences and zero
unresolved priority addresses**. Lower addresses are deliberately retained as
runtime RAM, controller MMIO/buffers, or numeric constants; they are not links
back into the original downloadable ROM image.

A complete serial C251/A251 O5/SIZE pass compiles every translation unit with
zero diagnostics. The physical 446-object link now completes with zero
warnings and errors; its complete HCONST class uses 18,510 bytes. Runtime
libraries and integration symbols resolve, and the packed image passes the
layout and checksum validators. These results do not turn remaining fixed
addresses into relocatable objects or prove operational behavior.

## Integration consequence

The generated constants currently occupy linker-selected HCONST space, so
internal reconstructed code remains correct if it moves. That alone does not
preserve the original bytes at canonical `RAM:ff8000..ff9ffd`. Image packaging
places the required externally visible copy there from the hash-pinned 1.04
reference before calculating the new checksum. The physical locator map and
ownership decision are therefore explicit and reproducible.
