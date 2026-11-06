# Repository archive cleanup audit

User's subsequent archive decision (2026-11-05): discard the entire `out/`
directory, including captures, recovery backups, and other historical
artifacts. The preservation recommendations below describe the original
evidence-preserving audit; they are not requirements for the user's chosen
archive scope. The user plans to perform the deletion. No deletion was
performed by the agent.

Audited 2026-11-05. No existing files were deleted or changed by the audit,
apart from the required agent-memory note. Sizes are logical file sizes,
not compressed sizes or allocated disk space. The inventory excludes `.git/`.

The working tree contained 91,270 files totaling 6.44 GiB before audit output;
`out/` accounts for 6.36 GiB. It is **not safe to discard all of `out/`**:
it mixes rebuildable output with original drive observations and historical
source/image archives.

## Deletion candidates

These are generated build output and caches. Removing them loses cached
builds, debugger symbols, and any build/test logs inside the named build
directories. Keep any such logs wanted as historical validation evidence.
Rebuilding requires the corresponding source, inputs, and toolchain; it does
not promise to reproduce a historical executable from current sources.

| Path or category | Size | Basis |
|---|---:|---|
| `out/agent-tests/redumper-no-probe-vs17/` | 968.0 MiB | CMake cache identifies sibling `../redumper` as source; generated Visual Studio projects, objects, executables, symbols, and dependencies. |
| `out/agent-tests/redumper-c4-review/` | 908.6 MiB | Same external Redumper build layout. |
| `out/agent-tests/redumper-no-probe/` | 823.2 MiB | Same external Redumper build layout; includes CTest output. |
| `out/agent-tests/fwpatch-plextor-rust/` | 90.6 MiB | Cargo target output (`CACHEDIR.TAG`, `.rustc_info.json`, `release/`); maintained patcher moved to `../spiraldrive-plextor/`. |
| Python `__pycache__/` directories | 0.66 MiB | 32 bytecode files under `tools/`, `tests/`, `tests/simulator/`, and `out/agent-tests/`. |
| Additional compiler intermediates under `out/` | 453.1 MiB | `.obj`, `.o`, `.ifc`, `.pch`, `.pdb`, `.ilk`, `.idb`, `.tlog`, `.rlib`, `.rmeta`; excludes the four build trees above and the explicit source archives below. |

The non-overlapping candidate set totals **3.17 GiB / 20,594 files**.
Every candidate was checked against `git ls-files`: none is tracked.
Untracked status alone was not used to decide whether a file is disposable.

The exact file list, including size and classification, is
`out/agent-tests/archive-audit/deletion-candidates.tsv`.
`inventory.json` records the initial inventory, and
`out-review-required.tsv` lists the remaining `out/` files for review.
These audit outputs are themselves disposable after this decision is recorded.
The lists are a point-in-time review aid, not an executable deletion script.

`tools/fwpatch-plextor/` is empty and can also be removed once its open
directory handle is released. It provides no meaningful space saving.

## Further cleanup after preserving evidence

| Location | What can be removed | What to preserve first |
|---|---|---|
| `out/windows/` (100.2 MiB before the candidate removals above) | Generated CMake/Visual Studio files, `.cmake/`, `CMakeFiles/`, build products, and generated sources. | Any desired final firmware image, `main.map`, listings, validation/tool JSON, test results, and build settings. The preset recreates the build directory, but original `data/` inputs are currently absent. |
| `out/agent-tests/diag19-build/` through `diag29-build/`, `windows-repro/`, and other CMake builds | Build-system scaffolding and intermediate outputs. | Final diagnostic images, hashes/manifests, linker maps/listings and reports associated with documented observations. A historical diagnostic may not be reproducible from the current source/configuration. |
| `out/simulator/` (1.25 GiB before candidate removals) | Converted stock/reconstruction images, generated projects, `.hex`, `.a51`, debugger scripts, and repeat-run working files. | Results, mismatches, trace evidence and the image/map/configuration identity needed to interpret them. In particular retain the documented pre-inline `auto-fa56c5` mismatch evidence. Removing resumable runs loses completed work. |
| `out/agent-tests/fwpatch-plextor/` | Old C++ patcher CMake build output. | Any historical executable or build record desired for provenance; the old C++ source is no longer present here. |
| Ghidra pseudocode/listing exports in `out/agent-tests/york-d8/` and elsewhere | Disposable views of analysis retained in the saved Ghidra database. | Handwritten probes, simulator results, and any export that captures historical state no longer represented by the current database. |

Do not apply extension-wide deletion rules to the repository. `.bin`, `.json`,
`.a51`, `.log`, `.hex`, `.bak`, and database-looking files can be evidence,
maintained inputs, or database internals.

## Keep in the archive

- `src/`, `tests/`, `tools/`, `cmake/`, `docs/`, root build/configuration files,
  and `AGENTS.md`, including untracked source and documentation. In particular,
  keep the reviewed `tests/simulator/metadata/W5224A104.json` snapshot and case
  JSON: they are maintained test inputs, not disposable exports.
- All of `ghidra/gplextor.gpr` and `ghidra/gplextor.rep/` together (58.2 MiB).
  The zero-byte `.gpr` is the project marker. `.gbf`, `.grf`, `.prp`,
  `~index.bak`, and journal files are project internals; do not prune them by
  filename. Save and close Ghidra before taking a consistent project copy.
  Any optional scratch-program removal belongs in Ghidra after reviewing its
  analysis, rather than deleting raw database files.
- Original drive observations and recovery material under
  `out/agent-tests/windows-*backup*/`, `windows-*readback*/`, diagnostic
  readback folders, `gofw-*/live/`, `gofw-*-live*/`, `gofw-trace-live/`,
  `gofw-c2-scan/`, `york-107-flash-20261104/`, and
  `york-g107-flash-20261104/`. These examples are not an exhaustive allowlist;
  other probe folders also contain physical observations.
- Historical source/image archives, especially
  `out/agent-tests/diag27-scoped-handshake-delay-20260926/`,
  `diag28-handshake-phases-20260926/`, `gofw-stream-failed-source/`, and
  `gofw-position-restored-src/`. The first two contain complete source trees;
  the latter two preserve old C4 sources absent from the current repository.
- Exact firmware images used in hardware experiments and the reports/hashes
  tying them to their observations, even though the images were generated.
- `.git/` if preserving repository history (686.3 MiB in this workspace).
  Omitting it from a source-only export is an archive-format choice, not
  ordinary cache cleanup.

## Archive completeness findings

The audit found **no `data/` directory in the current workspace view**.
The project requires original Mont 1.01–1.04 images, the runtime-monitor
capture, and comparison images such as York 1.07. Recover/include those
immutable inputs from their existing storage, with the documented hashes;
do not substitute a generated/patched image for an original.

There are modified tracked files and substantial untracked source,
documentation, and Ghidra databases. The tracked main-program database
`db.390.gbf` is gone, while current `db.399.gbf` and `db.400.gbf` are untracked.
A `git archive HEAD`, ordinary clone, or untracked-file cleanup would therefore
miss current work. Preserve the complete saved working state or commit the
intended archive contents before relying on a Git-only archive.

`firmware/` is also absent, consistent with the recorded move of patch
descriptions and the patcher to `../spiraldrive-plextor/`. Preserve that sibling
repository separately if the archive should include the maintained patcher.
`build/` is absent in this workspace view; if present in another copy, inspect
it separately and preserve the user's VS Code configuration as required by
`AGENTS.md`.

This was a filesystem/build-layout audit. It did not open or modify Ghidra,
access hardware, rebuild firmware, or validate archive restorability.
