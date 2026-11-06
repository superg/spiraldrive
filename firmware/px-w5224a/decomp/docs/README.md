# Reconstruction documentation

This directory contains the cross-program specifications and workflow needed
to reconstruct PX-W5224A firmware 1.04 as compilable C/C++ and generate
alternative images. It is not a second analysis database.

The canonical program-level reverse-engineering state is stored in project
`ghidra/gplextor`, programs `/validated/MONT_104.bin` and
`/validated/read_memory_ff0000.debug`. Function boundaries, symbols, types,
signatures, comments, references, bookmarks, and translation-unit ownership
must be read from and updated in Ghidra. Documentation here records image/build
rules, cross-program conclusions, reconstruction policy, and unresolved
hardware questions.

## Start here

1. [C_RECONSTRUCTION_STATUS.md](C_RECONSTRUCTION_STATUS.md) — preparation gates and the decompilation/C/C++
   acceptance workflow.
2. [COMPILER_IDENTIFICATION.md](COMPILER_IDENTIFICATION.md) — original-toolchain evidence and the
   exact-byte compiler experiment.
3. [C_TRANSLATION_UNITS.md](C_TRANSLATION_UNITS.md) — intended source-tree partition and translation
   order.
4. [W5224A_FIRMWARE_FORMAT.md](W5224A_FIRMWARE_FORMAT.md) — image layout, checksum, and generator
   requirements.
5. [FIRMWARE_MEMORY_LAYOUT.md](FIRMWARE_MEMORY_LAYOUT.md) — explicit linker, packer, recovery-hole,
   and volatile-memory ownership.
6. [HARDWARE_PLATFORM.md](HARDWARE_PLATFORM.md) — confirmed LC898095 platform facts and neutral
   MMIO/SFR policy.

## Subsystem references

- [D8_LEADIN_ANALYSIS.md](D8_LEADIN_ANALYSIS.md) — negative-D8 acquisition, lead-in selection, and
  C2 cursor behavior.
- [FIRMWARE_UPDATE_ANALYSIS.md](FIRMWARE_UPDATE_ANALYSIS.md) — READ/WRITE BUFFER, staged-image validation,
  runtime interrupt ownership, flash programming maps, activation, and recovery.

## Generated views

Generated CSV, pseudocode, call-graph, and inventory views are not retained in
this repository. Export them from the canonical programs into an explicit
temporary or build directory and discard or regenerate them as needed.

`agent-memory/` is maintained separately for agent continuity and is not part
of the technical handoff set described here.
