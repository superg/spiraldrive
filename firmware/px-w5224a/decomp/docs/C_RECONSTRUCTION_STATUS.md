# C/C++ reconstruction status

PX-W5224A firmware 1.04 has accepted C implementations for all 1,717 canonical
functions. Exact functions, bodies, data objects, symbols, types, signatures,
comments, and references are authoritative in Ghidra project
`ghidra/gplextor`, programs `/validated/MONT_104.bin` and
`/validated/read_memory_ff0000.debug`. Each C implementation carries a compact
machine-readable entry tag of the form `/* RAM:xxxxxx */`.

Preparation established complete code/data classification, resolved direct and
known indirect control flow, a stable processor model, function ownership, and
an empty high-risk tail among candidate and address-named functions. Ghidra
plate comments retain per-function ABI, behavior, confidence, and validation
evidence where it is useful. There is no separate reconstruction manifest or
generated progress report.

## Current source-build milestone

The maintained source covers all canonical functions. The temporary
disabled-idle entry and its exact-startup gate have been removed. CMake now
links the reconstructed `FirmwareMainLoop` and firmware-entry module and places
the main-loop module at canonical `RAM:fdcea9`. The generated link manifest now
passes all 446 declared C251/A251 objects to L251 exactly once; it replaced a
manual invocation that duplicated 19 names and omitted 40 translation units.
The corrected serial Wine/Keil baseline and the current native-Windows build
compile every object without source diagnostics. The physical O5/SIZE layout completes L251 and OH251 with zero
warnings and errors. Runtime libraries are selected by L251, all unresolved
symbols are gone, and compact A251 veneers own the six fixed reset/interrupt
entries. The current program uses 456,289 of 458,672 linker-owned main-payload
bytes. Packaging produces a structurally valid 512-KiB image with matching
container and recovery-monitor checksums. Recovery firmware is untouched. See
`BUILD_MILESTONE.md` for current build boundaries; the root `README.md`
contains build commands.

## Validate the baseline

With Ghidra closed, validate the stored project read-only:

```sh
GHIDRA_HOME=/path/to/ghidra \
    tools/validate_canonical_ghidra_project.sh \
    PROJECT_DIR gplextor --prepared-baseline
```

This checks both source hashes, the fixed language, project layout, memory
maps, coverage, checksum, function/tag/ABI totals, graph and data-reference
totals, risk gates, indirect tables, runtime overlays and interrupt routing,
and fixed-language semantics. Omit `--prepared-baseline` after intentional
reconstruction work changes mutable names or signatures.

Complete C coverage does not mean every decompiler inference is automatically
correct, the original compiler is known, undocumented hardware fields have
physical names, or a compiling/checksummed image is safe.

## Processor and ABI rules

- Use `80251-fixed:BE:24:default` with `srcMode=1`.
- Keep `RAM:`, `SFR:`, and `BITS:` identities distinct.
- Do not infer a firmware-wide calling convention. Parameters and returns may
  occupy function-specific registers, condition flags, or caller stack slots.
- Hardware return-address pushes are omitted from the decompiler stack model;
  explicit byte, word, and dword stack operations remain real.
- Inspect instructions and P-code around dynamic `@SPX`, shared tails,
  non-contiguous bodies, thunks, coroutine `ERET`, and flag-return helpers.
- Leave a mixed or composite ABI unforced until its source adapter can
  represent it faithfully.

## C/C++ data representation

Multi-byte firmware values are big-endian unless instructions prove otherwise.
Use distinct source types or accessors for CPU near pointers, CPU 24-bit far
pointers, controller-buffer addresses, SFR/bit references, and recovery flash
programming aliases. A 24-bit controller address is not necessarily a CPU
pointer.

Preserve volatile access order for SFRs, controller ports, flash aliases,
interrupt-shared state, and hardware/coroutine-polled state. Inherited 8051 SFR
labels are not evidence of LC898095 physical roles; use neutral names until
hardware evidence exists.

Use byte-exact packed records only with compile-time size and offset checks.
For rings and queues, preserve element size, capacity, cursor units, wrapping,
sentinels, and producer/consumer ownership. Model phase-reused RAM with explicit
lifetime views or unions: a broad clear/copy does not prove adjacent state is
one array, and overlapping views do not necessarily coexist.

The recovery capture's three routines copied to physical `RAM:000800` are
mutually exclusive images and must remain separate overlay/build units.

## Decompilation acceptance

Work directly from the canonical Ghidra database through the interactive
decompiler and references. Do not retain a bulk pseudocode mirror. Before
accepting a function as translation input:

1. Match its entry and body intervals to canonical Ghidra.
2. Reject decompiler failures and instruction-listing fallbacks.
3. Check inferred parameters against custom storage, callers, initial register
   writes, condition flags, and explicit stack operations.
4. Reject implausible stack arrays or overlapping locals; specifically recheck
   `RAM:fb7d5a`, which once produced a synthetic 252-byte local.
5. Preserve qualified address spaces, volatile ordering, integer widths,
   signedness, byte order, wrap behavior, and overlapping lifetimes.
6. Keep unproven physical meanings and units neutral.

Any temporary pseudocode or disassembly extracts are disposable views. The
accepted maintained output is the reviewed C implementation plus its compact
address tag and the supporting evidence retained in Ghidra.

## Source and image acceptance

Use the source partition in `C_TRANSLATION_UNITS.md`. Each translated function
needs one compact address tag, a reviewed ABI boundary, behavioral or
simulator-backed tests, explicit volatile accesses where required, and recorded
unresolved assumptions. Native ABI and validation notes belong in Ghidra plate
comments or focused subsystem documentation, not in a parallel manifest.

## Validation evidence

- All 274 registered CTest entries pass on native Windows, including both
  exhaustive reconstruction aggregates and the image/format checks.
- Keil C251 5.60 under Wine and natively on Windows has compiled the final vector, coroutine, and stack
  transform C modules at the uniform O5/SIZE setting with zero diagnostics; listing review confirmed the
  required RETI, ERET, reset-transfer, register, and stack operations.
- Keil A251 4.69 rebuilds the five retained assembly units with zero
  diagnostics. Four own low-level boundaries; the fifth exports the
  allocation-free bank-zero near-data symbol. Ordinary bit, arithmetic, and
  typed-call bodies were moved to C251 units; no same-name assembly
  implementation remains.
- The physical 446-object link, absolute-DATA audit, OH251 conversion, image
  pack, and structural verification complete successfully. This proves format
  and build consistency, not operational safety.
- The Windows firmware target also generates a no-I/O update plan containing
  the stock updater's 31 mode-4 chunks and final mode-5 activation CDB. This
  proves host-command reproducibility, not flash-bus mapping or safe effects.
- An INQUIRY-identified live PX-W5224A 1.04 supplied complete read-only
  backups of both 512-KiB CPU data windows. They are byte-identical and the
  execution view exactly equals immutable stock 1.04 through `RAM:feffff`
  plus the protected runtime capture at `RAM:ff0000..ffffff`. This proves the
  normal-mode read alias, not programming-state writes or erase behavior.
- The 1,717 unique compact C address tags exactly match the 1,717 live function
  entries in canonical Ghidra. Extra source occurrences are intentional host
  and target variants at the same canonical address.

Track these claims independently:

1. **Behavioral match:** tests reproduce outputs and state transitions.
2. **Function-byte match:** a candidate toolchain emits an identical bounded
   routine at the intended address.
3. **Image match:** code, tables, placement, padding, header, and checksum all
   match.

Image generation must follow `W5224A_FIRMWARE_FORMAT.md`. A valid checksum is
not evidence of equivalence or safe execution. Flash testing requires explicit
authorization and a verified recovery route.

## Remaining unknowns

- Original compiler, linker, libraries, optimization, and locator setup.
- LC898095 SFR/MMIO fields, buffer partition, signal identities, and servo
  units.
- Deterministic later-session lead-in selection and useful lead-in C2 data.
- Programming-state flash overlay/byte-lane behavior, physical erase/program
  effects, and recovery after a checksummed but semantically invalid image.
- Lowest optical data level accessible through the controller.
