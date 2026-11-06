# Firmware source readability baseline

This document records the validation boundary for the repository-wide source
readability refactor performed on 2026-09-20.

## Pre-refactor baseline

- All 270 registered host and firmware-format tests passed.
- The most recent verified Keil C251/A251 O5/SIZE build linked 446 unique
  objects with no missing objects or unresolved symbols.
- Its linker totals were `xdata=6916`, `const=18510`, and `code=439180`.
- The reconstructed 512-KiB image had checksum `0x6d86` and SHA-256
  `563ee1634a7a061e070de5305c508de384355257d0219e8b56b13d7a4db45ad8`.

The pre-refactor tree was subsequently rebuilt from Git `HEAD` with the local
Wine/Keil installation. That clean reference build reproduced the recorded
sizes, checksum, and image SHA-256 above, providing a fresh comparison set of
Keil listings, relocations, linker output, and reconstructed-image bytes.

## Post-refactor validation

- All 270 tests pass, including both exhaustive reconstruction aggregates.
- `check-format` passes with the repository `.clang-format`.
- Every C translation unit preprocesses through its target branch in the
  portable preprocessing check, which supplies an empty stand-in for the
  Keil-specific `intrins.h` header.
- The SCSI dispatch-map verifier passes against firmware 1.04.
- Domain `.c` files contain no `PLEXTOR_HOST_TEST` branches or `Observe...`
  declarations/calls.
- Direct generic firmware-memory calls in domain `.c` files use named map
  operands rather than numeric address operands.
- Near-memory helpers, direct SFR operations, and the identified bit-address
  bindings in domain `.c` files likewise use their qualified map operands.
- Tracked C and header files contain no CRLF line endings.
- `git diff --check` passes.

## Target-equivalence result

- Keil C251 5.60, A251 4.69.13, L251 4.66.93, and OH251 1.47 run through
  Wine from the local Keil installation.
- The refactored O5/SIZE build compiles and links all 446 objects with zero
  compiler, assembler, and linker diagnostics.
- All 446 normalized assembly listings match the rebuilt pre-refactor
  reference. The comparison retains emitted instruction/data bytes and
  relocation targets while ignoring source paths, timestamps, source-line
  metadata, and local-label numbering.
- Linker totals and placement match: `xdata=6916`, `const=18510`, and
  `code=439180`, with the same ECODE, HCONST, and CODE ranges.
- The reconstructed image is byte-identical to the pre-refactor baseline:
  checksum `0x6d86` and SHA-256
  `563ee1634a7a061e070de5305c508de384355257d0219e8b56b13d7a4db45ad8`.

Object containers are deliberately not compared literally because formatting
changes their source paths and line metadata even when emitted bytes and
relocations are identical.
