# Plextor PX-W5224A reverse-engineering guide

## Project context and goals

- Reverse-engineer the Plextor PX-W5224A CD-R/CD-RW drive firmware and produce an evidence-backed, maintainable C/C++ reconstruction.
- The primary firmware target is version 1.04 (`data/W5224A104/MONT_104.bin`), with versions 1.01 through 1.03 retained for comparison.
- Use Ghidra MCP as the primary interface for program inspection, control-flow recovery, symbol and type work, and carefully scoped database edits.
- The canonical Ghidra programs are `/validated/MONT_104.bin` and `/validated/read_memory_ff0000.debug` in `ghidra/gplextor`. Their analysis metadata is authoritative; generated CSV or pseudocode exports are disposable views, not competing sources of truth.
- Preserve behavioral fidelity to the original firmware. Do not present guessed names, types, hardware roles, calling conventions, or matching status as verified facts.
- Keep original firmware images and runtime captures immutable. Store analysis in Ghidra metadata, documentation, or reproducible tooling.

## Agent memory protocol

Before beginning substantial work:

1. Read this file.
2. Read `docs/agent-memory/active_context.md` for the current analysis state, open questions, and immediate priorities.
3. Read `docs/agent-memory/learned_patterns.md` for durable architecture, firmware-format, Ghidra, and hardware-interface findings.
4. Consult task-specific documentation and the live Ghidra programs before relying on remembered addresses or names.

At the end of every completed task or plan:

1. Refresh `docs/agent-memory/active_context.md` when the current state, verified results, open questions, or next priorities changed.
2. Append or refine `docs/agent-memory/learned_patterns.md` only when the task established a reusable fact through firmware evidence, runtime observation, or reproducible tooling.
3. Keep hypotheses in active context and label them explicitly. Promote them to learned patterns only after confirmation.
4. Keep active context concise by replacing superseded state instead of accumulating a chronological transcript.
5. Update this file only when a durable project goal, workflow rule, safety boundary, or architectural decision changes.

## Reverse-engineering conventions

- Always qualify addresses with their address space when ambiguity is possible: `RAM:`, `SFR:`, or `BITS:`.
- Record the firmware version or runtime-capture identity alongside address-based findings.
- Prefer direct evidence from instructions, data references, callers, runtime captures, and reproducible scripts over decompiler appearance alone.
- Clearly distinguish confirmed behavior, strong inference, and untested hypothesis.
- Preserve existing Ghidra symbols, comments, types, bookmarks, and user analysis unless the requested task requires a specific correction.
- Inspect before applying bulk Ghidra changes. Avoid speculative bulk renaming, typing, function creation, or flow repair.
- Use the custom `80251-fixed:BE:24:default` language for these programs. Do not silently fall back to Ghidra's stock preliminary 80251 language.
- Treat function signatures as local evidence. The firmware uses function-specific register assignments that are unsafe to generalize into one universal ABI.
- Before replacing stock instructions, verify every overwritten instruction boundary in the canonical disassembly and guard the full original instructions in the patcher. Check near/far call and return widths, including the executing code bank, in the linked payload.
- Validate decoder, stack-model, and calling-convention changes in an isolated import before applying them to the live analysis database.
- A decompiler result is not source equivalence. Any eventual C/C++ reconstruction must be tracked separately from behavioral and byte-identical matching claims.
- Once a canonical function has an accepted C implementation, remove its same-address assembly or literal-DB body and export. Retain only addressless ABI/atomic adapters or explicitly documented low-level exceptions.

## Safety boundaries

- `build/` is reserved for the user's VS Code workflow. Never place agent probes, test builds, extracted documentation, or scratch files there. Use `out/agent-tests/` for temporary work and `out/windows/` for the Windows preset output. Preserve user-managed build contents.
- Firmware reads, comparison, decompilation, and analysis-database edits are ordinary in-scope work.
- Flash writes, firmware activation, reset commands, destructive drive tests, and hardware modifications require explicit user authorization and a stated recovery strategy.
- Prefer read-only commands for live-drive experiments. Record the exact CDB, transfer direction, address, length, device identity, and observed status.
- LED diagnostics must be comfortably countable by eye, with slow pulses and clear pauses between groups. The user should not need a phone recording to count them; retain the slower post-diag21 timing in future reporters.
- Flashing is costly for the user. Do not resume one-observation-per-flash LED experiments. Establish concrete offline evidence and prioritize a reusable SCSI memory/trace interface before proposing another hardware diagnostic.
- Do not overwrite or transform original binaries in place. Derived files must be separately named and reproducible.
- Save Ghidra changes only after checking that they affect the intended program and address space.

## Primary references

- `docs/W5224A_FIRMWARE_FORMAT.md`: immutable image identities, layout, checksum, and image-generation contract.
- `docs/C_RECONSTRUCTION_STATUS.md`: validation, decompilation and C/C++ acceptance gates, ABI rules, and source data-model policy.
- `docs/C_TRANSLATION_UNITS.md`: proposed maintainable source partition and reconstruction order.
- `docs/COMPILER_IDENTIFICATION.md`: compiler/linker evidence, unknowns, and exact-match experiment plan.
- `docs/HARDWARE_PLATFORM.md`: external LC898095 platform evidence, address-map confidence, unresolved hardware evidence, and SFR naming caveats.
- `docs/D8_LEADIN_ANALYSIS.md`: exact negative-D8 acquisition, position-validation, session-selection, and C2 cursor behavior.
- `docs/FIRMWARE_UPDATE_ANALYSIS.md`: READ/WRITE BUFFER finalization, runtime interrupt ownership and flash loaders, programming ranges, and reset/recovery handoff.
- `tools/ghidra_80251_fixed/README.md`: processor-language corrections and stack/calling-model constraints.
- `tools/ghidra_scripts/ValidatePlextorCanonicalProject.java`: read-only validation of both canonical programs, including main coverage and runtime relocation/interrupt invariants, without replaying their metadata.
- `tools/ghidra_scripts/ValidatePlextor80251Language.java`: compact firmware-corpus regression tests for the fixed decoder, P-code flags, bit spaces, and explicit stack widths.
- `tools/validate_canonical_ghidra_project.sh`: read-only headless validation of both canonical programs, including stored project copies.
