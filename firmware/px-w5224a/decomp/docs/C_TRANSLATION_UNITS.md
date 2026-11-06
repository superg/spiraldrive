# Proposed C/C++ translation-unit boundaries

This is the working source architecture for firmware 1.04. Canonical Ghidra
function tags place all 1,717 functions, but 121 placements are conservative
caller-propagated candidates and 120 compact leaves retain address-based names.
Move those functions into permanent source units only after callers, shared
state, and hardware references support the placement.

The `.c` names below are provisional. Compiler selection determines whether
the target implementation remains C or uses C++ and corresponding extensions;
the subsystem boundaries apply to either choice.

| Unit | Initial contents | Readiness |
|---|---|---|
| `platform/mcs251.h` | fixed-width types, register-storage annotations, interrupt/coroutine boundary macros | Design only |
| `platform/lc898095.h` | qualified SFR/MMIO addresses and volatile accessors; no unverified bit names | Address inventory available |
| `platform/arithmetic.c`, `platform/integer_division.c` | `MultiplyU32Low`, dual-result unsigned and signed division, minima/absolute-value helpers, four-byte bit-consensus update, and the signed-width low-byte bit-7 transforms | Dual-result division is represented by C structs whose field order preserves the native result registers. The unsigned 32-bit implementation also models both instruction-defined zero-divisor paths without invoking C division. |
| `platform/encoding.c` | big-endian 16-bit access, binary/packed-BCD conversions, ASCII-hex decode, and hexadecimal-nibble seven-segment conversion | Exact behavior recovered; the seven-segment mapper is represented by its canonical 16-byte table and a `u16` result matching the original R2:R3 pair |
| `platform/memory.c` | `ZeroMemoryWords`, byte-copy helpers, `CopySixBytesFar`, far-bank helpers, `SwapByteOrderInWords`, nonzero-count far fill, byte-stride far word-window copy, and compiler save/restore stubs | Exact instruction behavior recovered for the listed helpers; host pointers cannot reproduce far-bank wrap directly, six-byte-copy overlap is unproven, and stack-context save/restore ABIs remain unforced |
| `platform/crc.c` | `BuildCrc16TablePolynomial1021` and consumers of the 256-word RAM table | Table construction and subsystem placement are available; individual consumers still require translation review |
| `platform/controller_io.c` | controller port submissions, parameter-word/profile loading, `MapOperatingCodeToProfileIndex`, four channel-profile loaders, indexed channel reset/value helpers, fixed parameter tables, register-index/status mapping, atomic buffer access, reset register blocks, external-buffer fill engine and completion handshake | Core helper behavior and selected ABIs recovered; the operating-code map has exhaustive portable coverage, while physical code/table/field meanings remain neutral |
| `boot/image.c` | image header, checksum formula, range-erased/checksum helpers | Suitable first behavioral reconstruction |
| `boot/startup.c` | `ResetVector`, `InitializeAndEnterFirmwareMain`, `RestartFirmware`, `InitializeDriveFirmware`, subsystem startup | High-level graph recovered |
| `kernel/coroutine.c` | four context constructors, resume/yield paths, saved-stack state | Control flow and storage confirmed |
| `kernel/interrupts.c` | five vector wrappers and handlers plus the shared ROM/main vector selector at `RAM:0000fc` | Entries and ownership switch confirmed; one peripheral identity unresolved |
| `host/transport.c` | host-interface interrupt, controller data-in/data-out loops, and their status-mapping wrappers | Wrapper register ABIs and status mapping are exact; lower MMIO transfer state remains partial |
| `scsi/dispatch.c` | CDB receive, 256-entry opcode table, unsupported-command path, CDB mask validation, and fixed-format sense helpers | Dispatch complete; mask-descriptor and sense helper ABIs recovered |
| `scsi/spc.c` | inquiry and `InitializeInquiryResponseBuffer`, request sense, test unit ready, mode select/sense, start/stop, prevent/allow | INQUIRY buffer construction and handler boundaries recovered |
| `scsi/mmc_read.c` | capacity, TOC/PMA/ATIP, subchannel, READ(6/10/12), READ CD/MSF including `IsBinaryMsfTupleValid`, D8/D9 | Command and lower raw-read pipelines are bounded and substantially named; physical controller formats remain partial |
| `scsi/mmc_audio.c` | play/pause/stop and track-relative commands | Handler boundaries named |
| `scsi/mmc_write.c` | write, blank, cue sheet, OPC, reserve/repair/close track, format | Handler boundaries named |
| `scsi/vendor.c` | DE, F0–F8, E9/EB–EF, BA/CD and other shared vendor slots | Handler ownership and many response paths are recovered; several vendor field meanings remain neutral |
| `update/buffer.c` | READ/WRITE BUFFER parsing, staged image policy/header/checksum validation, and shared transfer dispatcher | Command and validation behavior recovered |
| `update/flash.c` | runtime-loader contracts, relocated main/configuration flash programmers, reset and bootstrap handoff | Programming maps recovered; flash-alias hardware mechanism remains a strong inference |
| `media/state.c` | startup/media coroutines, drive-state and indexed-state dispatch | Mechanical structure recovered |
| `media/servo.c` | seek, radial movement, speed, spindle and lead-in state machines; the `SFR:da`/`SFR:db` measurement processing, reset, strategy, formatting, and snapshot helpers | Measurement data flow is substantially recovered, but physical SFR roles, units, and two zero-direct-caller integrations remain unresolved |
| `media/decoder.c` | CIRC/CD-ROM/subcode/C2 buffer setup and batch transfer | Hardware map required |
| `config/persistent.c` | split configuration reads, primary/extended checksum calculation, programming/verification, and derived startup/geometry state | Read/checksum/programming flow recovered; most payload fields remain unnamed |
| `data/tables.c` | opcode tables, packed selector/value records, numeric and state tables | Boundaries and reviewed layouts are canonical in Ghidra; `C_RECONSTRUCTION_STATUS.md` defines the C modeling policy; remaining tables need semantic types |
| `monitor/*` | ROM recovery monitor from `RAM:ff0000` capture | Separate image/build target |

## Reconstruction order

1. Define address-qualified platform types and a simulator-backed memory map.
2. Add behavioral tests beside translated arithmetic, checksum, range,
   byte-order, BCD, and table helpers using original firmware vectors.
3. Reconstruct coroutine context setup and the SCSI dispatch layer without
   hardware side effects.
4. Introduce a mocked LC898095 interface and translate interrupt/transport
   paths one state transition at a time.
5. Translate media, servo, decoder, and write state machines only after their
   shared RAM fields and MMIO contracts are named.
6. Maintain two independent comparisons: behavioral tests and emitted-image
   comparison. A valid checksum is not an equivalence result.

A fresh decompiler corpus must be generated from the prepared 1,717-function
database at the start of translation. It is evidence, not source to copy
verbatim. Bounded fake stack arrays, inferred arguments, overlapping globals,
and generic SFR names must be resolved at each function.

The canonical main Ghidra program stores this partition directly on functions
as `unit:*` tags. `placement:reviewed` marks evidence-backed assignments;
`placement:candidate` marks conservative direct-caller propagation that must be
reviewed before permanent source placement. The tags—not a generated CSV—are
the current ownership source of truth.
