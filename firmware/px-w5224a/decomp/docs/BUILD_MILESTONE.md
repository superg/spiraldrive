# Reconstructed main-firmware build status

## Source of truth

The reconstruction target is firmware 1.04, immutable image
`data/W5224A104/MONT_104.bin`, analyzed as `/validated/MONT_104.bin` in the
`ghidra/gplextor` project with `80251-fixed:BE:24:default` and source mode
enabled. The separate `/validated/read_memory_ff0000.debug` program supplies
runtime-monitor interface evidence; its code is not linked into the main image.

All 1,717 canonical main-image functions have accepted C implementations. This
is behavioral source coverage, not a claim of byte-identical output or safe
hardware operation.

## Entry and link model

- `src/boot/reset_vector.c` implements the canonical reset entry at
  `RAM:f80050` and transfers to `FirmwareMainLoop`.
- `src/platform/coroutine_initialization.c` implements `FirmwareMainLoop` at
  canonical `RAM:fdcea9`. L251 places its emitted module segment
  `?PR?COROUTINE_INITIALIZATION` at that address.
- `src/platform/firmware_entry.c` owns stack-reset continuation, hardware and
  drive initialization, restart, interrupt enabling, and entry into the main
  loop.
- The controller, host, packet, timer-zero, and SFR-E1 interrupt entries are C
  translation units under `src/boot/`.
- Monitor services at `RAM:ff2000`, `RAM:ff4500`, and `RAM:ff5000` remain
  external runtime boundaries.

The retired disabled-idle prototype, its fail-stop entry replacements, and its
bounded startup interpreter are not part of the current build.

## Toolchain and packaging

The selected reconstruction toolchain is Keil C251 5.60, A251 4.69, L251 4.66,
and OH251 1.47. CMake invokes PE-format tools through Wine on Linux and directly
on Windows. Tool binaries and libraries are not committed.

After a successful locator pass, OH251 emits `main.hex` and
`tools/firmware_image.py` packages it as `MONT_reconstructed.bin`. Packaging
checks the 512-KiB container layout, immutable regions, monitor entry window,
and checksum. It does not certify instruction equivalence, fixed-address
correctness, peripheral behavior, recovery behavior, or flash safety.
The same firmware target emits `firmware-update-plan.json` through
`tools/firmware_update_plan.py`. That file validates the image and records the
hash-pinned stock updater's 32 WRITE BUFFER CDBs, but the tool has no device or
DATA OUT backend and cannot flash a drive.
For a future hardware-approved plan, it can bind both the existing normal-mode
full backup and a still-pending, passing recovery-mode `0.00` full backup; the
latter must come from the current DATA-IN-only helper.

## Current status and blockers

The project is not hardware-validated or safe to flash. The generated manifest
links all 446 C251/A251 objects exactly once. L251 selects the standard C251 libraries itself; project
and runtime unresolved symbols are both zero. Six four-byte A251 veneers own
the canonical reset/interrupt addresses. An expanded diagnostic layout links
and converts the entire program with zero warnings and zero errors; the former
1,189 improper fixups were all cross-bank `LCALL` uses and disappeared after
relocatable reconstruction functions adopted 24-bit calls.

The current native-Windows physical layout links at uniform O5/SIZE with
`code=437779`, `const=18510`, and `xdata=6824`. Code plus constants occupy
456,289 of the 458,672-byte main payload.
The physical link, absolute-DATA audit, OH251 conversion, packaging, and image
verification all pass. The resulting 512-KiB image has SHA-256
`4fa274513c60ea14bf9aefbb918a75103a88090b5c1d73042e61ecb998783f3f`
and checksum `0xa1da`. A clean second Windows build produced byte-identical
BIN and HEX payloads; ABS and map containers retain build timestamps.

Internal original-ROM references are linker-resolved; packer-owned fixed data
and fixed flash/monitor references are explicit contracts. The remaining
blocker is operational evidence. Native S251 smoke, all 24 integrated cases,
and all 39 monitor cases pass on the current linked image; the refreshed
1,717-case direct catalog is in progress. A read-only live-drive backup proves
the complete normal-mode `RAM:f0..f7` to `RAM:f8..ff` data alias and preserves
the protected monitor capture. Programming-state mapping, physical interrupt/
media behavior, erase/program effects, activation, and recovery-mode transfer
still require staged hardware evidence and explicit authorization before any
flash operation.
